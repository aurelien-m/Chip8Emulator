#include <string>

class Chip8
{
private:
	// Current opcode.
	unsigned short opcode;

	/**
	 * Memory map:
	 * 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	 * 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	 * 0x200-0xFFF - Program ROM and work RAM
	 */
	unsigned char memory[4096];

	// CPU general purpose registers : V0, V1 to V15 and VE for the carry flag.
	unsigned char V[16];

	// Index register that can have values from 0x000 to 0xFFF.
	unsigned short I;

	// Program counter that can have values from 0x000 to 0xFFF.
	unsigned short pc;

	// Pixel in array is either 1 or 0.
	unsigned char gfx[64 * 32];

	// Delay timer : when set above 0 will start count down at 60 Hz.
	unsigned char delay_timer;

	// Sound timer : when set above 0 will start count down at 60 Hz and make buzzing sound.
	unsigned char sound_timer;

	// Hex type keyboard (0x0-0xF).
	unsigned char key[16];

	// The stack is used to remember the current location before a jump is performed.
	unsigned short stack[16];

	// Stack pointer. The stack is used to remember the current location before a jump is performed.
	unsigned short sp;

public:
	// Refresh screen if true.
	bool drawFlag;

	// Initializes the emulator.
	Chip8();

	// Loads the selected game.
	void loadGame(std::string game);

	// Emulates a Chip8 cycle.
	void emulateCycle();
};