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

/* Draw all pixels from the vram to the screen. Runs at a rate of
 * 60Hz (60 fps). */
void draw_monitor(SDL_Surface *screen, void *vram)
{
	// Fill the window background with black color
	SDL_FillRect(screen, NULL, 0x000000);

	// Create a SDL_Rect which will represent a pixel on the screen
	SDL_Rect pixel;
	pixel.x = pixel.y = 0;
	pixel.w = pixel.h = 10;

	int pixel_index = 0;
	for (; pixel_index < 256; pixel_index++) {
		// New row dawg
		if (pixel_index != 0 && ((pixel_index % 64) == 0)) {
			pixel.x = 0;
			pixel.y += 10;
		}

		// Print the SDL_Rect to the screen
		SDL_FillRect(screen, &pixel, 0xFFFFFFFF);

		// Move the rectangle
		pixel.x += 10;
		pixel.y += 10;
	}

	// Update the screen buffer
	SDL_Flip(screen);

	return;
}
