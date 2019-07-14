#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include "Chip8.h"

using namespace std;

Chip8::Chip8()
{
	uint8_t chip8_fontset[80] =
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

	// Load fontset
	for (int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}

	// Clear screen
	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			gfx[i][j] = 0;
		}
	}

	// Initialize the library
	if (!glfwInit())
		exit(-1);

	// Create a windowed mode window and its OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	Chip8::window = glfwCreateWindow(640, 320, "Chip-8 Emulator", NULL, NULL);
	if (!Chip8::window)
	{
		glfwTerminate();
		exit(-1);
	}
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
	case 0x000: // Display and flow
		switch (opcode & 0x000F)
		{
		case 0x0000: // 00E0: Clears the screen
			for (int i = 0; i < 32; i++)
			{
				for (int j = 0; j < 64; j++)
				{
					gfx[i][j] = 0;
				}
			}

			drawFlag = true;
			pc += 2;
			break;

		case 0x000E: // 00EE: Returns from a subroutine
			pc = stack[sp];
			if (sp > 0)
			{
				--sp;
			}
			break;

		default:
			printf("Cannot execute opcode '0x%X'.\n", opcode);
			break;
		}
		break;

	case 0x1000: // 1NNN: Jumps to address NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000: // 2NNN: Calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

		// 4XNN: Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

		// 5XY0: Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

	case 0x6000: // 6XNN: Sets VX to NN.
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;

	case 0x7000: // 7XNN: Adds NN to VX. (Carry flag is not changed)
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000: // 8XY0: Sets VX to the Value of VY.
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 8XY1: Sets VX to VX or VY. (Bitwise OR operation)
		case 0x0001:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 8XY2: Sets VX to VX and VY. (Bitwise AND operation)
		case 0x0002:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 8XY3: Sets VX to VX xor VY.
		case 0x0003:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 8XY4 : Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
		case 0x0004:
			if (V[(opcode & 0x0F00) >> 8] > (0xFF - V[(opcode & 0x00F0) >> 4]))
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}

			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		case 0x0005:
			if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4])
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			// 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
		case 0x0006:
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

			// 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		case 0x0007:
			if (V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8])
			{
				V[0xF] = 1;
			}
			else
			{
				V[0xF] = 0;
			}

			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			// 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
		case 0x000E:
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x8000;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;

		default:
			printf("Cannot execute opcode '0x%X'.\n", opcode);
			break;
		}
		break;

		// 9XY0: Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
		{
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

		// ANNN : Sets I to the address NNN
	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;

		// BNNN: Jumps to the address NNN plus V0.
	case 0xB000:
		pc = V[0] + (opcode & 0x0FFF);
		break;

	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
		srand(time(0));
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
				 // Each row of 8 pixels is read as bit-coded starting from memory location I; 
				 // I value doesn't change after the execution of this instruction. 
				 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, 
				 // and to 0 if that doesn't happen
	{
		int startX = V[(opcode & 0x0F00) >> 8];
		int startY = V[(opcode & 0x00F0) >> 4];
		int height = opcode & 0x000F;
		uint8_t pixelLine;

		V[0xF] = 0;
		for (int row = 0; row < height; row++)
		{
			pixelLine = memory[I + row];
			for (int col = 0; col < 8; col++)
			{
				if ((pixelLine & (0x80 >> col)) != 0)
				{
					if (gfx[startY + row][startX + col] == 1)
					{
						V[0xF] = 1;
					}
					gfx[startY + row][startX + col] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
		break;
	}

	case 0xE000:
		switch (opcode & 0x000F)
		{
			// EX9E: Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
		case 0x000E:
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;

			// EXA1: Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
		case 0x0001:
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;

		default:
			printf("Cannot execute opcode '0x%X'.\n", opcode);
			break;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
			// FX07: Sets VX to the value of the delay timer.
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

			// FX0A: A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
		case 0x000A:
			pc += 2;
			break;

		case 0x0015: // FX15: Sets the delay timer to VX.
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018:// FX18: Sets the sound timer to VX.
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E: // FX1E: Adds VX to I.
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
			I = memory[V[(opcode & 0x0F00) >> 8] + 0x050];
			pc += 2;
			break;

		case 0x0033: // FX33: Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

		case 0x0055: // FX55: Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
			for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
			{
				memory[I + i] = V[i];
			}
			pc += 2;
			break;

		case 0x0065: // FX65: Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
			for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
			{
				V[i] = memory[I + i];
			}
			pc += 2;
			break;

		default:
			printf("Cannot execute opcode '0x%X'.\n", opcode);
			break;
		}
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

	printf("Opcode '0x%X' executed.\n", opcode);
}

void Chip8::launch()
{
	// Make the window's context current
	glfwMakeContextCurrent(Chip8::window);
	
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(Chip8::window))
	{
		// Emulate one cycle
		emulateCycle();

		// Store key press state (Press and Release)

		// Render here
		glClear(GL_COLOR_BUFFER_BIT);
		glPointSize(10);
		glColor3f(1.0, 0.0, 0.0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, 64, 32, 0.0f, 0.0f, 1.0f);

		// If the draw flag is set, update the screen
		if (drawFlag)
		{
			for (int i = 0; i < 32; i++)
			{
				for (int j = 0; j < 64; j++)
				{
					if (gfx[i][j])
					{
						glBegin(GL_POINTS);
						glVertex2f(j, i);
						glEnd();
					}
				}
			}
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process eVents */
		glfwPollEvents();
	}

	glfwTerminate();
}