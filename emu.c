#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "monitor.h"
#include "chip8.h"

/* Include variables from other files */
extern SDL_Surface *g_scr;

/* Define the threads for the monitor- and emulator-runs */
pthread_t *monitor_thread;
pthread_t *emulator_thread;

int main(int argc, char *argv[])
{
	/* Set some dummy values */
	argc = 2;
	argv[1] = "programs/tetris.c8";

	/* Make the user specify which file to open */
	if (argc != 2) {
		printf("Usage: chip <filename>\n");
		return 1;
	}

	// Initialize the emulator
	chip8_t *cpu;
	cpu = malloc(sizeof(chip8_t));
	init_chip(cpu);

	// Initialize the monitor
	init_monitor(g_scr, argv[1]);

	// Load the file into the memory
	load_file(cpu, argv[1]);
	
	// Start the threads

	// Start the emulator thread
	for (;;) { 
		step(cpu); 
		usleep(1 / 60); 
	}

	// Free the needed resources
	free_monitor(g_scr);

	return 0;
}
