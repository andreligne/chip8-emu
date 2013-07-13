#include "monitor.h"

/* Functions for modifying the SDL Screen */
void init_monitor(SDL_Surface *screen, char *filename)
{
	SDL_Init(SDL_INIT_VIDEO);

	/* Set the title bar */
	SDL_WM_SetCaption("chip8-emu", filename);

	/* Create the window */
	screen = SDL_SetVideoMode(640, 320, 0, 0);

	/* Should start a thread for refreshing the window here */
}

/* Functions for freeing all SDL resources */
void free_monitor(SDL_Surface *screen)
{
	SDL_FreeSurface(screen);
	SDL_Quit();
}

/* A function which handles the refreshing of the screen
 * The screen should operate at a rate of 60Hz */
void run_monitor(SDL_Surface *screen)
{
	// Fill the window with black color
	SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0, 0, 0));

	// Update the screen buffer
	SDL_Flip(screen);

	return;
}
