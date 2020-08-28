/*
 *   These are the components we generate.
 */
#ifndef CUBECOORDS_H
struct cubecoords {
   int cpLex ;    /* corner permutation ordinal; 0..40319 */
   int coMask ;   /* corner orientation base-3; 0..6560 */
   int poIdxU ;   /* puzzle orientation up index; 0..5 or 7 */
   int epLex ;    /* edge permutation ordinal; 0..479001599 */
   int poIdxL ;   /* puzzle orientation left index; 0..3 */
   int moSupport ;/* center orientation support; 0..1 */
   int eoMask ;   /* edge orientation; 0..4095 */
   int moMask ;   /* center orientation; 0..4095 */
} ;
/*
 *   Routines in cubecoords.c
 */
extern unsigned char *tobytes11(const struct cubecoords *cc, unsigned char *p) ;
extern int frombytes11(const unsigned char *p, struct cubecoords *cc) ;
#define CUBECOORDS_H
#endif
