/*
 *   Test things.
 */
#include <stdio.h>
#include <stdlib.h>
#include "stickerstobin.h"
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
      showenc(s) ;
   }
}
