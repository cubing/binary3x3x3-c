#include "cubecoords.h"
#include "heykubetobin.h"
#include "moves.h"
#include "errors.h"
struct basemove {
   char movename ;
   struct cubecoords cc ;
} ;
static struct basemove basemoves[] = {
   {'U', {119750400, 0, 15120, 0}}, {'D', {5880, 0, 18, 0}},
   {'F', {323393334, 2188, 21006, 2412}}, {'B', {3312664, 547, 1233, 1708}},
   {'R', {33070610, 0, 9507, 5132}}, {'L', {247911, 0, 176, 588}},
} ;
// b and c must not be the same object
static void permmul(const perm a, const perm b, perm c) {
   for (int i=0; i<PERM_N; i++)
      c[i] = b[a[i]] ;
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
   permmul(a, allmoves[mv], a) ;
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
