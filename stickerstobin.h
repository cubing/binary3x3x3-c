/*
 *   These are the components we generate.
 */
#ifndef STICKERSTOBIN_H
#include "cubecoords.h"
/*
 *   Routines exported.
 */
extern int stickersToComponents(const unsigned char *stickers,
                                struct cubecoords *cc) ;
extern int componentsToStickers(const struct cubecoords *cc,
                                unsigned char *stickers) ;
extern int encodePerm(const unsigned char *a, int n) ;
extern void decodePerm(int lex, unsigned char *a, int n) ;
#define STICKERSTOBIN_H
#endif
