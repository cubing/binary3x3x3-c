stickerstobin: errors.h cubecoords.h cubecoords.c index.h index.c stickerstobin.h stickerstobin.c heykubetobin.h heykubetobin.c reidtobin.h reidtobin.c moves.h moves.c test.c
	gcc -g -o stickerstobin stickerstobin.c heykubetobin.c reidtobin.c index.c cubecoords.c moves.c test.c

.PHONY: clean
clean:
	rm -rf stickerstobin stickerstobin.dSYM
