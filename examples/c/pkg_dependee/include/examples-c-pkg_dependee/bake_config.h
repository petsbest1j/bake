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

#ifndef EXAMPLES_C_PKG_DEPENDEE_BAKE_CONFIG_H
#define EXAMPLES_C_PKG_DEPENDEE_BAKE_CONFIG_H

/* Headers of public dependencies */
#include <examples_c_pkg_w_dependee.h>
#include <examples_c_pkg_helloworld.h>

/* Convenience macro for exporting symbols */
#ifndef examples_c_pkg_dependee_STATIC
#if examples_c_pkg_dependee_EXPORTS && (defined(_MSC_VER) || defined(__MINGW32__))
  #define EXAMPLES_C_PKG_DEPENDEE_API __declspec(dllexport)
#elif examples_c_pkg_dependee_EXPORTS
  #define EXAMPLES_C_PKG_DEPENDEE_API __attribute__((__visibility__("default")))
#elif defined _MSC_VER
  #define EXAMPLES_C_PKG_DEPENDEE_API __declspec(dllimport)
#else
  #define EXAMPLES_C_PKG_DEPENDEE_API
#endif
#else
  #define EXAMPLES_C_PKG_DEPENDEE_API
#endif

#endif

