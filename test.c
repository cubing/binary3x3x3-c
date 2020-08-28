/*
 *   Test things.
 */
#include <stdio.h>
#include <stdlib.h>
#include "cubecoords.h"
#include "stickerstobin.h"
#include "heykubetobin.h"
#include "reidtobin.h"
void showcubecoords(struct cubecoords *cc) {
   printf("%d %d %d %d %d %d %d %d\n", cc->epLex, cc->eoMask, cc->cpLex,
           cc->coMask, cc->poIdxU, cc->poIdxL, cc->moSupport, cc->moMask) ;
}
void showbytes(unsigned char *b, int n) {
   for (int i=0; i<n; i++)
      printf(" %x", b[i]) ;
   printf("\n") ;
}
char reidbuf[100] ;
void showenc(unsigned char *s) {
   unsigned char bytes[11] ;
   struct cubecoords cc ;
   int err = stickersToComponents(s, &cc) ;
   if (err < 0)
      printf("Got an error: %d\n", err) ;
   showcubecoords(&cc) ;
   showbytes(tobytes11(&cc, bytes), 11) ;
   struct cubecoords cc2 ;
   err = frombytes11(bytes, &cc2) ;
   if (err < 0)
      printf("Got an error: %d\n", err) ;
   showcubecoords(&cc2) ;
   unsigned char stickers2[54] ;
   componentsToHeykube(&cc2, stickers2) ;
   componentsToReid(&cc2, reidbuf) ;
   err = componentsToStickers(&cc2, stickers2) ;
   if (err < 0)
      printf("Got an error: %d\n", err) ;
   showbytes(stickers2, 54) ;
   int badcount = 0 ;
   for (int i=0; i<54; i++)
      if (s[i] != stickers2[i])
         badcount++ ;
   if (badcount)
      printf("%d stickers did not match.\n", badcount) ;
}
int main() {
   unsigned char s[54] ;
   while (1) {
      for (int i=0; i<54; i++) {
         int v ;
         if (scanf("%d", &v) != 1)
            exit(0) ;
         s[i] = v ;
      }
      showbytes(s, 54) ;
      showenc(s) ;
   }
}
