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
int bake_do_build_intern(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *project,
    bool rebuild)
{
    /* Step 1: export metadata to environment to make project discoverable */
    ut_log_push("install-metadata");
    if (project->public)
        ut_try (bake_install_metadata(config, project), NULL);
    ut_log_pop();

    /* Step 2: parse driver configuration in project JSON */
    ut_log_push("load-drivers");
    ut_try (bake_project_parse_driver_config(config, project), NULL);
    ut_log_pop();

    /* Step 3: parse dependee configuration */
    ut_log_push("load-dependees");
    ut_try (bake_project_parse_dependee_config(config, project), NULL);
    ut_log_pop();

    /* Step 4: if rebuilding, clean project cache for current platform/config */
    if (rebuild) {
        ut_log_push("clean-cache");

        ut_try( bake_project_clean_current_platform(config, project), NULL);

        /* If project sets keep-binary to true, make sure to only rebuild when
         * the user is explicitly rebuilding this project. That way, even when
         * rebuilding a tree of projects, keep-binary projects won't get
         * rebuilt. This feature is useful if a tree contains projects that take
         * a long time to build, and for projects that want to version control
         * binaries. */
        if (project->keep_binary) {
            /* Only clean if just one project was discovered */
            if (bake_crawler_count(crawler) == 1) {

                /* Only remove the artefact for the current platform */
                ut_try( ut_rm(project->artefact_path), NULL);
            }
        }

        ut_log_pop();
    }

    /* Step 5: now that project config is fully loaded, check dependencies */
    ut_log_push("validate-dependencies");
    ut_try (bake_project_check_dependencies(config, project), NULL);
    ut_log_pop();

    /* Step 6: invoke code generators, if any */
    ut_log_push("generate");
    ut_try (bake_project_generate(config, project), NULL);
    ut_log_pop();

    /* Step 7: clear environment of old project files */
    ut_log_push("clear");
    if (project->public && project->type != BAKE_TOOL)
        ut_try (bake_install_clear(config, project, false), NULL);
    ut_log_pop();

    /* Step 8: export project files to environment */
    ut_log_push("install-prebuild");
    if (project->public && project->type != BAKE_TOOL)
        ut_try (bake_install_prebuild(config, project), NULL);
    ut_log_pop();

    /* Step 9: prebuild step */
    ut_log_push("prebuild");
    ut_try (bake_project_prebuild(config, project), NULL);
    ut_log_pop();

    /* Step 10: build project */
    ut_log_push("build");
    if (project->artefact)
        ut_try (bake_project_build(config, project), NULL);
    ut_log_pop();

    /* Step 11: postbuild step */
    ut_log_push("postbuild");
    ut_try (bake_project_postbuild(config, project), NULL);
    ut_log_pop();

    /* Step 12: install binary to environment */
    ut_log_push("install-postbuild");
    if (project->public && project->artefact)
        ut_try (bake_install_postbuild(config, project), NULL);
    ut_log_pop();

    return 0;
error:
    return -1;
}

int bake_do_build(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *project)
{
    return bake_do_build_intern(config, crawler, project, false);
}

int bake_do_clean(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *project)
{
    return bake_project_clean(config, project);
}

int bake_do_rebuild(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *project)
{
    return bake_do_build_intern(config, crawler, project, true);
}

int bake_do_install(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *project)
{
    if (!project->public) return 0;

    /* Step 1: export metadata to environment to make project discoverable */
    ut_try (bake_install_metadata(config, project), NULL);

    /* Step 2: export project files to environment */
    ut_try (bake_install_prebuild(config, project), NULL);

    /* Step 3: install binary to environment */
    if (project->artefact)
        ut_try (bake_install_postbuild(config, project), NULL);

    return 0;
error:
    return -1;
}
