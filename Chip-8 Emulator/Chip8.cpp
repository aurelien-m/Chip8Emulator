#include <iostream>
#include <fstream>
#include <string>
#include "Chip8.h"

using namespace std;

Chip8::Chip8()
{
	pc = 0x200; // Program counter starts at 0x200
	opcode = 0; // Reset current opcode
	I = 0;      // Reset index register
	sp = 0;     // Reset stack pointer

	// Clear display
	// Clear stack
	// Clear registers V0-VF
	// Clear memory

	// Load fontset
	for (int i = 0; i < 80; ++i)
	{
		//memory[i] = chip8_fontset[i];
	}

	// Reset timers
}

void Chip8::loadGame(string game)
{
	// Game selection
	ifstream gameFile;
	gameFile.open("Games/" + game + ".ch8", ios::ate | ios::out | ios::binary);

	string line;
	int counter = 0;

	if (gameFile.is_open())
	{
		int bufferSize = gameFile.tellg();
		char* buffer = new char[bufferSize];

		gameFile.seekg(0, ios::beg);
		gameFile.read(buffer, bufferSize);

		for (int i = 0; i < bufferSize; i++)
		{
			memory[512 + i] = buffer[i];
		}

		printf("Game file '%s.ch8' (%d bytes) loaded.\n", game.c_str(), bufferSize);
	}
	else
	{
		printf("Cannot open file: '%s.ch8'.\n", game.c_str());
	}

	gameFile.close();
}

void Chip8::emulateCycle()
{
	// Fetching upcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Decoding upcode
	switch (opcode & 0xF000)
	{
	// Calls subroutine at NNN
	case 0x2000:
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	// Bit and math operations
	case 0x8000:
		switch (opcode & 0x000F)
		{
		// 8XY4 : Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
		case 0x0004:
			unsigned char xIndex = (opcode & 0x00F0) >> 4;
			unsigned char yIndex = (opcode & 0x0F00) >> 8;

			if (V[xIndex] > (0xFF - V[yIndex]))
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}

			V[xIndex] += V[yIndex];
			pc += 2;
			break;
		}
		break;

	// ANNN : Sets I to the address NNN
	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	default:
		printf("Cannot execute opcode '0x%X'.\n", opcode);
		break;
	}

	// Update timers
	if (delay_timer > 0)
	{
		--delay_timer;
	}

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
		{
			printf("BEEP!\n");
		}

		--sound_timer;
	}
}