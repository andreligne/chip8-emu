

chip8 : *.c *.h
	gcc -o chip8 *.c `sdl-config --cflags --libs` -lpthread

