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
#include "stickerstobin.h"
static const unsigned char ReidOrder[] = {
    7,19,  5,28,  1,37,  3,10, // up edges
   46,25, 50,34, 52,43, 48,16, // down edges
   23,30, 21,14, 39,32, 41,12, // middle edges
    8,20,27,  2,29,36,  0,38,9,  6,11,18, // up corners
   47,33,26, 45,24,17, 51,15,44, 53,42,35, // down corners
   4, 13, 22, 31, 40, 49, } ;  // centers
/*
 *   A solved and oriented cube looks like this.
 */
static const unsigned char solved[] = {
  0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1, 2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4, 5,5,5,5,5,5,5,5,5,
} ;
/*
 *   To initialize, we want a table that goes from cubie coloring
 *   back to actual cubies.  A -1 value means invalid.  We store
 *   the rotations here too, to help us get orientation later.
 *   The max value we need to store is 01555 base 6 which is 431
 *   (although this will never be a valid cube).  The lowest two
 *   bits are the orientation; the next five bits are the cubie
 *   index.  The value 255 means illegal.
 */
static unsigned char cubieLookup[432] ;
static int cubieExpand[104] ;
static void initializeCubieTable() {
   if (cubieExpand[0] != 0) // only do the work once
      return ;
   for (int i=0; i<sizeof(cubieLookup)/sizeof(cubieLookup[0]); i++)
      cubieLookup[i] = 255 ;
   for (int i=0; i<sizeof(cubieExpand)/sizeof(cubieExpand[0]); i++)
      cubieExpand[i] = -1 ;
   for (int i=0; i<12; i++) {
      int c0 = solved[ReidOrder[2*i]] ;
      int c1 = solved[ReidOrder[2*i+1]] ;
      int v = 6*c0+c1;
      cubieLookup[36+v] = 4*i ;
      cubieExpand[4*i] = v ;
      v = 6*c1+c0;
      cubieLookup[36+v] = 4*i+1 ;
      cubieExpand[4*i+1] = v ;
   }
   for (int i=12; i<20; i++) {
      int c0 = solved[ReidOrder[3*i-12]] ;
      int c1 = solved[ReidOrder[3*i-11]] ;
      int c2 = solved[ReidOrder[3*i-10]] ;
      int v = 36*c0+6*c1+c2;
      cubieLookup[216+v] = 4*i ;
      cubieExpand[4*i] = v ;
      v = 36*c2+6*c0+c1 ;
      cubieLookup[216+v] = 4*i+1 ;
      cubieExpand[4*i+1] = v ;
      v = 36*c1+6*c2+c0 ;
      cubieLookup[216+v] = 4*i+2 ;
      cubieExpand[4*i+2] = v ;
   }
   for (int i=20; i<26; i++) {
      int v = solved[ReidOrder[i+28]] ;
      cubieLookup[6+v] = 4*i ;
      cubieExpand[4*i] = v ;
   }
}
/*
 *   Index a permutation.  Return -1 if all values are not seen.
 *   Zero based.
 */
static int encodePerm(const unsigned char *a, int n) {
   int bits = 0 ;
   for (int i=0; i<n; i++)
      bits |= 1<<a[i] ;
   if (bits + 1 != 1 << n)
      return -1 ;
   int r = 0 ;
   for (int i=0; i<n; i++) {
      r = r * (n-i) ;
      for (int j=i+1; j<n; j++)
         if (a[i] > a[j])
            r++ ;
   }
   return r ;
}
/*
 *   Unindex a perm.
 */
static void decodePerm(int lex, unsigned char *a, int n) {
   a[n-1] = 0 ;
   for (int i=n-2; i>=0; i--) {
      a[i] = lex % (n - i) ;
      lex /= n-i ;
      for (int j=i+1; j<n; j++)
         if (a[j] >= a[i])
            a[j]++ ;
   }
}
/*
 *   From an array of cubie values, calculate the relevant
 *   permutations and orientations.  Ensure all needed cubies are seen.
 */
int stickersToComponents(const unsigned char *stickers, struct cubecoords *cc) {
   initializeCubieTable() ;
   unsigned char edgep[12], cornerp[12], centerp[6] ;
   int edgeo = 0 ;
   int cornero = 0 ;
   for (int i=0; i<12; i++)
      edgep[i] = 255 ;
   for (int i=0; i<8; i++)
      cornerp[i] = 255 ;
   for (int i=0; i<6; i++)
      centerp[i] = 255 ;
   for (int i=0; i<54; i++)
      if (stickers[i] > 5)
         return STICKER_ELEMENT_OUT_OF_RANGE ;
   for (int i=0; i<12; i++) {
      int cubie = cubieLookup[36+6*stickers[ReidOrder[2*i]]+
                                   stickers[ReidOrder[2*i+1]]] ;
      if (cubie == 255)
         return ILLEGAL_CUBIE_SEEN ;
      edgep[i] = cubie >> 2 ;
      edgeo = 2 * edgeo + (cubie & 3) ;
   }
   for (int i=12; i<20; i++) {
      int cubie = cubieLookup[216+36*stickers[ReidOrder[3*i-12]]+
                                   6*stickers[ReidOrder[3*i-11]]+
                                     stickers[ReidOrder[3*i-10]]] ;
      if (cubie == 255)
         return ILLEGAL_CUBIE_SEEN ;
      cornerp[i-12] = (cubie >> 2) - 12 ;
      cornero = 3 * cornero + (cubie & 3) ;
   }
   for (int i=20; i<26; i++)
      centerp[i-20] = (cubieLookup[6+stickers[ReidOrder[i+28]]] >> 2) - 20 ;
   int cornerperm = encodePerm(cornerp, 8) ;
   if (cornerperm < 0)
      return MISSING_CORNER_CUBIE ;
   int edgeperm = encodePerm(edgep, 12) ;
   if (edgeperm < 0)
      return MISSING_EDGE_CUBIE ;
   if (encodePerm(centerp, 6) != 0)
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
unsigned char *tobytes11(const struct cubecoords *cc, unsigned char *p) {
   unsigned char *r = p ;
   *p++ = cc->cpLex >> 8 ;
   *p++ = cc->cpLex ;
   *p++ = cc->coMask >> 5 ;
   *p++ = (cc->coMask << 3) + cc->poIdxU ;
   *p++ = (cc->epLex >> 21) ;
   *p++ = (cc->epLex >> 13) ;
   *p++ = (cc->epLex >> 5) ;
   *p++ = (cc->epLex << 3) ;
   *p++ = (cc->eoMask >> 4) ;
   *p++ = (cc->eoMask << 4) ;
   *p++ = 0 ;
   return r ;
}
int frombytes11(const unsigned char *p, struct cubecoords *cc) {
   cc->cpLex = (p[0] << 8) + p[1] ;
   if (cc->cpLex >= 40320)
      return CORNER_PERMUTATION_OUT_OF_RANGE ;
   cc->coMask = (p[2] << 5) + (p[3] >> 3) ;
   if (cc->coMask >= 6561)
      return CORNER_ORIENTATION_OUT_OF_RANGE ;
   if ((p[3] & 7) != 7)
      return PUZZLE_ORIENTATION_NOT_SUPPORTED ;
   cc->epLex = (p[4] << 21) + (p[5] << 13) + (p[6] << 5) + (p[7] >> 3) ;
   if (cc->epLex >= 479001600)
      return EDGE_PERMUTATION_OUT_OF_RANGE ;
   if ((p[7] & 1) != 0)
      return CENTER_ORIENTATION_NOT_SUPPORTED ;
   cc->eoMask = (p[8] << 4) + (p[9] >> 4) ;
   if (cc->eoMask >= 4096)
      return EDGE_ORIENTATION_OUT_OF_RANGE ;
   cc->poIdxU = 7 ;
   cc->poIdxL = 0 ;
   cc->moSupport = 0 ;
   cc->moMask = 0 ;
   return 0 ;
}
int componentsToStickers(const struct cubecoords *cc, unsigned char *stickers) {
   initializeCubieTable() ;
   unsigned char perm[12] ;
   int eo = cc->eoMask ;
   decodePerm(cc->epLex, perm, 12) ;
   for (int i=0; i<12; i++) {
      int cubie = cubieExpand[4*perm[i]+(1&(eo>>(11-i)))] ;
      stickers[ReidOrder[2*i]] = cubie/6 ;
      stickers[ReidOrder[2*i+1]] = cubie%6 ;
   }
   decodePerm(cc->cpLex, perm, 8) ;
   int co = cc->coMask ;
   for (int i=19; i>=12; i--) {
      int cubie = cubieExpand[4*(12+perm[i-12])+(co%3)] ;
      stickers[ReidOrder[3*i-12]] = cubie/36 ;
      stickers[ReidOrder[3*i-11]] = cubie/6%6 ;
      stickers[ReidOrder[3*i-10]] = cubie%6 ;
      co /= 3 ;
   }
   for (int i=20; i<26; i++)
      stickers[ReidOrder[i+28]] = cubieExpand[4*i] ;
   return 0 ;
}
