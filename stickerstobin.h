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
 *   Routines exported.
 */
extern int stickersToComponents(const unsigned char *stickers,
                                struct cubecoords *cc) ;
extern unsigned char *tobytes11(const struct cubecoords *cc, unsigned char *p) ;
extern int frombytes11(const unsigned char *p, struct cubecoords *cc) ;
extern int componentsToStickers(const struct cubecoords *cc,
                                unsigned char *stickers) ;
/*
 *   Errors we can return.
 */
#define STICKER_ELEMENT_OUT_OF_RANGE (-1001)
#define ILLEGAL_CUBIE_SEEN (-1002)
#define MISSING_CORNER_CUBIE (-1003)
#define MISSING_EDGE_CUBIE (-1004)
#define MISSING_CENTER_CUBIE (-1005)
#define CORNER_PERMUTATION_OUT_OF_RANGE (-1006)
#define CORNER_ORIENTATION_OUT_OF_RANGE (-1007)
#define PUZZLE_ORIENTATION_NOT_SUPPORTED (-1008)
#define EDGE_PERMUTATION_OUT_OF_RANGE (-1009)
#define CENTER_ORIENTATION_NOT_SUPPORTED (-1010)
#define EDGE_ORIENTATION_OUT_OF_RANGE (-1011)
