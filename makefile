stickerstobin: stickerstobin.h stickerstobin.c test.c
	gcc -g -o stickerstobin stickerstobin.c test.c

.PHONY: clean
clean:
	rm -rf stickerstobin stickerstobin.dSYM
