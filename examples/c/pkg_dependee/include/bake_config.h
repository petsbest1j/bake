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

#ifndef PKG_DEPENDEE_BAKE_CONFIG_H
#define PKG_DEPENDEE_BAKE_CONFIG_H

/* Headers of public dependencies */
#include <pkg_w_dependee>
#include <pkg_helloworld>

/* Headers of private dependencies */
#ifdef PKG_DEPENDEE_IMPL
/* No dependencies */
#endif

/* Convenience macro for exporting symbols */
#if PKG_DEPENDEE_IMPL && defined _MSC_VER
#define PKG_DEPENDEE_EXPORT __declspec(dllexport)
#elif PKG_DEPENDEE_IMPL
#define PKG_DEPENDEE_EXPORT __attribute__((__visibility__("default")))
#elif defined _MSC_VER
#define PKG_DEPENDEE_EXPORT __declspec(dllimport)
#else
#define PKG_DEPENDEE_EXPORT
#endif

#endif

