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

#ifndef EXAMPLES_C_PKG_HELLOWORLD_BAKE_CONFIG_H
#define EXAMPLES_C_PKG_HELLOWORLD_BAKE_CONFIG_H

/* Headers of public dependencies */
/* No dependencies */

/* Convenience macro for exporting symbols */
#ifndef examples_c_pkg_helloworld_STATIC
#if defined(examples_c_pkg_helloworld_EXPORTS) && (defined(_MSC_VER) || defined(__MINGW32__))
  #define EXAMPLES_C_PKG_HELLOWORLD_API __declspec(dllexport)
#elif defined(examples_c_pkg_helloworld_EXPORTS)
  #define EXAMPLES_C_PKG_HELLOWORLD_API __attribute__((__visibility__("default")))
#elif defined(_MSC_VER)
  #define EXAMPLES_C_PKG_HELLOWORLD_API __declspec(dllimport)
#else
  #define EXAMPLES_C_PKG_HELLOWORLD_API
#endif
#else
  #define EXAMPLES_C_PKG_HELLOWORLD_API
#endif

#endif

