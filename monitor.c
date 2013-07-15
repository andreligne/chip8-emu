#include "monitor.h"

/* Functions for modifying the SDL Screen */
void init_monitor(SDL_Surface **screen, char *filename)
{
	SDL_Init(SDL_INIT_VIDEO);

	/* Allocate some variables */
	g_event = malloc(sizeof(SDL_Event));

	/* Set the title bar */
	SDL_WM_SetCaption("chip8-emu", filename);

	/* Create the window */
	*screen = SDL_SetVideoMode(640, 320, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

	/* Should start a thread for refreshing the window here */
}

/* Functions for freeing all SDL resources */
void free_monitor(SDL_Surface *screen)
{
	SDL_FreeSurface(screen);
	SDL_Quit();
}

/* Draw rectangle at x,y position */
void draw_pixel(uint8_t x, uint8_t y, SDL_Surface *screen)
{
	// Create a SDL_Rect which will represent a pixel on the screen
	SDL_Rect pixel;
	pixel.w = pixel.h = 10;

	// Put the pixel at the right position
	pixel.x = x * 10;
	pixel.y = y * 10;

	// Print the SDL_Rect to the screen
	SDL_FillRect(screen, &pixel, 0xFFFFFFFF);
}

/* Draw all pixels from the vram to the screen. Runs at a rate of
 * 60Hz (60 fps). */
void draw_monitor(SDL_Surface *screen, uint8_t *vram)
{
	// Fill background with black
	SDL_FillRect(screen, NULL, 0x000000);

	uint32_t index = 0;
	for (; index < (64 * 32); index++) {
		if (vram[index] == 0)
			continue;

		uint16_t x = index % 64;
		uint16_t y = index / 64;

		draw_pixel(x, y, screen);
	}

	// Update the screen buffer
	SDL_Flip(screen);
}
