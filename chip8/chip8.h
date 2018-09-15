#pragma once
//----------------------------------------------------------------------------
// chip8.h
//----------------------------------------------------------------------------

#include <string>

class Chip8 {
public:
	static const unsigned short fontBase = 0x50;
	static const unsigned short progBase = 0x200;
	static const unsigned short memorySize = 4096;
	static const unsigned short maxProgSize = 0xfff - 0x200;
	
	static const int screenWidth = 64;
	static const int screenHeight = 32;
	static const int screenSize = screenWidth * screenHeight;
	static const int numKeys = 16;
	static const int numRegs = 16;
	static const int stackSize = 16;

	unsigned short currentOpcode;

	// memory map
	// 0x000-0x1FF - Chip 8 interpreter
	// 0x050 - 0x0A0 - Used for the built in 4x5 pixel font set(0 - F)
	// 0x200 - 0xFFF - Program ROM and work RAM
	unsigned char memory[memorySize];

	unsigned char gfx[screenWidth * screenHeight];

	unsigned char keys[numKeys];

	enum KeyStatus
	{
		key_up,
		key_down
	};

	const unsigned char chip8Fontset[80] =
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

	unsigned char regs[numRegs];
	unsigned short pc;
	unsigned short I; // index register

	unsigned char delayTimer;
	unsigned char soundTimer;

	unsigned short stack[stackSize];
	unsigned short sp;
	bool drawFlag;
	bool beepFlag;

	Chip8() {};
	~Chip8() {};

	void reset();
	bool load(std::string filename);
	void tick();
	bool willDraw();
	bool willBeep();

	void decodeAndExecute(unsigned short opcode);
	void updateTimers();
};