#include <GLFW/glfw3.h>
#include <string>

class Chip8
{
private:
	// Current opcode.
	uint16_t opcode = 0;

	/**
	 * Memory map:
	 * 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	 * 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	 * 0x200-0xFFF - Program ROM and work RAM
	 */
	uint8_t* memory = new uint8_t[4096];

	// CPU general purpose registers : V0, V1 to V15 and VE for the carry flag.
	uint8_t* V = new uint8_t[16];

	// Index register that can have values from 0x000 to 0xFFF.
	uint16_t I = 0;

	// Program counter that can have values from 0x000 to 0xFFF.
	uint16_t pc = 0x200;

	// Pixel in array is either 1 or 0.
	uint8_t gfx[32][64];

	// Delay timer : when set above 0 will start count down at 60 Hz.
	uint8_t delay_timer = 0;

	// Sound timer : when set above 0 will start count down at 60 Hz and make buzzing sound.
	uint8_t sound_timer = 0;

	// Hex type keyboard (0x0-0xF).
	uint8_t* key = new uint8_t[16];

	// The stack is used to remember the current location before a jump is performed.
	uint16_t* stack = new uint16_t[16];

	// Stack pointer. The stack is used to remember the current location before a jump is performed.
	uint16_t sp = 0;

	// Refresh screen if true.
	bool drawFlag = false;

public:
	// Emulator's window.
	GLFWwindow* window;

	// Initializes the emulator.
	Chip8();

	// Loads the selected game.
	void loadGame(std::string game);

	// Emulates a Chip8 cycle.
	void emulateCycle();

	// Launchs the emulator
	void launch();

};