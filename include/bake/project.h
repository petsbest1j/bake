/* Copyright (c) 2010-2019 the corto developers
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

/** @file
 * @section project Project types
 * @brief Project settings loaded from project.json.
 **/

#ifndef BAKE_PROJECT_H_
#define BAKE_PROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum bake_project_type {
    BAKE_APPLICATION = 1,   /* Executable project in bake package store */
    BAKE_PACKAGE = 2,       /* Library project in bake package store */
    BAKE_TOOL = 3,          /* Executable project accessible globally */
    BAKE_TEMPLATE = 4       /* Template project */
} bake_project_type;

/* Driver required by project with its configuration */
typedef struct bake_project_driver {
    const char *id;
    bake_driver *driver;
    void *json;
    ut_ll attributes;
} bake_project_driver;

/* Bind bake project to a repository */
typedef struct bake_project_repository {
    char *id;
    char *url;
} bake_project_repository;

/* Identifies a specific revision in the repository */
typedef struct bake_ref {
    bake_project_repository *repository;
    char *branch;
    char *commit;
    char *tag;
} bake_ref;

/* Default values for revision to use by refs in bundle */
typedef struct bake_project_bundle_defaults {
    char *branch;
    char *tag;
} bake_project_bundle_defaults;

/* Bundle containing repository revisions */
typedef struct bake_project_bundle {
    char *id;
    bake_project_bundle_defaults defaults;
    ut_rb refs;
} bake_project_bundle;

struct bake_project {
    /* Project properties (managed by bake core) */
    char *path;             /* Project path */
    char *fullpath;         /* Project path from root */
    char *id;               /* Project id */
    bake_project_type type; /* Project kind */
    char *version;          /* Project version */
    char *repository;       /* Project repository */
    char *license;          /* Project license */
    char *author;           /* Project author */
    char *organization;     /* Project organization */
    char *description;      /* Project description */
    char *language;         /* Project programming language */
    bool public;            /* Is package public or private */
    bool coverage;          /* Include in coverage analysis (default = true) */
    ut_ll use;              /* Project dependencies */
    ut_ll use_private;      /* Local dependencies (not visible to dependees) */
    ut_ll use_build;        /* Packages only required by the build */
    ut_ll use_runtime;      /* Packages required when running the project */
    ut_ll link;             /* All resolved dependencies package must link with */
    ut_ll sources;          /* Paths to source files */
    ut_ll includes;         /* Paths to include files */
    bool keep_binary;       /* Keep artefact when cleaning project */
    char *dependee_json;    /* Build instructions for dependees */

    char *default_host;     /* Optional default git repository host */
    ut_ll use_bundle;       /* Global bundle dependencies */
    ut_rb repositories;     /* Repositories in bundle */
    ut_rb bundles;          /* Repository references in bundle */

    ut_ll drivers;          /* Drivers used to build this project */
    bake_project_driver *language_driver; /* Driver loaded for the language */

    char *artefact;         /* Name of artefact generated by project */
    char *artefact_path;    /* Project path where artefact will be stored */
    char *artefact_file;    /* Project path including artefact name */
    char *bin_path;         /* Project bin path (not including platform) */
    char *cache_path;       /* Project path containing temporary build files */
    char *id_underscore;    /* Id with underscores instead of dots */
    char *id_dash;          /* Id with dashes instead of dots */
    char *id_base;          /* Last element of id */
    bool bake_extension;    /* Is package a bake extension (install to HOME) */

    /* Direct access to the parson JSON data */
    void *json;

    /* Runtime status (managed by driver) */
    bool error;
    bool freshly_baked;
    bool changed;

    /* Should project be rebuilt (managed by bake action) */
    bool artefact_outdated;
    bool sources_outdated;

    /* Dependency administration (managed by crawler) */
    int unresolved_dependencies; /* number of dependencies still to be built */
    ut_ll dependents; /* projects that depend on this project */
    bool built;

    /* filelist with generated sources (set before build) */
    void *generated_sources;

    /* Files to be cleaned other than objects and artefact (populated by
     * language binding) */
    ut_ll files_to_clean;
};

#ifdef __cplusplus
}
#endif

#endif
