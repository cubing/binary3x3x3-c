/**
 *   Convert the Reid representation of a cube state to the
 *   3x3x3 binary representation, and back again.
 */
#include <string.h>
#include "heykubetobin.h"
#include "index.h"
#include "errors.h"
static const char *solved =
   "UF UR UB UL DF DR DB DL FR FL BR BL UFR URB UBL ULF DRF DFL DLB DBR" ;
/*
 *   To initialize, we want a table that goes from cubie coloring
 *   back to actual cubies and forward to cubie coloring.  We separate
 *   edges and corners here.
 */
#ifdef DYNAMIC_INITIALIZATION
static unsigned char edgeLookup[64] ;    // 2 chars -> index * 2 + ori
static unsigned char cornerLookup[64] ;  // 2 chars -> index * 4 + ori
static unsigned short edgeExpand[24] ;   // index * 2 + ori -> 2 5-bit fields
static unsigned short cornerExpand[32] ; // index * 4 + ori -> 3 5-bit fields
static void initializeReidTable() {
   if (edgeLookup[0] != 0) // only do the work once
      return ;
   for (int i=0; i<64; i++)
      edgeLookup[i] = cornerLookup[i] = 255 ;
   for (int i=0; i<24; i++)
      edgeExpand[i] = 0 ;
   for (int i=0; i<32; i++)
      cornerExpand[i] = 0 ;
   for (int i=0; i<12; i++) {
      int c0 = solved[3*i]&31 ;
      int c1 = solved[3*i+1]&31 ;
      edgeLookup[(c0+15*c1)&63] = 2*i ;
      edgeExpand[2*i] = (c0<<5)+c1 ;
      edgeLookup[(c1+15*c0)&63] = 2*i+1 ;
      edgeExpand[2*i+1] = (c1<<5)+c0 ;
   }
   for (int i=0; i<8; i++) {
      int c0 = solved[36+4*i]&31 ;
      int c1 = solved[36+4*i+1]&31 ;
      int c2 = solved[36+4*i+2]&31 ;
      cornerLookup[(c0+15*c1)&63] = 4*i ;
      cornerExpand[4*i] = (c0<<10)+(c1<<5)+c2 ;
      cornerLookup[(c1+15*c2)&63] = 4*i+1 ;
      cornerExpand[4*i+1] = (c1<<10)+(c2<<5)+c0 ;
      cornerLookup[(c2+15*c0)&63] = 4*i+2 ;
      cornerExpand[4*i+2] = (c2<<10)+(c0<<5)+c1 ;
   }
/*
   for (int i=0; i<64; i++) printf(" %d", edgeLookup[i]) ;
   printf("\n") ;
   for (int i=0; i<64; i++) printf(" %d", cornerLookup[i]) ;
   printf("\n") ;
   for (int i=0; i<24; i++) printf(" %d", edgeExpand[i]) ;
   printf("\n") ;
   for (int i=0; i<32; i++) printf(" %d", cornerExpand[i]) ;
   printf("\n") ;
 */
}
#else
/*
 *   We normally use static initialization.  The above routines
 *   generate the following tables.
 */
#define initializeReidTable() // no-op the initialization routine
static const unsigned char edgeLookup[] = { 255, 1, 9, 255, 255, 255, 255,
   7, 15, 6, 255, 255, 255, 3, 11, 255, 20, 255, 10, 255, 16, 255, 255,
   255, 255, 255, 255, 255, 255, 255, 8, 255, 255, 255, 12, 2, 255, 255,
   19, 255, 255, 255, 23, 255, 17, 255, 255, 0, 21, 255, 255, 4, 255, 255,
   22, 255, 14, 255, 18, 255, 255, 5, 13, 255 } ;
static const unsigned char cornerLookup[] = {255, 14, 18, 255, 255, 255,
   255, 10, 22, 12, 255, 255, 255, 2, 30, 255, 29, 255, 16, 255, 1, 255,
   255, 255, 255, 255, 255, 255, 255, 255, 20, 255, 255, 255, 28, 4, 255,
   255, 13, 255, 255, 255, 25, 255, 17, 255, 255, 0, 5, 255, 255, 8, 255,
   255, 9, 255, 24, 255, 21, 255, 255, 6, 26, 255 } ;
static const unsigned short edgeExpand[] = {678, 213, 690, 597, 674, 85,
   684, 405, 134, 196, 146, 580, 130, 68, 140, 388, 210, 582, 204, 390, 82,
   578, 76, 386 } ;
static const unsigned short cornerExpand[] = {21714, 6741, 19110, 0, 22082,
   18517, 2738, 0, 21580, 2453, 12962, 0, 21894, 12501, 6828, 0, 4678,
   18628, 6290, 0, 4300, 6532, 12422, 0, 4482, 12356, 2188, 0, 4178, 2628,
   18562, 0 } ;
#endif
/*
 *   From an array of cubie values, calculate the relevant
 *   permutations and orientations.  Ensure all needed cubies are seen.
 */
int ReidToComponents(const char *Reid, struct cubecoords *cc) {
   initializeReidTable() ;
   unsigned char perm[12] ;
   int edgeo = 0 ;
   int cornero = 0 ;
   if (strlen(solved) != strlen(Reid))
      return WRONG_REID_LENGTH ;
   for (int i=0; solved[i]; i++)
      if (solved[i] != Reid[i] && (solved[i] == ' ' ||
           (Reid[i] != 'U' && Reid[i] != 'F' && Reid[i] != 'R' &&
            Reid[i] != 'D' && Reid[i] != 'B' && Reid[i] != 'L')))
         return REID_ELEMENT_OUT_OF_RANGE ;
   for (int i=0; i<12; i++) {
      int cubie = edgeLookup[(Reid[3*i]+15*Reid[3*i+1])&63] ;
      if (cubie == 255 ||
          edgeExpand[cubie] != ((Reid[3*i]&31) << 5) + (Reid[3*i+1]&31))
         return ILLEGAL_CUBIE_SEEN ;
      perm[i] = cubie >> 1 ;
      edgeo = 2 * edgeo + (cubie & 1) ;
   }
   int edgeperm = encodePerm(perm, 12) ;
   if (edgeperm < 0)
      return MISSING_EDGE_CUBIE ;
   for (int i=0; i<8; i++) {
      int cubie = cornerLookup[(Reid[36+4*i]+15*Reid[37+4*i])&63] ;
      if (cubie == 255 || cornerExpand[cubie] != ((Reid[36+4*i]&31) << 10) +
                               ((Reid[37+4*i]&31) << 5) + (Reid[38+4*i]&31))
         return ILLEGAL_CUBIE_SEEN ;
      perm[i] = (cubie >> 2) ;
      cornero = 3 * cornero + (cubie & 3) ;
   }
   int cornerperm = encodePerm(perm, 8) ;
   if (cornerperm < 0)
      return MISSING_CORNER_CUBIE ;
   cc->cpLex = cornerperm ;
   cc->coMask = cornero ;
   cc->poIdxU = 7 ;
   cc->epLex = edgeperm ;
   cc->poIdxL = 0 ;
   cc->moSupport = 0 ;
   cc->eoMask = edgeo ;
   cc->moMask = 0 ;
   return 0 ;
}
int componentsToReid(const struct cubecoords *cc, char *Reid) {
   initializeReidTable() ;
   unsigned char perm[12] ;
   for (int i=0; solved[i]; i++)
      Reid[i] = ' ' ;
   Reid[strlen(solved)] = 0 ;
   int eo = cc->eoMask ;
   decodePerm(cc->epLex, perm, 12) ;
   for (int i=0; i<12; i++) {
      int colors = edgeExpand[2*perm[i]+(1&(eo>>(11-i)))] ;
      Reid[3*i] = '@'+(colors>>5) ;
      Reid[3*i+1] = '@'+(colors&31) ;
   }
   decodePerm(cc->cpLex, perm, 8) ;
   int co = cc->coMask ;
   for (int i=7; i>=0; i--) {
      int colors = cornerExpand[4*perm[i]+(co%3)] ;
      Reid[36+4*i] = '@'+(colors>>10) ;
      Reid[37+4*i] = '@'+((colors>>5)&31) ;
      Reid[38+4*i] = '@'+(colors&31) ;
      co /= 3 ;
   }
   return 0 ;
}
