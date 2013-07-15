#include "chip8.h"

/* Define the fontset */
uint8_t c8_fontset[0x80] =
{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0   
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void init_chip(chip8_t *cpu) 
{
	// Allocate 4kB of ram
	cpu->memory = malloc(4096); // Allocate 4kB for the memory
	memset(cpu->memory, 0x00, 4096);

	// Set the fontset
	memcpy(cpu->memory, c8_fontset, 0x80);

	// Allocate memory for the register. It has 16 8-bit data registers.
	cpu->V = malloc(16);

	// Allocate the stack. It's 16 level deep
	cpu->stack = malloc(16);

	// Allocate space for the display matris.
	// The screen is 64x32 pixels wide.
	cpu->display = (uint8_t *)malloc(64 * 32);
	memset(cpu->display, 0, 64*32);

	// Set default value for the timers
	cpu->delay_timer = 0;
	cpu->sound_timer = 0;

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
	 * Read the first 8 bits, shift them up and OR with the other 8 bits */
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
					memset(cpu->display, 0, sizeof(*cpu->display));

					// Move the PC to the next instruction
					cpu->pc += 2;
					break;
				}

				case 0xEE: { // 00EE: Returns from a subroutine.
					printf("Returning from a subroutine.\n");

					/* Pop the adr off the stack and put as the new PC */
					cpu->stackPointer--;
					cpu->pc = cpu->stack[cpu->stackPointer];
					break;
				}

				default: {
					printf("Unimplemented 0x00%x. Exiting...\n", (opcode & 0x00FF));
					exit(1);
				}
			}

			break;
		}

		case 0x1000: { // 1NNN: Jumps to address NNN.
			printf("Jumping to 0x0%x.\n", (opcode & 0x0FFF));
			
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
			uint8_t _x = (opcode & 0x0F00) >> 8;
			uint8_t _nn = (opcode & 0x00FF);
			cpu->V[_x] += _nn;

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}			 

		case 0x8000: { // Multivalued instruction, 8XYN
			uint16_t _x = (opcode & 0x0F00) >> 8;
			uint16_t _y = (opcode & 0x00F0) >> 4;
			uint16_t _n = (opcode & 0x000F);
			
			switch (_n)
			{
				case 0x0: { // 8XY0 Sets VX to the value of VY.
					printf("\tSetting VX to the value of VY.\n");
					cpu->V[_x] = cpu->V[_y];

					break;
				}

				case 0x2: { // 8XY2: Sets VX to VX and VY.
					printf("\tSetting VX to VX and VY.\n");
					cpu->V[_x] &= cpu->V[_y];

					break;
				}

				case 0x3: { // 8XY3: Sets VX to VX xor VY.
					printf("\tSetting VX to VX xor VY.\n");
					cpu->V[_x] ^= cpu->V[_y];

					break;
				}

				// 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, 
				// and to 0 when there isn't.
				case 0x4: { 
					printf("\tAdds VY to VX.\n");
					cpu->V[_x] += cpu->V[_y];

					break;
				}

				// 8XY6: Shifts VX right by one. VF is set to the value of 
				// the least significant bit of VX before the shift.
				case 0x6: {
					printf("\tShifted VX right by one bit.\n");
					cpu->V[0xF] = (opcode & 0x1);
					cpu->V[_x] >>= 0x1;

					break;
				}

				default: {
					printf("\tUnimplemented instruction.\n");
					exit(1);
				}
			}
		}

		case 0x9000: { // 9XY0: Skips the next instruction if VX doesn't equal VY.
			printf("Skips the next instruction if VX != VY.\n");
			uint16_t _x = (opcode & 0x0F00) >> 8;
			uint16_t _y = (opcode & 0x00F0) >> 4;

			if (cpu->V[_x] != cpu->V[_y]) {
				printf("\tSkipping.\n");
				cpu->pc += 2;
			}

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

		case 0xB000: { // BNNN: Jumps to the address NNN plus V0.
			printf("Jumping to 0x%x + V[0].\n", (opcode & 0x0FFF));
			printf("\tV[0] = %i.\n", cpu->V[0]);

			cpu->pc = (opcode & 0x0FFF) + cpu->V[0];
			break;
		}

		case 0xC000: { // CXNN: Sets VX to a random number and NN.
			printf("Setting VX to a random number and NN.\n");
			uint8_t _x = (opcode & 0x0F00) >> 8;
			uint8_t _nn = opcode & 0x00FF;

			uint16_t rand_number = rand();
			rand_number &= _nn;

			cpu->V[_x] = rand_number;
			printf("RANDOM NUMBER: %x\n", rand_number);

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0xD000: { // DXYN: Draw a sprite from I to position X, Y
			/* Parse out the values that's going to be needed */
			uint16_t x = cpu->V[(opcode & 0x0F00) >> 8];
			uint16_t y = cpu->V[(opcode & 0x00F0) >> 4];
			uint16_t height = (opcode & 0x000F);

			printf("Draw a sprite from I with height %i to x,y (%x,%x).\n", 
					height, x, y);

			// Reset the V[0xF] bit
			cpu->V[0xF] = 0x0;

			int _y, _x;

			// For each row in the sprite
			for (_y = 0; _y < height; _y++) {
				uint8_t line = cpu->memory[cpu->I + _y];
				
				// For each pixel
				for (_x = 0; _x < 8; _x++) {
					uint8_t pixel = line & (0x80 >> _x);
					uint32_t display_index = (64 * (_y + y)) + (x + _x);

					printf("%c", pixel ? 'x' : ' ');

					if (pixel != 0) {
						if (cpu->display[display_index] == 1)
							cpu->V[0xF] = 1;

						cpu->display[display_index] ^= 1;
					}
				}

				printf("\n");
			}

			// 1xxxxxxx = 0x80
			// x1xxxxxx = 0x40
			// xx1xxxxx = 0x20
			// xxx1xxxx = 0x10

			// Move to the next instruction
			cpu->pc += 2;
			break;
		}

		case 0xE000: { // Multivalued instruction
			printf("Entering multivalued instruction with 0x%x\n\t", opcode);
			uint16_t _x = (opcode & 0x0F00) >> 8;

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
			uint16_t _x = (opcode & 0x0F00) >> 8;

			switch (opcode & 0x00FF) 
			{
				// FX07: Sets VX to the value of the delay timer.
				case 0x07: {
					printf("Setting VX to the value of the delay timer.\n");
					cpu->V[_x] = cpu->delay_timer;
					break;
				}

				case 0x15: { // FX15: Sets the delay timer to VX.
					printf("Setting delay timer to VX.\n");
					cpu->delay_timer = cpu->V[_x];
					break;
				}

				case 0x18: { // FX18: Sets the sound timer to VX.
					printf("Setting sound timer to VX.\n");
					cpu->sound_timer = cpu->V[_x];
					break;
				}

				case 0x1E: { // FX1E Adds VX to I. (If overflow, set VF)
					printf("Adding VX to I.\n");

					/* If it overflows we need to set the carry flag */
					/* NOTE: Kinda cool that the compiler complained when tried
					 * using a uint8_t since that always will be false */
					uint16_t _i = cpu->I + cpu->V[_x];
					if (_i > 0xFFF) {
						printf("Overflow: Setting carry flag\n\t");
						cpu->V[0xF] = 1;
					}

					/* Do the addition */
					cpu->I += _i;
					break;
				}

				// FX29: Sets I to the location of the sprite for the character in VX. 
				// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				case 0x29: {
					printf("Setting I to the location of char %x.\n", _x);

					// One character takes up 5 bytes
					cpu->I = _x * 5;
					break;
				}

				// FX33: Store a binary coded representation of VX with the three most 
				// significant digits at I. Meaning that the number 156 would be placed as
				// I[0] = 1, I[1] = 5, I[2] = 6
				case 0x33: {
					printf("Storing a binary-coded representation of VX at I. (skipped)\n"); 
					printf("\tVX = %i\n", cpu->V[_x]);
					uint8_t hundreds, tens, ones;
					hundreds = tens = ones = 0;

					// Calculate the new value
					hundreds = cpu->V[_x] / 100;
					tens = (cpu->V[_x] / 10) - (hundreds * 10);
					ones = cpu->V[_x] % 10;
					
					printf("Hundreds: %i\n", hundreds);
					printf("Tens: %i\n", tens);
					printf("Ones: %i\n", ones);

					// Set them at I
					cpu->memory[cpu->I] = hundreds;
					cpu->memory[cpu->I + 1] = tens;
					cpu->memory[cpu->I + 2] = ones;
					break;
				}

				// FX55: Stores V0 to VX in memory starting at address I.
				case 0x55: {
					printf("Stores V0...VX in memory at I.\n");

					uint16_t _x = (opcode & 0x0F00) >> 8;
					memcpy(&cpu->memory[cpu->I], cpu->V, _x);
					break;
				}
				
				// FX65: Fills V0 to VX with values from memory starting at address I.
				case 0x65: {
					printf("Stores values from I in V0...VX.\n");

					uint16_t _x = (opcode & 0x0F00) >> 8;
					memcpy(cpu->V, &cpu->memory[cpu->I], _x);
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

	// Count down the timers
	cpu->sound_timer -= 1;
	cpu->delay_timer -= 1;
}

/* Tick the timers on the cpu */
void tick(chip8_t *cpu)
{
	cpu->delay_timer -= 1;
	cpu->sound_timer -= 1;
}

/* Return the display matrix */
uint8_t *get_display(chip8_t *cpu)
{
	return cpu->display;
}

/* Function which steps through the program */
void run_chip(chip8_t *cpu)
{
	for (;;) {
		usleep(10 * 1000);
		step(cpu);
	}
}
