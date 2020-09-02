/*
 *   Test things.  Run with
 *
 *   ./stickerstobin [-b] [-c] [-h] [-s] [-R] [-v] < input > output
 *
 *   Input is auto-detected amongst binary, component, heycube,
 *   sticker, and Reid format.  The options -b, -c, -h, -s, and -R
 *   select binary, component, heycube, sticker, and Reid format for
 *   output; more than one can be selected.  The -v option turns on
 *   verbose mode.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cubecoords.h"
#include "stickerstobin.h"
#include "heykubetobin.h"
#include "reidtobin.h"
#include "moves.h"
int formatstoshow ;
int verbose ;
const int INBUFSZ = 2048 ;
char inbuffer[INBUFSZ] ;
void error(const char *s) {
   fprintf(stderr, "rubikconvert: %s\n", s) ;
   exit(10) ;
}
struct cubecoords cc ;
char reidbuf[INBUFSZ] ;
unsigned char buf1[100] ;
unsigned char buf2[100] ;
char *toks[54] ;
int itoks[54] ;
void toints(int n, int lo, int hi, int base) {
   for (int i=0; i<n; i++) {
      char *end ;
      int v = strtol(toks[i], &end, base) ;
      if (*end)
         error("! bad parse of int") ;
      if (v < lo || v >= hi)
         error("! integer value out of range") ;
      itoks[i] = v ;
   }
}
int ismovestring(const char *a) {
   return ((*a == 'U' || *a == 'F' || *a == 'R' ||
            *a == 'D' || *a == 'B' || *a == 'L') &&
           (a[1] == 0 || (a[2] == 0 && (a[1] == '2' || a[1] == '\'')))) ;
}
int main(int argc, char *argv[]) {
   while (argc > 1 && argv[1][0] == '-') {
      argc-- ;
      argv++ ;
      switch (argv[0][1]) {
case 'R': formatstoshow |= 1<<('r'-'a') ; break ;
case 'b': formatstoshow |= 1<<('b'-'a') ; break ;
case 'c': formatstoshow |= 1<<('c'-'a') ; break ;
case 's': formatstoshow |= 1<<('s'-'a') ; break ;
case 'h': formatstoshow |= 1<<('h'-'a') ; break ;
case 'v': verbose = 1 ; break ;
      }
   }
   if (formatstoshow == 0) {
      verbose = 1 ;
      formatstoshow = -1 ; // show everything
   }
   while (fgets(inbuffer, INBUFSZ-1, stdin)) {
      // let's try to figure out what we got.  how many tokens?
      int ntoks = 0 ;
      int intok = 0 ;
      char *q = inbuffer + strlen(inbuffer) - 1 ;
      while (q >= inbuffer && *q <= ' ') // clear trailing whitespace
         *q-- = 0 ;
      strcpy(reidbuf, inbuffer) ; // keep a copy; we're about to trash inbuffer
      for (char *p = inbuffer; *p; p++) {
         if (*p <= ' ') {
            if (intok) {
               *p = 0 ;
               intok = 0 ;
            }
         } else {
            if (!intok) {
               if (ntoks > 54)
                  error("! too many tokens") ;
               toks[ntoks++] = p ;
               intok = 1 ;
            }
         }
      }
      int err = 0 ;
      if (ntoks > 0 && ismovestring(toks[0])) {
         perm p ;
         iota(p) ;
         err = domoves(p, reidbuf) ;
         if (err == 0)
            err = heykubeToComponents(p, &cc) ;
      } else if (ntoks == 4) { // has to be 4-valued coordinate values
         toints(ntoks, 0, 500000000, 10) ;
         cc.epLex = itoks[0] ;
         cc.eoMask = itoks[1] ;
         cc.cpLex = itoks[2] ;
         cc.coMask = itoks[3] ;
         cc.poIdxU = 7 ;
         cc.poIdxL = cc.moSupport = cc.moMask = 0 ;
      } else if (ntoks == 11) { // has to be 11-byte hex
         toints(ntoks, 0, 256, 16) ;
         for (int i=0; i<11; i++)
            buf1[i] = itoks[i] ;
         err = frombytes11(buf1, &cc) ;
      } else if (ntoks == 20) { // has to be reid
         err = ReidToComponents(reidbuf, &cc) ;
      } else if (ntoks == 54) { // only stickers and hexcube are left.
         toints(ntoks, 0, 54, 10) ;
         int hival = 0 ;
         for (int i=0; i<54; i++) {
            if (itoks[i] > hival)
               hival = itoks[i] ;
            buf1[i] = itoks[i] ;
         }
         if (hival == 5) {
            err = stickersToComponents(buf1, &cc) ;
         } else if (hival == 53) {
            err = heykubeToComponents(buf1, &cc) ;
         } else {
            error("! bad stickers or permutation values") ;
         }
      } else {
         error("! bad number of tokens on a line") ;
      }
      if (err == 0) {
         tobytes11(&cc, buf1) ;
         err = frombytes11(buf1, &cc) ; // use error checking here
      }
      if (err != 0) {
         fprintf(stderr, "Failed with error code %d\n", err) ;
         exit(10) ;
      }
      for (int of='a'; of<='z'; of++) {
         if ((formatstoshow >> (of-'a')) & 1) {
            err = 0 ;
            switch(of) {
case 'b':
               if (verbose)
                   printf("Binary: ") ;
               for (int i=0; i<11; i++) {
                  if (i)
                     printf(" ") ;
                  printf("%02x", buf1[i]) ;
               }
               printf("\n") ;
               break ;
case 'c':
               if (verbose)
                   printf("Components: ") ;
               printf("%d %d %d %d\n", cc.epLex, cc.eoMask, cc.cpLex,
                                       cc.coMask) ;
               break ;
case 'r':
               err = componentsToReid(&cc, reidbuf) ;
               if (verbose)
                   printf("Reid: ") ;
               printf("%s\n", reidbuf) ;
               break ;
case 'h':
               if (verbose)
                   printf("Heycube: ") ;
               err = componentsToHeykube(&cc, buf1) ;
               for (int i=0; i<54; i++) {
                  if (i)
                     printf(" ") ;
                  printf("%d", buf1[i]) ;
               }
               printf("\n") ;
               break ;
case 's':
               if (verbose)
                   printf("Stickers: ") ;
               err = componentsToStickers(&cc, buf1) ;
               for (int i=0; i<54; i++) {
                  if (i)
                     printf(" ") ;
                  printf("%d", buf1[i]) ;
               }
               printf("\n") ;
               break ;
default:
               break ;
            }
            if (err)
               error("! error during output conversion") ;
         }
      }
   }
}
