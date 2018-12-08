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

int bake_do_build(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *p)
{
    /* Step 2: export metadata to environment to make project discoverable */
    ut_try (bake_export_metadata(config, p), NULL);

    /* Step 3: parse driver configuration in project JSON */
    ut_try (bake_project_parse_driver_config(config, p), NULL);

    /* Step 4: parse dependee configuration */
    ut_try (bake_project_parse_dependee_config(config, p), NULL);

    /* Step 5: now that project config is fully loaded, check dependencies */
    ut_try (bake_project_check_dependencies(config, p), NULL);

    /* Step 6: invoke code generators, if any */
    ut_try (bake_project_generate(config, p), NULL);

    /* Step 7: clear environment of old project files */
    ut_try (bake_export_clear(config, p), NULL);

    /* Step 8: export project files to environment */
    ut_try (bake_export_prebuild(config, p), NULL);

    /* Step 9: build generated projects, in case code generation created any */
    ut_try (bake_project_build_generated(config, p), NULL);

    /* Step 10: build project */
    ut_try (bake_project_build(config, p), NULL);

    /* Step 11: install binary to environment */
    ut_try (bake_export_postbuild(config, p), NULL);

    return 0;
error:
    return -1;
}

int bake_do_clean(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *p)
{
    return 0;
}

int bake_do_rebuild(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *p)
{
    return 0;
}

int bake_do_export(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *p)
{
    return 0;
}

int bake_do_uninstall(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *p)
{
    return 0;
}

int bake_do_foreach(
    bake_config *config,
    bake_crawler *crawler,
    bake_project *p)
{
    return 0;
}
