/*
                                   )
                                  (.)
                                  .|.
                                  | |
                              _.--| |--._
                           .-';  ;`-'& ; `&.
                          \   &  ;    &   &_/
                           |"""---...---"""|
                           \ | | | | | | | /
                            `---.|.|.|.---'

 * This file is generated by bake.lang.c for your convenience. Headers of
 * dependencies will automatically show up in this file. Include bake_config.h
 * in your main project file. Do not edit! */

#ifndef EXAMPLES_C_APP_CLIB_BAKE_CONFIG_H
#define EXAMPLES_C_APP_CLIB_BAKE_CONFIG_H

/* Generated includes are specific to the bake environment. If a project is not
 * built with bake, it will have to provide alternative methods for including
 * its dependencies. */
#ifdef __BAKE__
/* Headers of public dependencies */
/* No dependencies */

/* Headers of private dependencies */
#ifdef EXAMPLES_C_APP_CLIB_IMPL
/* No dependencies */
#endif
#endif

/* Convenience macro for exporting symbols */
#ifndef EXAMPLES_C_APP_CLIB_STATIC
  #if EXAMPLES_C_APP_CLIB_IMPL && defined _MSC_VER
    #define EXAMPLES_C_APP_CLIB_EXPORT __declspec(dllexport)
  #elif EXAMPLES_C_APP_CLIB_IMPL
    #define EXAMPLES_C_APP_CLIB_EXPORT __attribute__((__visibility__("default")))
  #elif defined _MSC_VER
    #define EXAMPLES_C_APP_CLIB_EXPORT __declspec(dllimport)
  #else
    #define EXAMPLES_C_APP_CLIB_EXPORT
  #endif
#else
  #define EXAMPLES_C_APP_CLIB_EXPORT
#endif

#endif

