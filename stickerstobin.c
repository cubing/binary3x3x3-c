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
/*
 *   Index a permutation.  Return -1 if all values are not seen.
 *   Zero based.  This can be made faster (no inner loop) if the
 *   CPU has a popcount instruction but some microcontrollers
 *   might not have such.
 */
static int encodePerm(const unsigned char *a, int n) {
   int bits = 0 ;
   int r = 0 ;
   for (int i=0; i<n; i++) {
      bits |= 1<<a[i] ;
      r = r * (n-i) ;
      for (int j=i+1; j<n; j++)
         if (a[i] > a[j])
            r++ ;
   }
   if (bits + 1 != 1 << n)
      return -1 ;
   return r ;
}
/*
 *   Unindex a perm.  This can be made faster if the CPU has
 *   64-bit ints but microcontrollers might not have such.
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
   for (int i=20; i<26; i++)
      perm[i-20] = stickers[ReidOrder[i+28]] ;
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
