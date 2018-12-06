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

#ifndef UT_LOAD_H_
#define UT_LOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ut_load_cb)(char *file, int argc, char* argv[], void* userData);

/** Load a resource.
 * The ut_use function provides a single interface to loading files or
 * packages into corto. The function accepts any filetype known to corto (types
 * are registered as packages in the driver/ext scope).
 *
 * When a provided resource identifier contains a '.', the string after the '.'
 * is treated as a file extension, and `ut_use` will treat the identifier as
 * a filename. If the identifier contains multiple dots, the last dot will be
 * used to extract the extension.
 *
 * If the identifier contains no dots, `ut_use` will treat the identifier as
 * a logical package name. In this case, `ut_use` will use `ut_locate` to
 * find the package image. Each identifier will only be looked up once, after
 * which the result is cached.
 *
 * The argc and argv parameters can be optionally passed to the resource if the
 * resource type supports it. This is most commonly used for passing parameters
 * to the `cortomain` function of packages.
 *
 * If a resource is found, the handler for the resource type will be executed.
 * This mechanism is used by library loader to invoke the `cortomain` function,
 * but can also involve loading an XML file or executing a corto script,
 * depending on how the resource type is implemented.
 *
 * Resource types are regular packages located in the `driver/ext` scope. They
 * need to call `ut_load_register` in their `cortomain` function to register
 * the resource type handler, which determines how a resource is loaded.
 *
 * @param identifier The resource identifier (either a file or a package)
 * @param argc The number of arguments to pass to the resource
 * @param argv The arguments to pass to the resource (array must be NULL terminated).
 * @return Zero if success, nonzero if failed.
 * @see ut_locate ut_load_register
 */
UT_EXPORT
int ut_use(
    const char *identifier,
    int argc,
    char *argv[]);

/** Execute a resource.
 * The same as ut_use, but with the difference that each time ut_run is
 * ran, the cortomain routine (or equivalent) of the resource is invoked.
 *
 * @param identifier The resource identifier (either a file or a package)
 * @param argc The number of arguments to pass to the resource
 * @param argv The arguments to pass to the resource (array must be NULL terminated).
 * @return Zero if success, nonzero if failed.
 * @see ut_locate ut_load_register
 */
UT_EXPORT
int ut_run(
    const char *identifier,
    int argc,
    char *argv[]);

typedef enum ut_locate_kind {
    UT_LOCATE_ENV,     /* environment, for example: /usr/local */
    UT_LOCATE_LIB,     /* full path to library */
    UT_LOCATE_APP,     /* full path to application */
    UT_LOCATE_BIN,     /* full path to either library or application */
    UT_LOCATE_ETC,     /* full path to project etc directory */
    UT_LOCATE_INCLUDE, /* full path to project include directory */
    UT_LOCATE_PACKAGE  /* full path to project directory */
} ut_locate_kind;

/** Find project locations in the package hierarchy.
 * The `ut_locate` function can be used to locate a package on disk, and to
 * obtain information about the various locations associated with a package.
 *
 * The function accepts a logical package identifier. Local package identifiers
 * are the same across platforms and are equal to the identifier of the package
 * object once loaded into the corto object store. A package identifier is
 * formatted like any other object identifier, in the form `/foo/bar`. The
 * initial `/` is optional.
 *
 * The function looks in three locations, `$BAKE_HOME`, `$BAKE_TARGET` and
 * the global package repository (`/usr/local` on Linux based systems). From
 * these locations `ut_locate` selects the newest version of the package it can
 * find. It also performs a test to ensure that the package has been compiled
 * with the same corto library.
 *
 * A package is only located once. After the first lookup, the result is cached.
 * That means that if a newer version of a package is installed while applications
 * are running, these applications will have to be restarted to see the new
 * package if it is in a different location.
 *
 * If the package has already been loaded by `ut_use` or equivalent function,
 * the function returns a pointer to the library object through the `dl_out`
 * parameter.
 *
 * @param package A logical package name.
 * @param dl_out Will be set to library object, if already loaded.
 * @param kind Specify which information should be obtained from package.
 * @return The requested information.
 */
UT_EXPORT
const char* ut_locate(
    const char *package,
    ut_dl *dl_out,
    ut_locate_kind kind);

/** Load a resource from a library.
 * The `ut_load_sym` function looks up a function or global variable from a
 * package. The function takes a logical package identifier which is internally
 * passed to `ut_lookup` to find the corresponding library.
 *
 * If `dl_out` is set to an existing library object, the `package` parameter is
 * ignored and the function will perform a lookup on the library directly. If
 * `dl_out` has no been set, the function will lookup the library, and assign the
 * `dl_out` parameter to the library object, if found. This facilitates doing
 * efficient repeated lookups on the same package, while also allowing for doing
 * lookups on non-package libraries (pass NULL to `package` and specify a
 * library for `dl_out`).
 *
 * The symbol must point to a global symbol. Different platforms have different
 * rules for which symbols are made visible. If using a corto package, you can
 * use the <package name>_EXPORT macro to make symbols globally visible.
 *
 * @param package A logical package identifier.
 * @param dl_out Cached pointer to library to avoid doing repeated `ut_lookup`'s
 * @param symbol Name of the symbol to lookup.
 */
UT_EXPORT
void* ut_load_sym(
    char *package,
    ut_dl *dl_out,
    char *symbol);

/** Same as ut_load_sym, but for procedures.
 *
 * @see ut_load_sym
 */
UT_EXPORT
void (*ut_load_proc(
    char *package,
    ut_dl *dl_out,
    char *symbol))(void);

/** Register a load action.
 * The `ut_load_register` function registers a load action that needs to be
 * invoked when loading a resource of a specified extension.
 *
 * To load a file of a specified extension, corto will first look in the
 * `driver/ext` scope if a package with the extension name can be found. If it
 * has found one, it will load the package library as an ordinary package, which
 * means the `cortomain` will be invoked.
 *
 * In the `cortomain` function, the package should call `ut_load_register` to
 * register the load action with corto for the specific extension. If a package
 * in the `driver/ext` scope did not register the extension after running the
 * `cortomain` function, an error will be thrown.
 *
 * The load action specifies how to load a file of a specific extension. For
 * example, a library will be loaded with `dlopen` while an XML file will be
 * loaded as text file, and is then parsed with an XML parser.
 *
 * @param ext The extension for which to register the load action.
 * @param action The callback to invoke when loading a file of the specified extension
 * @param userData Data passed to the callback.
 * @return Zero if success, non-zero if failed.
 */
UT_EXPORT
int ut_load_register(
    char *ext,
    ut_load_cb action,
    void* userData);

/* Internal function for initializing paths in loader */
UT_EXPORT
void ut_load_init(
    const char *target,
    const char *home,
    const char *target_config,
    const char *home_config,
    const char *build,
    bool standalone);

UT_EXPORT
void ut_load_deinit(void);

UT_EXPORT
const char* ut_load_homePath(void);

UT_EXPORT
const char* ut_load_targetPath(void);

UT_EXPORT
const char* ut_load_homeMetaPath(void);

UT_EXPORT
const char* ut_load_targetMetaPath(void);

#ifdef __cplusplus
}
#endif

#endif
