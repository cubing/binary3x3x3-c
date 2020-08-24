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
const unsigned char stickercubie[] = {
//Cor Edg Cor Edg Cen Edg Cor Edg Cor
   14,  2, 13,  3, 20,  1, 15,  0, 12, /* U face */
   14,  3, 15, 11, 21,  9, 18,  7, 17, /* L face */
   15,  0, 12,  9, 22,  8, 17,  4, 16, /* F face */
   12,  1, 13,  8, 23, 10, 16,  5, 19, /* R face */
   13,  2, 14, 10, 24, 11, 19,  6, 18, /* B face */
   17,  4, 16,  7, 25,  5, 18,  6, 19, /* D face */
};
/*
 *   These are the components we generate.
 */
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
 *   A solved and oriented cube looks like this.
 */
const unsigned char solved[] = {
  0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1, 2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4, 5,5,5,5,5,5,5,5,5,
} ;
/*
 *   Errors we can return.
 */
const int STICKER_ELEMENT_OUT_OF_RANGE = -1001 ;
const int ILLEGAL_CUBIE_SEEN = -1002 ;
const int MISSING_CORNER_CUBIE = -1003 ;
const int MISSING_EDGE_CUBIE = -1004 ;
const int MISSING_CENTER_CUBIE = -1005 ;
/*
 *   Here's the encode routine.  Returns 0 if everything looks good.
 *   Returns a negative number for an error code.  We scan in a
 *   specific order so orientations work (specifically, always clockwise
 *   around corners).
 */
const char faceletOrder[] = { 
  0, 1, 2, 3, 4, 5, 6, 7, 8, 45, 46, 47, 48, 49, 50, 51, 52, 53, // U/D
  19, 21, 22, 23, 25, 37, 39, 40, 41, 43, // F/B non-corners
  10, 12, 13, 14, 16, 28, 30, 31, 32, 34, // L/R non-corners
  9, 17, 18, 26, 27, 35, 36, 44,          // Right-twist corners
  11, 15, 20, 24, 29, 33, 38, 42 } ;      // left twist corners
int stickersToCubies(const unsigned char *stickers, int *cubies) {
   for (int i=0; i<26; i++)
      cubies[i] = 1 ;
   for (int fi=0; fi<54; fi++) {
      int i = faceletOrder[fi] ;
      int v = stickers[i] ;
      if (v > 5)
         return STICKER_ELEMENT_OUT_OF_RANGE ;
      int c = stickercubie[i] ;
      cubies[c] = cubies[c] * 6 + stickers[i] ;
   }
   return 0 ;
}
/*
 *   To initialize, we want a table that goes from cubie coloring
 *   back to actual cubies.  A -1 value means invalid.  We store
 *   the rotations here too, to help us get orientation later.
 *   The max value we need to store is 01555 base 6 which is 431
 *   (although this will never be a valid cube).  The lowest two
 *   bits are the orientation; the next five bits are the cubie
 *   index.  The value 255 means illegal.
 */
unsigned char cubieLookup[432] ;
void initializeCubieTable() {
   for (int i=0; i<sizeof(cubieLookup)/sizeof(cubieLookup[0]); i++)
      cubieLookup[i] = 255 ;
   int cubies[26] ;
   stickersToCubies(solved, cubies) ;
   for (int i=0; i<12; i++) {
      int v = cubies[i] ;
      cubieLookup[v] = 4 * i ;
      cubieLookup[36 + 6 * (v % 6) + (v / 6 - 6)] = 4 * i + 1 ;
   }
   for (int i=12; i<20; i++) {
      int v = cubies[i] ;
      cubieLookup[v] = 4 * i ;
      int c0 = v % 6 ;
      int c1 = (v / 6) % 6 ;
      int c2 = (v / 36) - 6 ;
      cubieLookup[216 + 36 * c0 + 6 * c2 + c1] = 4 * i + 1 ;
      cubieLookup[216 + 36 * c1 + 6 * c0 + c2] = 4 * i + 2 ;
   }
   for (int i=20; i<26; i++)
      cubieLookup[cubies[i]] = 4 * i ;
}
/*
 *   Index a permutation.  Return -1 if all values are not seen.
 *   Zero based.
 */
int encodePerm(const unsigned char *a, int n) {
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
 *   From an array of cubie values, calculate the relevant
 *   permutations and orientations.  Ensure all needed cubies are seen.
 */
int cubiesToComponents(const int *cubies, struct cubecoords *cc) {
   unsigned char edgep[12], cornerp[12], centerp[6] ;
   int edgeo = 0 ;
   int cornero = 0 ;
   for (int i=0; i<12; i++)
      edgep[i] = 255 ;
   for (int i=0; i<8; i++)
      cornerp[i] = 255 ;
   for (int i=0; i<6; i++)
      centerp[i] = 255 ;
   /*
    *   Make sure all cubies are valid.
    */
   for (int i=0; i<26; i++) {
      int c = cubies[i] ;
      if (c < 0 || c >= 432 || cubieLookup[c] == 255)
         return ILLEGAL_CUBIE_SEEN ;
   }
   for (int i=0; i<12; i++) {
      int cubie = cubieLookup[cubies[i]] ;
      edgep[i] = cubie >> 2 ;
      edgeo = 2 * edgeo + (cubie & 3) ;
   }
   for (int i=12; i<20; i++) {
      int cubie = cubieLookup[cubies[i]] ;
      cornerp[i-12] = (cubie >> 2) - 12 ;
      cornero = 3 * cornero + (cubie & 3) ;
   }
   for (int i=20; i<26; i++)
      centerp[i-20] = (cubieLookup[cubies[i]] >> 2) - 20 ;
   int cornerperm = encodePerm(cornerp, 8) ;
   if (cornerperm < 0)
      return MISSING_CORNER_CUBIE ;
   int edgeperm = encodePerm(edgep, 12) ;
   if (edgeperm < 0)
      return MISSING_EDGE_CUBIE ;
   if (encodePerm(centerp, 6) < 0)
      return MISSING_CENTER_CUBIE ;
   cc->cpLex = cornerperm ;
   cc->coMask = cornero ;
   cc->poIdxU = 0 ;
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
   *p++ = (cc->coMask << 3) + 6 ; // 6:  no center orientation
   *p++ = (cc->epLex >> 21) ;
   *p++ = (cc->epLex >> 13) ;
   *p++ = (cc->epLex >> 5) ;
   *p++ = (cc->epLex << 3) ;
   *p++ = (cc->eoMask >> 4) ;
   *p++ = (cc->eoMask << 4) ;
   *p++ = 0 ;
   return r ;
}
/*
 *   Test things.
 */
#include <stdio.h>
#include <stdlib.h>
void showcubecoords(struct cubecoords *cc) {
   printf("%d %d %d %d %d %d %d\n", cc->cpLex, cc->coMask, cc->poIdxU,
           cc->epLex, cc->poIdxL, cc->moSupport, cc->eoMask) ;
}
void showbytes(unsigned char *b, int n) {
   for (int i=0; i<n; i++)
      printf(" %d", b[i]) ;
   printf("\n") ;
}
void showenc(unsigned char *s) {
   int cubies[26] ;
   unsigned char bytes[11] ;
   struct cubecoords cc ;
   int err = stickersToCubies(s, cubies) ;
   if (err < 0)
      printf("Got an error: %d\n", err) ;
   err = cubiesToComponents(cubies, &cc) ;
   if (err < 0)
      printf("Got an error: %d\n", err) ;
   showcubecoords(&cc) ;
   showbytes(tobytes11(&cc, bytes), 11) ;
}
int main() {
   initializeCubieTable() ;
   unsigned char s[54] ;
   while (1) {
      for (int i=0; i<54; i++) {
         int v ;
         if (scanf("%d", &v) != 1)
            exit(0) ;
         s[i] = v ;
      }
      showenc(s) ;
   }
}
