#pragma once
#include <stdint.h>

class CPU
{
public:
	CPU();
	~CPU();

	void ResetRegisters();
	void LoadROM(const char* fileName);
	void PerformCycle();

	bool drawn;

	uint8_t pixels[64 * 32];
	uint8_t keys[16];

private:
	uint16_t stack[16];
	uint16_t stackPointer;

	uint8_t memory[4096];
	uint8_t V[16];

	uint16_t pc;
	uint16_t opcode;
	uint16_t index;

	uint16_t delay;
	uint16_t sound;

};

