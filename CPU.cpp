#include "CPU.h"
#include <random>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "time.h"

unsigned char fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

CPU::CPU()
{
	ResetRegisters();
}

CPU::~CPU()
{
}

void CPU::ResetRegisters()
{
	pc = 0x200;
	opcode = 0;
	index = 0;
	stackPointer = 0;

	for (int i = 0; i < 2048; i++)
		pixels[i] = 0;

	for (int i = 0; i < 16; i++)
	{
		stack[i] = 0;
		keys[i] = 0;
		V[i] = 0;
	}

	for (int i = 0; i < 4096; i++)
		memory[i] = 0;

	for (int i = 0; i < 80; i++)
		memory[i] = fontset[i];

	delay = 0;
	sound = 0;

	srand(time(NULL));
}

void CPU::LoadROM(const char* fileName)
{
	ResetRegisters();

	FILE* rom;
	errno_t err;
	
	fopen_s(&rom, fileName, "rb");
	fseek(rom, 0, SEEK_END);
	long romSize = ftell(rom);
	rewind(rom);

	char* buffer = (char*)malloc(sizeof(char) * romSize);
	size_t result = fread(buffer, sizeof(char), (size_t)romSize, rom);

	if ((4096 - 512) > romSize) 
	{
		for (int i = 0; i < romSize; i++)
		{
			memory[i + 512] = (uint8_t)buffer[i];
		}
	}

	fclose(rom);
	free(buffer);
}

void CPU::PerformCycle()
{
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Lowest 12 Bits
	uint16_t NNN = opcode & 0x0FFF;

	// Lowest 8 bits
	uint8_t NN = opcode & 0x00FF;

	// Lowest 4 bits
	uint8_t N = opcode & 0x000F;

	// Lower 4 bits of the upper byte
	uint8_t X = opcode >> 8 & 0x000F;

	// Upper 4 bits of the lower byte
	uint8_t Y = opcode >> 4 & 0x000F;

	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode & 0x000F)
		{
		//00E0 - Clears the screen
		case 0x0000:
			for (int i = 0; i < 2048; i++)
				pixels[i] = 0;

			drawn = true;
			pc += 2;
			break;

		//00EE - Returns from a subroutine. 
		case 0x000E:
			stackPointer--;
			pc = stack[stackPointer];
			pc += 2;
			break;
		}
		break;

	// 1NNN - Jumps to address NNN. 
	case 0x1000:
		pc = NNN;
		break;

	// 2NNN - Calls subroutine at NNN. 
	case 0x2000:
		stack[stackPointer] = pc;
		stackPointer++;
		pc = NNN;
		break;

	// 3XNN - Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block) 
	case 0x3000:
		if (V[X] == NN)
			pc += 4;
		else
			pc += 2;
		break;

	// 4XNN - Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block) 
	case 0x4000:
		if (V[X] != NN)
			pc += 4;
		else
			pc += 2;
		break;

	// 5XY0 - Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block) 
	case 0x5000:
		if (V[X] == V[Y])
			pc += 4;
		else
			pc += 2;
		break;

	// 6XNN - Sets VX to NN.
	case 0x6000:
		V[X] = NN;
		pc += 2;
		break;

	// 7XNN - Adds NN to VX. (Carry flag is not changed) 
	case 0x7000:
		V[X] += NN;
		pc += 2;
		break;

	// 8XY - Sets VX to VX or VY. (Bitwise OR operation) 
	case 0x8000:
		switch (N)
		{
		// 8XY0 - Sets VX to the value of VY. 
		case 0x0000:
			V[X] = V[Y];
			pc += 2;
			break;

		// 8XY1 - Sets VX to VX or VY. (Bitwise OR operation) 
		case 0x0001:
			V[X] = V[X] | V[Y];
			pc += 2;
			break;

		// 8XY2 - Sets VX to VX and VY. (Bitwise AND operation) 
		case 0x0002:
			V[X] = V[X] & V[Y];
			pc += 2;
			break;

		// 8XY3 - Sets VX to VX xor VY. 
		case 0x0003:
			V[X] = V[X] ^ V[Y];
			pc += 2;
			break;

		// 8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. 
		case 0x0004:
			if (V[Y] > (0xFF - V[X]))
				V[0xF] = 1;
			else
				V[0xF] = 0;

			V[X] += V[Y];
			pc += 2;
			break;

		// 8XY5 - VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. 
		case 0x0005:
			if (V[Y] > (0xFF - V[X]))
				V[0xF] = 1;
			else
				V[0xF] = 0;

			V[X] -= V[Y];
			pc += 2;
			break;

		// 8XY6 - Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
		case 0x0006:
			V[0xF] = V[X] & 0x1;
			V[X] >>= 1;
			pc += 2;
			break;

		// 8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. 
		case 0x0007:
			if (V[X] > V[Y])
				V[0xF] = 1;
			else
				V[0xF] = 0;

			V[X] = V[Y] = V[X];
			pc += 2;
			break;

		// 8XYE - Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
		case 0x000E:
			V[0xF] = V[X] >> 7;
			V[X] <<= 1;
			pc += 2;
			break;
		}
		break;

	// 9XY0 - Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block) 
	case 0x9000:
		if (V[X] != V[Y])
			pc += 4;
		else
			pc += 2;
		break;

	// ANNN - Sets I to the address NNN. 
	case 0xA000:
		index = NNN;
		pc += 2;
		break;

	// BNNN - Jumps to the address NNN plus V0. 
	case 0xB000:
		pc = NNN + V[0x0];
		break;

	// CXNN - Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. 
	case 0xC000:
		V[X] = (rand() % 0xFF) & NN;
		pc += 2;
		break;

	// DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen 
	case 0xD000:
	{
		uint16_t x = V[(opcode & 0x0F00) >> 8];
		uint16_t y = V[(opcode & 0x00F0) >> 4];
		uint16_t height = opcode & 0x000F;
		uint16_t pixel;

		V[0xF] = 0;

		for (int vertical = 0; vertical < height; vertical++)
		{
			pixel = memory[index + vertical];

			for (int horizontal = 0; horizontal < 8; horizontal++)
			{
				if ((pixel & (0x80 >> horizontal)) != 0)
				{
					if (pixels[(x + horizontal + ((y + vertical) * 64))] == 1)
						V[0xF] = 1;

					pixels[(x + horizontal + ((y + vertical) * 64))] ^= 1;
				}
			}
		}

		drawn = true;
		pc += 2;
	}
	break;

	// EX9E - Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block) 
	case 0xE000:
		switch (NN)
		{
		// EX9E - Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block) 
		case 0x009E:
			if (keys[V[X]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		// EXA1 - Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block) 
		case 0x00A1:
			if (keys[V[X]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		}
		break;

	case 0xF000:
		switch (NN)
		{
		// FX07 - Sets VX to the value of the delay timer. 
		case 0x0007:
			V[X] = delay;
			pc += 2;
			break;

		// FX0A - A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event) 
		case 0x000A: {
			bool pressed = false;

			for (int i = 0; i < 16; i++)
			{
				if (keys[i] != 0)
				{
					V[X] = i;
					pressed = true;
				}
			}

			if (!pressed)
				return;

			pc += 2;
		}
		break;

		// FX15 - Sets the delay timer to VX. 
		case 0x0015:
			delay = V[X];
			pc += 2;
			break;

		// FX18 - Sets the sound timer to VX. 
		case 0x0018:
			sound = V[X];
			pc += 2;
			break;

		// FX1E - Adds VX to I. VF is not affected.
		case 0x001E:
			index += V[X];
			pc += 2;
			break;

		// FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font. 
		case 0x0029:
			index = V[X] * 0x5;
			pc += 2;
			break;

		// FX33 - Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.) 
		case 0x0033:
			memory[index] = V[X] / 100;
			memory[index + 1] = (V[X] / 10) % 10;
			memory[index + 2] = V[X] % 10;
			pc += 2;
			break;
		
		// FX55 - Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
		case 0x0055:
			for (int i = 0; i <= X; i++)
				memory[index + i] = V[i];

			index += X + 1;
			pc += 2;
			break;

		// FX65 - Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
		case 0x0065:
			for (int i = 0; i <= X; i++)
				V[i] = memory[index + i];

			index += X + 1;
			pc += 2;
			break;
		}

		break;
	}

	if (delay > 0)
		delay--;
}
