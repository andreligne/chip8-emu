

chip8 : *.c *.h
	clang -o chip8 *.c `sdl-config --cflags --libs` -lpthread

clean :
	rm chip8
