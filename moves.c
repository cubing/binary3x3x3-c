#include "cubecoords.h"
#include "heykubetobin.h"
#include "moves.h"
#include "errors.h"
struct basemove {
   char movename ;
   struct cubecoords cc ;
} ;
static struct basemove basemoves[] = {
   {'U', {43908480, 0, 5880, 0}}, {'D', {15120, 0, 9, 0}},
   {'F', {363310128, 2188, 16008, 2412}}, {'B', {2949785, 547, 4352, 1708}},
   {'R', {25813736, 0, 20325, 5132}}, {'L', {328525, 0, 486, 588}}
} ;
// a and c must not be the same object
static void permmul(const perm a, const perm b, perm c) {
   for (int i=0; i<PERM_N; i++)
      c[i] = a[b[i]] ;
}
void iota(perm a) {
   for (int i=0; i<PERM_N; i++)
      a[i] = i ;
}
static perm allmoves[18] ;
static int inited = 0 ;
static void initmoves() {
   if (inited)
      return ;
   for (int i=0; i<6; i++) {
      componentsToHeykube(&basemoves[i].cc, allmoves[3*i]) ;
      for (int m=1; m<3; m++)
         permmul(allmoves[3*i+m-1], allmoves[3*i], allmoves[3*i+m]) ;
   }
   inited = 1 ;
}
void domove(perm a, int mv) {
   initmoves() ;
   perm t ;
   permmul(a, allmoves[mv], t) ;
   for (int i=0; i<PERM_N; i++)
      a[i] = t[i] ;
}
int domoves(perm a, const char *s) {
   for (;;) {
      while (*s && *s <= ' ')
         s++ ;
      if (!*s)
         return 0 ;
      int mv = -1 ;
      for (int i=0; i<6; i++)
         if (*s == basemoves[i].movename)
            mv = 3*i ;
      if (mv < 0)
         return BAD_MOVE_FORMAT ;
      s++ ;
      if (*s == '2') {
         mv++ ;
         s++ ;
      } else if (*s == '\'') {
         mv += 2 ;
         s++ ;
      }
      domove(a, mv) ;
   }
   return 0 ;
}
