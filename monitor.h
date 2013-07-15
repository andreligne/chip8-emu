#ifndef MONITOR_H_
#define MONITOR_H_

#include "SDL.h"

/* Should define some colors here */

/* Variables for the monitor */
SDL_Surface *g_scr;
SDL_Event *g_event;

/* Functions for initializing the SDL resources */
void init_monitor(SDL_Surface **, char *);

/* Functions for freeing all SDL resources */
void free_monitor(SDL_Surface *);

/* Draw rectangle at x,y position */
void draw_pixel(uint8_t, uint8_t, SDL_Surface *);

/* A function which handles the refreshing of the screen */
void draw_monitor(SDL_Surface *, uint8_t *);

#endif
