#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "monitor.h"
#include "chip8.h"

/* Include variables from other files */
extern SDL_Surface *g_scr;
extern SDL_Event *g_event;

/* Define the threads for the monitor- and emulator-runs */
pthread_t *monitor_thread;
pthread_t *emulator_thread;
int quiting = 0;

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
	init_monitor(&g_scr, argv[1]);

	// Load the file into the cpu memory
	load_file(cpu, argv[1]);

	// Start the threads
	emulator_thread = malloc(sizeof(pthread_t));
	pthread_create(emulator_thread, NULL, (void *)&run_chip, cpu);
	
	// Run the main program
	while (!quiting) {
		// TODO: Wait for events instead of spamming
		// Handle all the events
		while (SDL_PollEvent(g_event)) {
			switch (g_event->type)
			{
				case SDL_QUIT:
					printf("Quiting.\n");
					quiting = 1;
					break;

				default:
					// Set keys to not pressed
					break;
			}
		}

		// draw_monitor(g_scr, get_display(cpu));

		sleep(1 / 60);
	}

	// Stop and the threads
	pthread_cancel(*emulator_thread);
	free(emulator_thread);

	// Clean up
	free_monitor(g_scr);
	free_chip(cpu);

	exit(0);
}

