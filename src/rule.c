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

bake_pattern* bake_pattern_new(
    const char *name,
    const char *pattern)
{
    bake_pattern *result = ut_calloc(sizeof(bake_pattern));
    result->super.kind = BAKE_RULE_PATTERN;
    result->super.name = name;
    result->super.cond = NULL;
    result->pattern = pattern ? ut_strdup(pattern) : NULL;
    return result;
}

bake_rule* bake_rule_new(
    const char *name,
    const char *source,
    bake_rule_target target,
    bake_rule_action_cb action)
{
    bake_rule *result = ut_calloc(sizeof(bake_rule));
    result->super.kind = BAKE_RULE_RULE;
    result->super.name = name;
    result->super.cond = NULL;
    result->target = target;
    result->source = source;
    result->action = action;
    return result;
}

bake_dependency_rule* bake_dependency_rule_new(
    const char *name,
    const char *deps,
    bake_rule_target dep_mapping,
    bake_rule_action_cb action)
{
    bake_dependency_rule *result = ut_calloc(sizeof(bake_dependency_rule));
    result->super.name = name;
    result->super.cond = NULL;
    result->target = dep_mapping;
    result->deps = deps;
    result->action = action;
    return result;
}

bake_file* bake_file_new(
    const char *name,
    uint64_t timestamp)
{
    bake_file *result = ut_calloc(sizeof(bake_file));
    result->name = ut_strdup(name);
    result->timestamp = timestamp;
    return result;
}

static
int16_t bake_assertPathForFile(
    char *path)
{
    char buffer[512];
    strcpy(buffer, path);
    char *ptr = strrchr(buffer, '/');
    if (ptr) {
        ptr[0] = '\0';
    }

    if (!ut_file_test(buffer)) {
        ut_try (ut_mkdir(buffer), NULL);
    }

    return 0;
error:
    return -1;
}

static
bake_filelist* bake_node_eval_pattern(
    bake_node *n,
    bake_project *p)
{
    bool isSources = false;
    bake_filelist *targets = NULL;

    ut_trace("evaluating pattern");
    if (n->name && !stricmp(n->name, "SOURCES")) {
        targets = bake_filelist_new(NULL, NULL); /* Create empty list */
        isSources = true;

        /* If this is the special SOURCES rule, apply the pattern to
         * every configured source directory */
        ut_iter it = ut_ll_iter(p->sources);
        ut_dirstack ds = NULL;
        while (ut_iter_hasNext(&it)) {
            char *src = ut_iter_next(&it);
            ut_try (!(ds = ut_dirstack_push(ds, src)), NULL);

            ut_try (
                bake_filelist_addPattern(
                    targets, src, ((bake_pattern*)n)->pattern), NULL);

            ut_dirstack_pop(ds);
        }
    } else if (((bake_pattern*)n)->pattern) {
        /* If this is a regular pattern, match against project directory */
        targets = bake_filelist_new(NULL, ((bake_pattern*)n)->pattern);
    }

    if (!targets) {
        goto error;
    }

    return targets;
error:
    return NULL;
}

static
int16_t bake_node_run_rule_map(
    bake_driver *driver,
    bake_project *p,
    bake_config *c,
    bake_rule *r,
    bake_filelist *inputs,
    bake_filelist *targets)
{
    ut_iter it = bake_filelist_iter(inputs);
    int count = 0;
    while (ut_iter_hasNext(&it)) {
        bake_file *src = ut_iter_next(&it);
        bake_file *dst = NULL;
        const char *map = r->target.is.map(driver, c, p, src->name, NULL);
        if (!map) {
            ut_throw("failed to map file '%s'", src->name);
            goto error;
        }
        if (!(dst = bake_filelist_add(targets, map))) {
            ut_throw(NULL);
            goto error;
        }

        count ++;
        if (src->timestamp > dst->timestamp) {
            ut_log_overwrite(UT_OK, "#[green][#[white]%lld%%#[green]]#[white] %s",
                100 * count / bake_filelist_count(inputs),
                src->name);

            /* Make sure target directory exists */
            ut_try (bake_assertPathForFile(dst->name), NULL);

            /* Invoke action */
            char *srcPath = src->name;
            if (src->offset) {
                srcPath = ut_asprintf("%s/%s", src->offset, src->name);
            }
            r->action(driver, c, p, srcPath, dst->name, NULL);
            if (srcPath != src->name) {
                free(srcPath);
            }

            /* Check if error flag was set */
            if (p->error) {
                ut_throw("command for task '%s' failed", src->name);
                goto error;
            } else {
                p->freshly_baked = true;
                p->changed = true;
            }

            /* Update target with latest timestamp */
            dst->timestamp = ut_lastmodified(dst->name);
        } else {
            ut_trace("#[grey][%3lld%%] %s",
                100 * count / bake_filelist_count(inputs),
                src->name);
        }
    }

    return 0;
error:
    return -1;
}

static
int16_t bake_node_run_rule_pattern(
    bake_driver *driver,
    bake_project *p,
    bake_config *c,
    bake_rule *r,
    bake_filelist *inputs,
    bake_filelist *targets,
    bool shouldBuild)
{
    /* Do n-to-n comparison between sources and targets. If the
     * target list is empty, it is possible that files still have to
     * be generated, in which case the rule must be executed. */
    if (!shouldBuild) {
        if (!targets || !bake_filelist_count(targets)) {
            shouldBuild = true;
            ut_trace("no targets found for rule '%s', rebuilding",
                ((bake_node*)r)->name);
        } else {
            ut_iter src_iter = bake_filelist_iter(inputs);
            while (!shouldBuild && ut_iter_hasNext(&src_iter)) {
                bake_file *src = ut_iter_next(&src_iter);

                ut_iter dst_iter = bake_filelist_iter(targets);
                while (!shouldBuild && ut_iter_hasNext(&dst_iter)) {
                    bake_file *dst = ut_iter_next(&dst_iter);
                    if (!src->timestamp) {
                        shouldBuild = true;
                        ut_trace("'%s' does not exist for '%s', rebuilding",
                            dst->name,
                            ((bake_node*)r)->name);
                    } else if (src->timestamp > dst->timestamp) {
                        shouldBuild = true;
                        ut_trace("'%s' is newer than '%s', rebuilding",
                            src->name,
                            dst->name);
                    }
                }
            }
        }
    }

    char *dst = NULL;
    if (bake_filelist_count(targets) == 1) {
        bake_file *f = ut_ll_get(targets->files, 0);
        dst = f->name;
    }

    if (shouldBuild && inputs && bake_filelist_count(inputs)) {
        ut_strbuf source_list = UT_STRBUF_INIT;
        ut_iter src_iter = bake_filelist_iter(inputs);
        int count = 0;
        while (ut_iter_hasNext(&src_iter)) {
            bake_file *src = ut_iter_next(&src_iter);
            if (count) {
                ut_strbuf_appendstr(&source_list, " ");
            }
            ut_strbuf_appendstr(&source_list, src->name);
            count ++;
        }

        char *source_list_str = ut_strbuf_get(&source_list);

        if (dst) {
            ut_ok("#[bold]%s#[normal]", dst);
        } else {
            ut_ok("from #[bold]%s#[normal]", source_list_str);
        }

        r->action(driver, c, p, source_list_str, dst, NULL);
        if (p->error) {
            if (dst) {
                ut_throw("command for task '%s' failed", dst);
            } else {
                ut_throw("rule failed");
            }
            free(source_list_str);
            ut_throw(NULL);
            goto error;
        } else {
            p->freshly_baked = true;
            p->changed = true;
        }

        free(source_list_str);
    } else if (dst) {
        ut_trace("#[grey]%s", dst);
    }

    return 0;
error:
    return -1;
}

static
int16_t bake_node_eval(
    bake_driver *driver,
    bake_node *n,
    bake_project *p,
    bake_config *c,
    bake_filelist *inherits,
    bake_filelist *outputs)
{
    bake_filelist *targets = NULL, *inputs = NULL;

    if (n->cond && !n->cond(driver, c, p)) {
        return 0;
    }

    ut_log_push((char*)n->name);

    if (n->kind == BAKE_RULE_PATTERN) {
        targets = bake_node_eval_pattern(n, p);
        if (!targets) {
            targets = inherits;
        }
    } else {
        ut_trace("evaluating rule");
        targets = inherits;
    }

    /* Collect input files for node */
    if (n->deps) {
        bake_filelist *inputs = bake_filelist_new(NULL, NULL);
        ut_try (!inputs, NULL);

        /* Evaluate dependencies of node & collect its inputs */
        ut_iter it = ut_ll_iter(n->deps);
        while (ut_iter_hasNext(&it)) {
            bake_node *e = ut_iter_next(&it);
            if (bake_node_eval(driver, e, p, c, targets, inputs)) {
                ut_throw("dependency '%s' failed", e->name);
                goto error;
            }
        }

        /* Generate target files */
        if (n->kind == BAKE_RULE_RULE) {
            bake_rule *r = (bake_rule*)n;

            /* When rule specifies a map, generate targets from inputs */
            if (r->target.kind == BAKE_RULE_TARGET_MAP) {
                targets = bake_filelist_new(NULL, NULL);
                ut_try (!targets, NULL);
                ut_try (
                    bake_node_run_rule_map(driver, p, c, r, inputs, targets),
                    NULL);

            /* When rule specifies a pattern, generate targets from pattern */
            } else if (r->target.kind == BAKE_RULE_TARGET_PATTERN) {
                bool shouldBuild = false;

                if (!r->target.is.pattern || (r->target.is.pattern[0] == '$' && inherits)) {
                    targets = inherits;
                } else {
                    char *pattern = ut_strdup(r->target.is.pattern);
                    targets = bake_filelist_new(NULL, NULL);

                    char *tok = strtok(pattern, ",");
                    while (tok) {
                        bake_node *targetNode = bake_node_find(driver, &tok[1]);
                        if (!targetNode->cond || targetNode->cond(driver, c, p)) {
                            bake_filelist *list = bake_filelist_new(
                                NULL, ((bake_pattern*)targetNode)->pattern);
                            if (!list || !bake_filelist_count(list)) {
                                ut_trace(
                                   "no targets matched by '%s', need to rebuild '%s'",
                                    tok,
                                    n->name);
                                shouldBuild = true;
                            } else {
                                bake_filelist_addList(targets, list);
                                bake_filelist_free(list);
                            }
                        }
                        tok = strtok(NULL, ",");
                    }
                    free(pattern);
                }

                if (!targets && !bake_filelist_count(inputs)) {
                    ut_throw("no targets for rule");
                    goto error;
                }

                if (!targets) {
                    targets = bake_filelist_new(NULL, NULL);
                }

                ut_try (bake_node_run_rule_pattern(
                    driver, p, c, r, inputs, targets, shouldBuild), NULL);
            }
        }
    }

    /* Add targets to list of outputs (inputs for parent node) */
    if (outputs && targets) {
        bake_filelist_addList(outputs, targets);
    }

    ut_trace("done");
    ut_log_pop();

    return 0;
error:
    ut_log_pop();
    return -1;
}
