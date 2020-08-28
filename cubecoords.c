#include "cubecoords.h"
#include "errors.h"
unsigned char *tobytes11(const struct cubecoords *cc, unsigned char *p) {
   p[0] = cc->epLex >> 21 ;
   p[1] = cc->epLex >> 13 ;
   p[2] = cc->epLex >> 5 ;
   p[3] = (cc->epLex << 3) + (cc->eoMask >> 9) ;
   p[4] = cc->eoMask >> 1 ;
   p[5] = (cc->eoMask << 7) + (cc->cpLex >> 9) ;
   p[6] = cc->cpLex >> 1 ;
   p[7] = (cc->cpLex << 7) + (cc->coMask >> 6) ;
   p[8] = (cc->coMask << 2) + (cc->poIdxU >> 1) ;
   p[9] = (cc->poIdxU << 7) + (cc->poIdxL << 5) +
          (cc->moSupport << 4) + (cc->moMask >> 8) ;
   p[10] = cc->moMask ;
   return p ;
}
int frombytes11(const unsigned char *p, struct cubecoords *cc) {
   cc->epLex = (p[0] << 21) + (p[1] << 13) + (p[2] << 5) + (p[3] >> 3) ;
   if (cc->epLex >= 479001600)
      return EDGE_PERMUTATION_OUT_OF_RANGE ;
   cc->eoMask = ((p[3] & 07) << 9) + (p[4] << 1) + (p[5] >> 7) ;
   if (cc->eoMask >= 4096)
      return EDGE_ORIENTATION_OUT_OF_RANGE ;
   cc->cpLex = ((p[5] & 0177) << 9) + (p[6] << 1) + (p[7] >> 7) ;
   if (cc->cpLex >= 40320)
      return CORNER_PERMUTATION_OUT_OF_RANGE ;
   cc->coMask = ((p[7] & 0177) << 6) + (p[8] >> 2) ;
   if (cc->coMask >= 6561)
      return CORNER_ORIENTATION_OUT_OF_RANGE ;
   cc->poIdxU = ((p[8] & 3) << 1) + (p[9] >> 7) ;
   if (cc->poIdxU != 7)
      return PUZZLE_ORIENTATION_NOT_SUPPORTED ;
   cc->poIdxL = (p[9] >> 5) & 3 ;
   cc->moSupport = (p[9] >> 4) & 1 ;
   if (cc->moSupport)
      return CENTER_ORIENTATION_NOT_SUPPORTED ;
   cc->moMask = ((p[9] & 017) << 8) + p[10] ;
   return 0 ;
}
