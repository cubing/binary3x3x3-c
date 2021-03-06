/**
 *   Convert a sticker representation of the Rubik's cube into the
 *   3x3x3 binary representation, and back again.
 *
 *   Sticker representation:  54 values of 0..5; in row major order
 *   for each face, with the faces themselves in row major order,
 *   as follows:
 *
 *      U
 *    L F R B
 *      D
 *
 *   So the positions are as follows:
 *
 *               0  1  2
 *               3  4  5
 *               6  7  8
 *
 *    9 10 11   18 19 20   27 28 29   36 37 38
 *   12 13 14   21 22 23   30 31 32   39 40 41
 *   15 16 17   24 25 26   33 34 35   42 43 44
 *
 *              45 46 47
 *              48 49 50
 *              51 52 53
 *
 *   We give the cubie position in the Reid notation as follows.
 *   Edges first, then corners, then centers.
 *
UF UR UB UL DF DR DB DL FR FL BR BL UFR URB UBL ULF DRF DFL DLB DBR U L F R B D
 0  1  2  3  4  5  6  7  8  9 10 11  12  13  14  15  16  17  18  19 20  .... 25
 */
#include "cubecoords.h"
#include "stickerstobin.h"
#include "index.h"
#include "errors.h"
static const unsigned char ReidOrder[] = {
    7,19,  5,28,  1,37,  3,10            , // up edges
   46,25, 50,34, 52,43, 48,16,             // down edges
   23,30, 21,14, 39,32, 41,12,             // middle edges
    8,20,27,  2,29,36,  0,38,9,   6,11,18, // up corners
   47,33,26, 45,24,17, 51,15,44, 53,42,35, // down corners
   4, 13, 22, 31, 40, 49, } ;              // centers
/*
 *   To initialize, we want a table that goes from cubie coloring
 *   back to actual cubies and forward to cubie coloring.  We separate
 *   edges and corners here.
 */
#ifdef DYNAMIC_INITIALIZATION
static unsigned char edgeLookup[36] ;   // 2 colors -> index * 2 + ori
static unsigned char cornerLookup[36] ; // 2 colors -> index * 4 + ori
static unsigned char edgeExpand[24] ;   // index * 2 + ori -> 2 3-bit fields
static unsigned short cornerExpand[32] ; // index * 4 + ori -> 3 3-bit fields
static void initializeCubieTable() {
   if (edgeLookup[0] != 0) // only do the work once
      return ;
   for (int i=0; i<36; i++)
      edgeLookup[i] = cornerLookup[i] = 255 ;
   for (int i=0; i<24; i++)
      edgeExpand[i] = 0 ;
   for (int i=0; i<32; i++)
      cornerExpand[i] = 0 ;
   for (int i=0; i<12; i++) {
      int c0 = ReidOrder[2*i]/9 ;
      int c1 = ReidOrder[2*i+1]/9 ;
      edgeLookup[6*c0+c1] = 2*i ;
      edgeExpand[2*i] = (c0<<3)+c1 ;
      edgeLookup[6*c1+c0] = 2*i+1 ;
      edgeExpand[2*i+1] = (c1<<3)+c0 ;
   }
   for (int i=0; i<8; i++) {
      int c0 = ReidOrder[24+3*i]/9 ;
      int c1 = ReidOrder[24+3*i+1]/9 ;
      int c2 = ReidOrder[24+3*i+2]/9 ;
      cornerLookup[c0*6+c1] = 4*i ;
      cornerExpand[4*i] = (c0<<6)+(c1<<3)+c2 ;
      cornerLookup[c1*6+c2] = 4*i+1 ;
      cornerExpand[4*i+1] = (c1<<6)+(c2<<3)+c0 ;
      cornerLookup[c2*6+c0] = 4*i+2 ;
      cornerExpand[4*i+2] = (c2<<6)+(c0<<3)+c1 ;
   }
}
#else
/*
 *   We normally use static initialization.  The above routines
 *   generate the following tables.
 */
#define initializeCubieTable() // no-op the initialization routine
static const unsigned char edgeLookup[] = { 255, 6, 0, 2, 4, 255, 7, 255,
   19, 255, 23, 15, 1, 18, 255, 16, 255, 9, 3, 255, 17, 255, 21, 11, 5, 22,
   255, 20, 255, 13, 255, 14, 8, 10, 12, 255 } ;
static const unsigned char edgeExpand[] = { 2, 16, 3, 24, 4, 32, 1, 8, 42,
   21, 43, 29, 44, 37, 41, 13, 19, 26, 17, 10, 35, 28, 33, 12 } ;
static const unsigned char cornerLookup[] = { 255, 12, 0, 4, 8, 255, 10,
   255, 13, 255, 25, 22, 14, 21, 255, 1, 255, 18, 2, 255, 17, 255, 5, 30,
   6, 9, 255, 29, 255, 26, 255, 24, 20, 16, 28, 255 } ;
static const unsigned short cornerExpand[] = { 19, 152, 194, 0, 28, 224,
   259, 0, 33, 264, 68, 0, 10, 80, 129, 0, 346, 213, 171, 0, 337, 141, 106,
   0, 332, 101, 297, 0, 355, 285, 236, 0,} ;
#endif
/*
 *   From an array of cubie values, calculate the relevant
 *   permutations and orientations.  Ensure all needed cubies are seen.
 */
int stickersToComponents(const unsigned char *stickers, struct cubecoords *cc) {
   initializeCubieTable() ;
   unsigned char perm[12] ;
   int edgeo = 0 ;
   int cornero = 0 ;
   for (int i=0; i<54; i++)
      if (stickers[i] > 5)
         return STICKER_ELEMENT_OUT_OF_RANGE ;
   for (int i=0; i<12; i++) {
      int cubie = edgeLookup[6*stickers[ReidOrder[2*i]]+
                               stickers[ReidOrder[2*i+1]]] ;
      if (cubie == 255)
         return ILLEGAL_CUBIE_SEEN ;
      perm[i] = cubie >> 1 ;
      edgeo = 2 * edgeo + (cubie & 1) ;
   }
   int edgeperm = encodePerm(perm, 12) ;
   if (edgeperm < 0)
      return MISSING_EDGE_CUBIE ;
   for (int i=0; i<8; i++) {
      int cubie = cornerLookup[6*stickers[ReidOrder[3*i+24]]+
                                 stickers[ReidOrder[3*i+25]]] ;
      if (cubie == 255 ||
          (cornerExpand[cubie] & 7) != stickers[ReidOrder[3*i+26]])
         return ILLEGAL_CUBIE_SEEN ;
      perm[i] = (cubie >> 2) ;
      cornero = 3 * cornero + (cubie & 3) ;
   }
   int cornerperm = encodePerm(perm, 8) ;
   if (cornerperm < 0)
      return MISSING_CORNER_CUBIE ;
   for (int i=0; i<6; i++)
      perm[i] = stickers[ReidOrder[i+48]] ;
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
int componentsToStickers(const struct cubecoords *cc, unsigned char *stickers) {
   initializeCubieTable() ;
   unsigned char perm[12] ;
   int eo = cc->eoMask ;
   decodePerm(cc->epLex, perm, 12) ;
   for (int i=0; i<12; i++) {
      int colors = edgeExpand[2*perm[i]+(1&(eo>>(11-i)))] ;
      stickers[ReidOrder[2*i]] = colors >> 3 ;
      stickers[ReidOrder[2*i+1]] = colors & 7 ;
   }
   decodePerm(cc->cpLex, perm, 8) ;
   int co = cc->coMask ;
   for (int i=7; i>=0; i--) {
      int colors = cornerExpand[4*perm[i]+(co%3)] ;
      stickers[ReidOrder[3*i+24]] = colors >> 6 ;
      stickers[ReidOrder[3*i+25]] = (colors >> 3) & 7 ;
      stickers[ReidOrder[3*i+26]] = colors & 7 ;
      co /= 3 ;
   }
   for (int i=0; i<6; i++)
      stickers[ReidOrder[i+48]] = i ;
   return 0 ;
}
