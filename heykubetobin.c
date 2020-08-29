/**
 *   Convert a permutation representation of the Rubik's cube into the
 *   3x3x3 binary representation, and back again.
 *
 *   Permutation representation:  54 values of 0..53; in column major order
 *   for each face, with the faces themselves in the order L F R B U D.
 *
 *      U
 *    L F R B
 *      D
 *
 *   So the positions are as follows:
 *
 *              36 39 42
 *              37 40 43
 *              38 41 44
 *
 *    0  3  6    9 12 15   18 21 24   27 30 33
 *    1  4  7   10 13 16   19 22 25   28 31 34
 *    2  5  8   11 14 17   20 23 26   29 32 35
 *
 *              45 48 51
 *              46 49 52
 *              47 50 53
 *
 *   We give the cubie position in the Reid notation as follows.
 *   Edges first, then corners, then centers.
 *
UF UR UB UL DF DR DB DL FR FL BR BL UFR URB UBL ULF DRF DFL DLB DBR U L F R B D
 0  1  2  3  4  5  6  7  8  9 10 11  12  13  14  15  16  17  18  19 20  .... 25
 */
#include "cubecoords.h"
#include "index.h"
#include "heykubetobin.h"
#include "errors.h"
static const unsigned char ReidOrder[] = {
   41,12, 43,21, 39,30, 37,3,              // up edges
   48,14, 52,23, 50,32, 46,5,              // down edges
   16,19, 10,7,  28,25, 34,1,              // middle edges
   44,15,18, 42,24,27, 36,33,0,  38,6,9,   // up corners
   51,20,17, 45,11,8,  47,2,35,  53,29,26, // down corners
   4, 13, 22, 31, 40, 49 } ;              // centers
/*
 *   To initialize, we want a table that goes from cubie coloring
 *   back to actual cubies and forward to cubie coloring.  We separate
 *   edges and corners here.
 */
#ifdef DYNAMIC_INITIALIZATION
static unsigned char edgeLookup[36] ;   // 2 colors -> index * 2 + ori
static unsigned char cornerLookup[36] ; // 2 colors -> index * 4 + ori
static unsigned short edgeExpand[24] ;  // index * 2 + ori -> 2 6-bit fields
static int cornerExpand[32] ;           // index * 4 + ori -> 3 6-bit fields
static void initializeHeyKubeTable() {
   if (edgeLookup[0] != 0) // only do the work once
      return ;
   for (int i=0; i<36; i++)
      edgeLookup[i] = cornerLookup[i] = 255 ;
   for (int i=0; i<24; i++)
      edgeExpand[i] = 0 ;
   for (int i=0; i<32; i++)
      cornerExpand[i] = 0 ;
   for (int i=0; i<12; i++) {
      int c0 = ReidOrder[2*i] ;
      int c1 = ReidOrder[2*i+1] ;
      edgeLookup[6*(c0/9)+c1/9] = 2*i ;
      edgeExpand[2*i] = (c0<<6)+c1 ;
      edgeLookup[6*(c1/9)+c0/9] = 2*i+1 ;
      edgeExpand[2*i+1] = (c1<<6)+c0 ;
   }
   for (int i=0; i<8; i++) {
      int c0 = ReidOrder[24+3*i] ;
      int c1 = ReidOrder[24+3*i+1] ;
      int c2 = ReidOrder[24+3*i+2] ;
      cornerLookup[c0/9*6+c1/9] = 4*i ;
      cornerExpand[4*i] = (c0<<12)+(c1<<6)+c2 ;
      cornerLookup[c1/9*6+c2/9] = 4*i+1 ;
      cornerExpand[4*i+1] = (c1<<12)+(c2<<6)+c0 ;
      cornerLookup[c2/9*6+c0/9] = 4*i+2 ;
      cornerExpand[4*i+2] = (c2<<12)+(c0<<6)+c1 ;
   }
/*   dump the created tables, to create the static tables below
   for (int i=0; i<36; i++) printf(" %d", edgeLookup[i]) ;
   printf("\n") ;
   for (int i=0; i<36; i++) printf(" %d", cornerLookup[i]) ;
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
#define initializeHeyKubeTable() // no-op the initialization routine
static const unsigned char edgeLookup[] = { 255, 19, 255, 23, 7, 15, 18,
   255, 16, 255, 1, 9, 255, 17, 255, 21, 3, 11, 22, 255, 20, 255, 5, 13, 6,
   0, 2, 4, 255, 255, 14, 8, 10, 12, 255, 255 } ;
static const unsigned char cornerLookup[] = { 255, 13, 255, 25, 10, 22, 21,
   255, 1, 255, 14, 18, 255, 17, 255, 5, 2, 30, 9, 255, 29, 255, 6, 26, 12,
   0, 4, 8, 255, 255, 24, 20, 16, 28, 255, 255 } ;
static const unsigned short edgeExpand[] = { 2636, 809, 2773, 1387, 2526,
   1959, 2371, 229, 3086, 944, 3351, 1524, 3232, 2098, 2949, 366, 1043,
   1232, 647, 458, 1817, 1628, 2177, 98 } ;
static const int cornerExpand[] = { 181202, 62636, 76559, 0, 173595,
   100074, 113304, 0, 149568, 135204, 2337, 0, 156041, 25190, 39302, 0,
   210193, 83059, 72916, 0, 185032, 45613, 35659, 0, 192675, 10479, 146370,
   0, 218970, 120501, 109917, 0 } ;
#endif
/*
 *   From an array of cubie values, calculate the relevant
 *   permutations and orientations.  Ensure all needed cubies are seen.
 */
int heykubeToComponents(const unsigned char *kubeperm, struct cubecoords *cc) {
   initializeHeyKubeTable() ;
   unsigned char perm[12] ;
   int edgeo = 0 ;
   int cornero = 0 ;
   for (int i=0; i<54; i++)
      if (kubeperm[i] > 53)
         return PERM_ELEMENT_OUT_OF_RANGE ;
   for (int i=0; i<12; i++) {
      int cubie = edgeLookup[6*(kubeperm[ReidOrder[2*i]]/9)+
                                kubeperm[ReidOrder[2*i+1]]/9] ;
      if (cubie == 255 || edgeExpand[cubie] !=
                    (kubeperm[ReidOrder[2*i]]<<6)+kubeperm[ReidOrder[2*i+1]])
         return ILLEGAL_CUBIE_SEEN ;
      perm[i] = cubie >> 1 ;
      edgeo = 2 * edgeo + (cubie & 1) ;
   }
   int edgeperm = encodePerm(perm, 12) ;
   if (edgeperm < 0)
      return MISSING_EDGE_CUBIE ;
   for (int i=0; i<8; i++) {
      int cubie = cornerLookup[6*(kubeperm[ReidOrder[3*i+24]]/9)+
                                  kubeperm[ReidOrder[3*i+25]]/9] ;
      if (cubie == 255 || cornerExpand[cubie] !=
          (kubeperm[ReidOrder[3*i+24]]<<12)+(kubeperm[ReidOrder[3*i+25]]<<6)+
           kubeperm[ReidOrder[3*i+26]])
         return ILLEGAL_CUBIE_SEEN ;
      perm[i] = (cubie >> 2) ;
      cornero = 3 * cornero + (cubie & 3) ;
   }
   int cornerperm = encodePerm(perm, 8) ;
   if (cornerperm < 0)
      return MISSING_CORNER_CUBIE ;
   for (int i=0; i<6; i++)
      perm[i] = kubeperm[ReidOrder[i+48]]/9 ;
   if (encodePerm(perm, 6) != 0)
      return PUZZLE_ORIENTATION_NOT_SUPPORTED ;
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
int componentsToHeykube(const struct cubecoords *cc, unsigned char *kubeperm) {
   initializeHeyKubeTable() ;
   unsigned char perm[12] ;
   int eo = cc->eoMask ;
   decodePerm(cc->epLex, perm, 12) ;
   for (int i=0; i<12; i++) {
      int colors = edgeExpand[2*perm[i]+(1&(eo>>(11-i)))] ;
      kubeperm[ReidOrder[2*i]] = colors >> 6 ;
      kubeperm[ReidOrder[2*i+1]] = colors & 63 ;
   }
   decodePerm(cc->cpLex, perm, 8) ;
   int co = cc->coMask ;
   for (int i=7; i>=0; i--) {
      int colors = cornerExpand[4*perm[i]+(co%3)] ;
      kubeperm[ReidOrder[3*i+24]] = colors >> 12 ;
      kubeperm[ReidOrder[3*i+25]] = (colors >> 6) & 63 ;
      kubeperm[ReidOrder[3*i+26]] = colors & 63 ;
      co /= 3 ;
   }
   for (int i=0; i<6; i++)
      kubeperm[ReidOrder[i+48]] = ReidOrder[i+48] ;
   return 0 ;
}
