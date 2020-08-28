/*
 *   Perm to index and back.
 */
#include "index.h"
/*
 *   Index a permutation.  Return -1 if all values are not seen.
 *   Zero based.
 */
static const unsigned char popcount64[] = {
   0,1,1,2,1,2,2,3, 1,2,2,3,2,3,3,4, 1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5,
   1,2,2,3,2,3,3,4, 2,3,3,4,3,4,4,5, 2,3,3,4,3,4,4,5, 3,4,4,5,4,5,5,6 } ;
int encodePerm(const unsigned char *a, int n) {
   int bits = 0 ;
   int r = 0 ;
   for (int i=0; i<n; i++) {
      bits |= 1<<a[i] ;
      int low = ((1<<a[i])-1) & bits ;
      r = r * (n-i) + a[i] - popcount64[low>>6] - popcount64[low&63] ;
   }
   if (bits + 1 != 1 << n)
      return -1 ;
   return r ;
}
/*
 *   Unindex a perm.  This can be made faster if the CPU has
 *   64-bit ints but microcontrollers might not have such.
 */
void decodePerm(int lex, unsigned char *a, int n) {
   a[n-1] = 0 ;
   for (int i=n-2; i>=0; i--) {
      a[i] = lex % (n - i) ;
      lex /= n-i ;
      for (int j=i+1; j<n; j++)
         if (a[j] >= a[i])
            a[j]++ ;
   }
}
