#include "chip8.h"

void init_chip(chip8_t *cpu) 
{
	// Allocate 4kB of ram
	cpu->memory = malloc(4096); // Allocate 4kB for the memory
	memset(cpu->memory, 0xFF, 4096);

	// Allocate memory for the register. It has 16 8-bit data registers.
	cpu->V = malloc(16);

	// Allocate the stack. It's 16 level deep
	cpu->stack = malloc(16);

	// Allocate space for the display matris.
	// The screen is 64x32 pixels wide.
	cpu->display = malloc(256); // Got a screen with 64x32 ppx (256 bytes)

	// Set default values for pointers
	cpu->stackPointer = 0;
	cpu->pc = 0x200;
	cpu->I = 0;
}

/* Free all resources for the cpu */
void free_chip(chip8_t *cpu)
{
	// Free all allocated memory in the chip8_t struct
	free(cpu->memory);
	free(cpu->V);
	free(cpu->stack);
	free(cpu->display);

	// Free the struct
	free(cpu);
}

void load_file(chip8_t *cpu, char *filename)
{
	/* Open the file */
	FILE *pFile;
	pFile = fopen(filename, "r");

	if (!pFile) {
		printf("Couldn't open the given file.\n");
		return;
	}

	/* Read instruction from the file and put in the memory
	 * Instructions start at 0x200 */
	int read_bytes = fread(&cpu->memory[cpu->pc], 1, 4096, pFile);

	/* Print out the starting byte */
	printf("Read %i bytes from the file %s.\n", read_bytes, filename);

	/* Close the file */
	printf("Successfully loaded '%s' into the memory.\n", filename);
	fclose(pFile);
}

/* Step and handle one instruction into the program */
void step(chip8_t *cpu)
{
	/* Get the next instruction 
	 * Read the first 8 bits, shift them up and AND with the other 8 bits */
	uint16_t opcode = (cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];

	/* Mask out the OPCODE and handle the correct operation
	 * Print out the instruction so we can follow it later */
	printf("0x%X: ", opcode);
	switch (opcode & 0xF000) 
	{
		case 0x0000: { // Multivalued instruction
			printf("Entering multivalued instruction with 0xF0%X\n\t", (opcode & 0x00FF));

			/* Need to switch on that new opcode */
			switch (opcode & 0x00FF) 
			{

				case 0xE0: { // 00E0: Clears the screen.
					printf("Clearing the screen.\n"); 

					/* Just set the whole video memory to 0s */
					memset(cpu->display, 0, sizeof(cpu->display));

					// Move the PC to the next instruction
					cpu->pc += 2;
					break;
				}

				case 0xEE: { // 00EE: Returns from a subroutine.
					printf("Returning from a subroutine.\n", (opcode & 0x0FFF));

					/* Pop the adr off the stack and put as the new PC */
					cpu->stackPointer--;
					cpu->pc = cpu->stack[cpu->stackPointer];
					break;
				}

				default: {
					printf("Unimplemented 0x00%X. Exiting...\n", (opcode & 0x00FF));
					exit(1);
				}
			}

			break;
		}

		case 0x1000: { // 1NNN: Jumps to address NNN.
			printf("Jumping to 0x%x.\n", (opcode & 0x0FFF));
			
			/* Set the PC to the new value */
			cpu->pc = (opcode & 0x0FFF);
			break;
		}

		case 0x2000: { // 2NNN: Calls subroutine at NNN.
			printf("Calling subroutine at 0x%x.\n", (opcode & 0x0FFF));

			/* Save the next instruction on the stack and increase the sp */
			cpu->stack[cpu->stackPointer++] = cpu->pc + 2;

			/* Set the PC to the new adr */
			cpu->pc = (opcode & 0x0FFF);
			break;
		}

		case 0x3000: { // 3XNN: Skips the next instruction if VX equals NN.
			printf("Skip if VX equals NN ");

			/* Print out the values */
			printf("(V[0x%X] = 0x%X and NN = 0x%X). ", (opcode & 0x0F00) >> 8,
												cpu->V[(opcode & 0x0F00) >> 8],
												opcode & 0x00FF);

			if (cpu->V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
				printf("Skipping.\n");
				cpu->pc += 2;
			} else {
				printf("Not skipping.\n");
			}

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0x4000: { // 4XNN:: Skips the next instruction if VX doesn't equal NN
			printf("Skip if VX doesn't equals NN");

			/* Print out the values */
			printf("(V[0x%X] = 0x%X and NN = 0x%X). ", (opcode & 0x0F00) >> 8,
												cpu->V[(opcode & 0x0F00) >> 8],
												opcode & 0x00FF);

			if (cpu->V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
				printf("Skipping.\n");
				cpu->pc += 2;
			} else {
				printf("Not skipping.\n");
			}

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0x6000: { // 6XNN: Sets VX to NN.
			printf("Setting VX to 0x%x.\n", (opcode & 0x00FF));
			cpu->V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0x7000: { // 7XNN: Adds NN to VX.
			printf("Adds NN to VX.\n");

			/* Do the addition */
			cpu->V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}			 

		case 0xA000: { // ANNN: Sets I to the address NNN.
			printf("Setting I to 0x%x.\n", (opcode & 0x0FFF));

			/* Set I to NNN */
			cpu->I = (opcode & 0x0FFF);

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0xC000: { // CXNN: Sets VX to a random number and NN.
			printf("Setting VX to a random number and NN. (skipped)\n");

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0xD000: { // DXYN: Draw a sprite from I to position X, Y
			/* Parse out the values that's going to be needed */
			uint8_t width = 8; // This value is always 8
			uint8_t height = (opcode & 0x000F); // This value is always 8
			uint8_t x_pos = (opcode & 0x0F00) >> 8;
			uint8_t y_pos = (opcode & 0x00F0) >> 4;

			printf("Draw a sprite from I width height %i to X,Y.\n", height);

			/* Unset the carry flag before we start */
			cpu->V[0xF] = 0;

			// Starting position for the drawing
			// uint16_t *asdf = cpu->display[() + x_pos];

			/* The actual drawing */
			int _x, _y;
			_x = _y = 0;

			// Iterate over the rows of the sprite
			for (; _y < height; _y++) {

				// Get the old rows

				// Check if we need to set VF
				// TODO: Invert the bits and & with it. If not 0xFFFF, set VF

				// Copy over the rows of the sprite to the video memory
			}

			// Dump the video memory content
			/*
			int index = 0;
			for (; index < (64 * 32); index++) {
				if ((index % 64) == 0)
					printf("\n");

				printf("%c", cpu->display[index] ? ' ' : 'x');
			}
			*/

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0xE000: { // Multivalued instruction
			printf("Entering multivalued instruction with 0x%x\n\t", opcode);
			uint8_t x = (opcode & 0x0F00) >> 8;

			switch (opcode & 0x00FF) {

				// EX9E: Skips the next instruction if the key stored in VX is pressed.
				case 0x9E: {
					printf("Skipping next instruction if key is pressed (skipping).\n");
					break;
				}
				
				// EXA1: Skips the next instruction if the key stored in VX isn't pressed.
				case 0xA1: { 
					printf("Skipping next instruction if key isn't pressed. (skipping)\n");
					break;
				}

				default: {
					printf("Unimplemented instruction 0x00%X. \
							Exiting...\n", (opcode & 0x00FF));
					exit(1);
				}
			}

			// Every instruction in this will not modify the pc in any way
			cpu->pc += 2;
			break;
		}
		
		case 0xF000: { // Multivalued instruction
			printf("Entering multivalued instruction with 0xF%X\n\t", (opcode & 0x0FFF));
			uint8_t x = (opcode & 0x0F00) >> 8;

			switch (opcode & 0x00FF) 
			{
				// FX07: Sets VX to the value of the delay timer.
				case 0x07: {
					printf("Setting VX to the value of the delay timer. (skipped)\n");
					break;
				}

				case 0x15: { // FX15: Sets the delay timer to VX.
					printf("Setting delay time to VX. (skipped)\n");
					break;
				}

				case 0x1E: { // FX1E Adds VX to I. (If overflow, set VF)
					printf("Adding VX to I.\n");

					/* If it overflows we need to set the carry flag */
					/* NOTE: Kinda cool that the compiler complained when tried
					 * using a uint8_t since that always will be false */
					uint16_t _i = cpu->I + cpu->V[x];
					if (_i > 0xFFF) {
						printf("Overflow: Setting carry flag\n\t");
						cpu->V[0xF] = 1;
					}

					/* Do the addition */
					cpu->I = _i;
					break;
				}

				// FX33: Store a binary coded representation of VX with the three most 
				// significant digits at I. Meaning that the number 156 would be placed as
				// I[0] = 1, I[1] = 5, I[2] = 6
				case 0x33: {
					printf("Storing a binary-coded representation of VX at I. (skipped)\n"); 
					break;
				}

				default: {
					printf("Unimplemented instruction 0x00%x. \
							Exiting...\n", (opcode & 0x00FF));
					exit(1);
				}
			}
			
			// Every instruction in this will not modify the pc in any way
			cpu->pc += 2;
			break;
		}

		default: {
			printf("Unimplemented instruction: 0x%X. Exiting...\n", opcode);
			exit(1);
			break;
		}
	}
}

/* Return the display matrix */
void *get_display(chip8_t *cpu)
{
	return cpu->display;
}

/* Function which steps through the program */
void run_chip(chip8_t *cpu)
{
	for (;;)
		step(cpu);
}
