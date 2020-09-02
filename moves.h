#ifndef MOVES_H
#define PERM_N 54
typedef unsigned char perm[PERM_N] ;
extern void iota(perm a) ;
extern void domove(perm a, int mv) ;
extern int domoves(perm a, const char *s) ;
#define MOVES_H
#endif
