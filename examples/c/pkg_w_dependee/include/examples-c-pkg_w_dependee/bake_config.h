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

#ifndef EXAMPLES_C_PKG_W_DEPENDEE_BAKE_CONFIG_H
#define EXAMPLES_C_PKG_W_DEPENDEE_BAKE_CONFIG_H

/* Generated includes are specific to the bake environment. If a project is not
 * built with bake, it will have to provide alternative methods for including
 * its dependencies. */
/* Headers of public dependencies */
/* No dependencies */

/* Headers of private dependencies */
#ifdef EXAMPLES_C_PKG_W_DEPENDEE_IMPL
/* No dependencies */
#endif

/* Convenience macro for exporting symbols */
#ifndef EXAMPLES_C_PKG_W_DEPENDEE_STATIC
  #if EXAMPLES_C_PKG_W_DEPENDEE_IMPL && (defined(_MSC_VER) || defined(__MINGW32__))
    #define EXAMPLES_C_PKG_W_DEPENDEE_API __declspec(dllexport)
  #elif EXAMPLES_C_PKG_W_DEPENDEE_IMPL
    #define EXAMPLES_C_PKG_W_DEPENDEE_API __attribute__((__visibility__("default")))
  #elif defined _MSC_VER
    #define EXAMPLES_C_PKG_W_DEPENDEE_API __declspec(dllimport)
  #else
    #define EXAMPLES_C_PKG_W_DEPENDEE_API
  #endif
#else
  #define EXAMPLES_C_PKG_W_DEPENDEE_API
#endif

#endif

