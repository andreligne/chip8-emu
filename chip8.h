#ifndef CHIP_H_
#define CHIP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

/* Fontset in bytes */
/*{ 
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
};*/

/*
 * 0000 0010
 * 1010 1010
 * ---------
 * 1010 1100
 */

/* Define a struct that contains the emulator variables */
typedef struct {
	uint8_t *memory; // RAM for the machine
	uint8_t *V; // Data registers
	uint16_t I; // Points to a specific point in the memory

	uint16_t *stack; // The stack (16 levels deep)

	uint16_t *display; // The screen matris

	// Timers
	uint16_t sound_timer;
	uint16_t delay_timer;
	
	uint8_t stackPointer; // The stack pointer
	uint16_t pc; // The PC
} chip8_t;

/* A function which initializes all values for the cpu */
void init_chip(chip8_t *);

/* Free all resources for the cpu */
void free_chip(chip8_t *);

/* Load the file into the cpus memory */
void load_file(chip8_t *, char *);

/* Step and handle one instruction into the program */
void step(chip8_t *);

/* Return the display matrix */
void *get_display(chip8_t *);

/* Function which steps through the program */
void run_chip(chip8_t *);

#endif
