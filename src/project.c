/* Copyright (c) 2010-2018 Sander Mertens
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "bake.h"

static
int16_t bake_project_parse_value(
    bake_project *p,
    JSON_Object *jo)
{
    uint32_t i, count = json_object_get_count(jo);
    bool error = false;

    for (i = 0; i < count; i ++) {
        JSON_Value *v = json_object_get_value_at(jo, i);
        const char *member = json_object_get_name(jo, i);

        if (!strcmp(member, "public")) {
           ut_try (bake_json_set_boolean(&p->public, member, v), NULL);
        } else
        if (!strcmp(member, "author")) {
            ut_try (bake_json_set_string(&p->author, member, v), NULL);
        } else
        if (!strcmp(member, "description")) {
            ut_try (bake_json_set_string(&p->description, member, v), NULL);
        } else
        if (!strcmp(member, "version")) {
            ut_try (bake_json_set_string(&p->version, member, v), NULL);
        } else
        if (!strcmp(member, "repository")) {
            ut_try (bake_json_set_string(&p->repository, member, v), NULL);
        } else
        if (!strcmp(member, "language")) {
            ut_try (bake_json_set_string(&p->language, member, v), NULL);
        } else
        if (!strcmp(member, "use")) {
            ut_try (bake_json_set_array(&p->use, member, v), NULL);
        } else
        if (!strcmp(member, "use_private")) {
            ut_try (bake_json_set_array(&p->use_private, member, v), NULL);
        } else
        if (!strcmp(member, "link")) {
            ut_try (bake_json_set_array(&p->link, member, v), NULL);
        } else
        if (!strcmp(member, "sources")) {
            ut_try (bake_json_set_array(&p->sources, member, v), NULL);
        } else
        if (!strcmp(member, "includes")) {
            ut_try (bake_json_set_array(&p->includes, member, v), NULL);
        } else
        if (!strcmp(member, "keep_binary")) {
            ut_try (bake_json_set_boolean(&p->keep_binary, member, v), NULL);
        } else {
            ut_throw("unknown member '%s' in project.json", member);
            goto error;
        }
    }

    return 0;
error:
    return -1;
}

static
int16_t bake_project_set(
    bake_project *p,
    const char *id,
    const char *type)
{
    p->id = strdup(id);
    char *ptr, ch;
    for (ptr = p->id; (ch = *ptr); ptr ++) {
        if (ch == '.') {
            *ptr = '/';
        } else
        if (!isalpha(ch) && !isdigit(ch) &&
            ch != '_' && ch != '/' && ch != '-') {
            ut_throw("project id '%s' contains invalid characters", id);
            goto error;
        }
    }

    if (!strcmp(type, "application")) {
        p->type = BAKE_APPLICATION;
    } else if (!strcmp(type, "package")) {
        p->type = BAKE_PACKAGE;
    } else if (!strcmp(type, "tool")) {
        p->type = BAKE_TOOL;
    } else if (!strcmp(type, "executable")) {
        p->type = BAKE_APPLICATION;
        ut_warning("'executable' is deprecated, use 'application' instead");
    } else if (!strcmp(type, "library")) {
        p->type = BAKE_PACKAGE;
        ut_warning("'library' is deprecated, use 'package' instead");
    } else {
        ut_throw("project type '%s' is not valid", type);
        goto error;
    }

    return 0;
error:
    return -1;
}

static
int16_t bake_project_parse(
    bake_project *project)
{
    char *file = ut_asprintf("%s/project.json", project->path);

    if (ut_file_test(file) == 1) {
        JSON_Value *j = json_parse_file(file);
        if (!j) {
            ut_throw("failed to parse '%s'", file);
            goto error;
        }

        JSON_Object *jo = json_value_get_object(j);
        if (!jo) {
            ut_throw("failed to parse '%s' (expected object)", file);
            goto error;
        }

        const char *j_id = json_object_get_string(jo, "id");
        if (!j_id) {
            ut_throw("failed to parse '%s': missing 'id'", file);
            goto error;
        }

        const char *j_type = json_object_get_string(jo, "type");
        if (!j_type) {
            j_type = "package";
        }

        ut_try (bake_project_set(project, j_id, j_type), NULL);

        JSON_Object *j_value = json_object_get_object(jo, "value");
        if (j_value) {
            ut_try (bake_project_parse_value(project, j_value), NULL);
        }

        project->json = jo;
    } else {
        ut_throw("could not find file '%s'", file);
        goto error;
    }

    return 0;
error:
    return -1;
}

static
bake_project_driver* bake_project_get_driver(
    bake_project *project,
    const char *driver_id)
{
    ut_iter it = ut_ll_iter(project->drivers);
    while (ut_iter_hasNext(&it)) {
        bake_project_driver *driver = ut_iter_next(&it);
        if (!strcmp(driver->driver->id, driver_id)) {
            return driver;
        }
    }

    return NULL;
}

static
int bake_project_load_driver(
    bake_project *project,
    const char *driver_id,
    JSON_Object *config)
{
    bake_driver *driver = bake_driver_get(driver_id);
    if (!driver) {
        goto error;
    }

    bake_project_driver *project_driver = malloc(sizeof(bake_project_driver));
    project_driver->driver = driver;
    project_driver->json = config;

    ut_ll_append(project->drivers, project_driver);

    return 0;
error:
    return -1;
}

static
int16_t bake_project_load_dependee_config(
    bake_config *config,
    bake_project *project,
    const char *package_id,
    const char *file)
{
    JSON_Value *j = json_parse_file(file);
    if (!j) {
        ut_throw("failed to parse '%s'", file);
        goto error;
    }

    JSON_Object *jo = json_value_get_object(j);
    if (!jo) {
        ut_throw("failed to parse '%s' (expected object)", file);
        goto error;
    }

    uint32_t i, count = json_object_get_count(project->json);

    for (i = 0; i < count; i ++) {
        const char *member = json_object_get_name(project->json, i);

        if (!strcmp(member, "value") || !strcmp(member, "id") ||
            !strcmp(member, "type"))
        {
            ut_throw("dependee concig cannot override 'value', 'type' or 'id'");
            goto error;
        }

        JSON_Value *value = json_object_get_value_at(project->json, i);
        JSON_Object *obj = json_value_get_object(value);

        bake_project_driver *driver = bake_project_get_driver(project, member);
        if (!driver) {
            ut_try( bake_project_load_driver(project, member, obj), NULL);
        } else {
            if (!bake_attributes_parse(
                config, project, project->id, driver->json, driver->config))
            {
                ut_throw("failed to load dependee config for driver %s",member);
                goto error;
            }
        }
    }

    return 0;
error:
    return -1;
}

static
int16_t bake_project_add_dependee_config(
    bake_config *config,
    bake_project *project,
    const char *dependency)
{
    const char *libpath = ut_locate(dependency, NULL, UT_LOCATE_PACKAGE);
    if (!libpath) {
        ut_throw("failed to locate path for dependency '%s'", dependency);
        goto error;
    }

    /* Check if dependency has a dependee file with build instructions */
    char *file = ut_asprintf("%s/dependee.json", libpath);
    if (ut_file_test(file)) {
        ut_try (
          bake_project_load_dependee_config(config, project, dependency, file),
          NULL);
    }

    free(file);

    return 0;
error:
    return -1;
}

/* -- Public API -- */

bake_project* bake_project_new(
    const char *path,
    bake_config *cfg)
{
    bake_project *result = ut_calloc(sizeof (bake_project));
    if (!path && !cfg) {
        return result;
    }

    result->path = path ? strdup(path) : NULL;
    result->sources = ut_ll_new();
    result->includes = ut_ll_new();
    result->use = ut_ll_new();
    result->use_private = ut_ll_new();
    result->use_build = ut_ll_new();
    result->link = ut_ll_new();
    result->files_to_clean = ut_ll_new();
    result->drivers = ut_ll_new();

    /* Parse project.json if available */
    if (path) {
        ut_try (
            bake_project_parse(result),
            "failed to parse '%s/project.json'",
            path);

        /* If 'src' and 'includes' weren't set, use defaults */
        if (!ut_ll_count(result->sources)) {
            ut_ll_append(result->sources, "src");
        }
        if (!ut_ll_count(result->includes)) {
            ut_ll_append(result->includes, "include");
        }

        if (!result->language) {
            result->language = ut_strdup("c");
        }
    }

    return result;
error:
    return NULL;
}

void bake_project_free(
    bake_project *project)
{

}

int bake_project_load_drivers(
    bake_project *project)
{
    uint32_t i, count = json_object_get_count(project->json);
    bool error = false;

    for (i = 0; i < count; i ++) {
        const char *member = json_object_get_name(project->json, i);

        if (!strcmp(member, "id") || !strcmp(member, "type") ||
            !strcmp(member, "value"))
            continue;

        JSON_Value *value = json_object_get_value_at(project->json, i);
        JSON_Object *obj = json_value_get_object(value);
        ut_try( bake_project_load_driver(project, member, obj), NULL);
    }

    return 0;
error:
    return -1;
}

int bake_project_parse_driver_config(
    bake_config *config,
    bake_project *project)
{
    ut_iter it = ut_ll_iter(project->drivers);

    while (ut_iter_hasNext(&it)) {
        bake_project_driver *driver = ut_iter_next(&it);
        driver->config = bake_attributes_parse(
            config, project, project->id, driver->json, NULL);
    }

    return 0;
error:
    return -1;
}

int16_t bake_project_parse_dependee_config(
    bake_config *config,
    bake_project *project)
{
    /* Add dependencies to link list */
    if (project->use) {
        ut_iter it = ut_ll_iter(project->use);
        while (ut_iter_hasNext(&it)) {
            char *dep = ut_iter_next(&it);
            if (bake_project_add_dependee_config(config, project, dep)) {
                goto error;
            }
        }
    }

    /* Add private dependencies to link list */
    if (project->use_private) {
        ut_iter it = ut_ll_iter(project->use_private);
        while (ut_iter_hasNext(&it)) {
            char *dep = ut_iter_next(&it);
            if (bake_project_add_dependee_config(config, project, dep)) {
                goto error;
            }
        }
    }

    return 0;
error:
    return -1;
}

static
int16_t bake_check_dependency(
    bake_project *p,
    const char *dependency,
    uint32_t artefact_modified,
    bool private)
{
    const char *lib = ut_locate(dependency, NULL, UT_LOCATE_PACKAGE);
    if (!lib) {
        ut_info("use '%s' => #[red]missing#[normal]", dependency, lib);
        ut_throw("missing dependency '%s'", dependency);
        goto error;
    }

    time_t dep_modified = ut_lastmodified(lib);

    if (!artefact_modified || dep_modified <= artefact_modified) {
        const char *fmt = private
            ? "use '%s' => '%s' #[yellow](private)"
            : "use '%s' => '%s'"
            ;
        ut_ok(fmt, dependency, lib);
    } else {
        p->artefact_outdated = true;
        const char *fmt = private
            ? "use '%s' => '%s' #[green](changed) #[yellow](private)"
            : "use '%s' => '%s' #[green](changed)"
            ;
        ut_ok(fmt, dependency, lib);
    }

proceed:
    return 0;
error:
    return -1;
}

int16_t bake_project_check_dependencies(
    bake_config *config,
    bake_project *project)
{
    time_t artefact_modified = 0;

    char *artefact_full = ut_asprintf("bin/%s-%s/%s",
        UT_PLATFORM_STRING, config->configuration, project->artefact);
    if  (ut_file_test(artefact_full)) {
        artefact_modified = ut_lastmodified(artefact_full);
    }
    free(artefact_full);

    if (project->use) {
        ut_iter it = ut_ll_iter(project->use);
        while (ut_iter_hasNext(&it)) {
            char *package = ut_iter_next(&it);
            if (bake_check_dependency(
                project, package, artefact_modified, false))
            {
                goto error;
            }
        }
    }

    if (project->use_private) {
        ut_iter it = ut_ll_iter(project->use_private);
        while (ut_iter_hasNext(&it)) {
            char *package = ut_iter_next(&it);
            if (bake_check_dependency(
                project, package, artefact_modified, true))
            {
                goto error;
            }
        }
    }

    if (project->artefact_outdated) {
        if (ut_rm(project->artefact)) {
            goto error;
        }
    }

    return 0;
error:
    return -1;
}


int16_t bake_project_generate(
    bake_config *config,
    bake_project *project)
{
    return 0;
}

int16_t bake_project_build_generated(
    bake_config *config,
    bake_project *project)
{
    return 0;
}

int16_t bake_project_build(
    bake_config *config,
    bake_project *project)
{
    return 0;
}
