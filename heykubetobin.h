/*
 *   Routines exported.
 */
#ifndef HEYKUBETOBIN_H
#include "cubecoords.h"
extern int heykubeToComponents(const unsigned char *heykubePerm,
                               struct cubecoords *cc) ;
extern int componentsToHeykube(const struct cubecoords *cc,
                               unsigned char *heykubePerm) ;
#define HEYKUBETOBIN_H
#endif
