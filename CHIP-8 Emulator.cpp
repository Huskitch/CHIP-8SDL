#include <iostream>
#include "CPU.h"
#include "Renderer.h"
#include <thread>

int main(int argc, char** argv)
{
	CPU* cpu = new CPU();
	cpu->LoadROM("test_opcode.ch8");

	Renderer* renderer = new Renderer();
	renderer->CreateWindow(1024, 512);

	while (true)
	{
		cpu->PerformCycle();

		renderer->ProcessInputs(cpu);
		renderer->Draw(cpu);

		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
}