#ifndef MONITOR_H_
#define MONITOR_H_

#include "sdl.h"

/* Variables for the monitor */
SDL_Surface *g_scr;

/* Functions for initializing the SDL resources */
void init_monitor(SDL_Surface *, char *);

/* Functions for freeing all SDL resources */
void free_monitor(SDL_Surface *);

/* A function which handles the refreshing of the screen */
void run_monitor(SDL_Surface *);

#endif
