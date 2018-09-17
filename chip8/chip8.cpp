//----------------------------------------------------------------------------
// chip8.cpp
//----------------------------------------------------------------------------

#include "chip8.h"
#include <fstream>
#include <iostream>

//----------------------------------------------------------------------------
// reset
//----------------------------------------------------------------------------
void Chip8::reset()
{
	pc = progBase;
	currentOpcode = 0;
	I = 0;
	sp = 0;
	drawFlag = false;
	soundTimer = 0;
	delayTimer = 0;

	// clear gfx and memory etc.
	memset(gfx, 0, screenSize);
	memset(memory, 0, memorySize);
	memset(stack, 0, stackSize * sizeof(unsigned short));
	memset(regs, 0, 16);
	memset(keys, 0, 16);

	// Load fontset
	for (int i = 0; i < 80; ++i) 
	{
		memory[fontBase + i] = chip8Fontset[i];
	}
}

//----------------------------------------------------------------------------
// load
//----------------------------------------------------------------------------
bool Chip8::load(std::string filename)
{
	// lets try and load a rom file
	std::basic_fstream<unsigned char> romFile;
	romFile.open(filename, std::ios::binary | std::ios::in);

	if(romFile.is_open())
	{
		// how long is the ROM file?
		std::streampos fsize = romFile.tellg();
		romFile.seekg(0, std::ios::end);
		fsize = romFile.tellg() - fsize;
		romFile.seekg(0, std::ios::beg);

		if (fsize <= maxProgSize)
		{
			unsigned char *m = memory + progBase;
			romFile.read(m, fsize);
			std::cout << "Loaded ROM " << filename << std::endl;
			return true;
		}
		else
		{
			std::cout << "ROM file is bigger than available memory. Size:" 
				<< fsize << " mem: " << maxProgSize << std::endl;
		}
	}
	else
	{
		std::cout << "Could not load ROM file " << filename << std::endl;
	}

	return false;
}

//----------------------------------------------------------------------------
// tick
//----------------------------------------------------------------------------
void Chip8::tick()
{
	currentOpcode = memory[pc] << 8 | memory[pc + 1];
	decodeAndExecute(currentOpcode);
	return;
}

//----------------------------------------------------------------------------
// updateTimers - Chip8 timers update at 60hz
//----------------------------------------------------------------------------
void Chip8::updateTimers()
{
	if (delayTimer > 0)
	{
		delayTimer -= 1;
	}

	beepFlag = false;
	if (soundTimer > 0)
	{
		if (soundTimer == 1)
		{
			beepFlag = true;
		}
		soundTimer -= 1;
	}
}

//----------------------------------------------------------------------------
// willDraw
//----------------------------------------------------------------------------
bool Chip8::willDraw() 
{
	return drawFlag;
}

//----------------------------------------------------------------------------
// willBeep
//----------------------------------------------------------------------------
bool Chip8::willBeep()
{
	return beepFlag;
}

//----------------------------------------------------------------------------
// decodeAndExecute
//----------------------------------------------------------------------------
void Chip8::decodeAndExecute(unsigned short opcode)
{
	// first 4 bits of opcode will tell us what the instruction is
	// opcode info from https://en.wikipedia.org/wiki/CHIP-8
	// and http://mattmik.com/files/chip8/mastering/chip8.html

	switch (opcode & 0xf000)
	{
		case 0x0000:
			switch (opcode & 0x000f)
			{	
				case 0x0000:
					//00E0    disp_clear()    Clears the screen.
					memset(gfx, 0, screenSize);
					drawFlag = true;
					pc += 2;
				break;

				case 0x000E: 
					//00EE return; Returns from a subroutine.
					pc = stack[--sp];
					pc += 2;
				break;
				// are we going to do 0x0NNN (call rca?)
			default:
				std::cout << "Unknown opcode " << std::hex << opcode << std::endl;
				break;
			}
			break;

		case 0x1000:
			// 1NNN 	goto NNN;	Jumps to address NNN.
			pc = opcode & 0x0fff;
			break;
		case 0x2000:
			//2NNN	Flow	*(0xNNN)()	Calls subroutine at NNN.
			stack[sp++] = pc;
			pc = opcode & 0x0fff;
			break;	
		case 0x3000:
			// 3XNN	Cond if (Vx == NN) Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
			if (regs[(opcode & 0x0f00) >> 8] == (opcode & 0x00ff)) 
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;
		case 0x4000:
			// skip if Vx != NN
			if (regs[(opcode & 0x0f00) >> 8] != (opcode & 0x00ff))
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;
		case 0x5000:
			// skip if Vx == Vy
			if (regs[(opcode & 0x0f00) >> 8] == regs[(opcode &0x00f0) >> 4])
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;
		case 0x6000:
			// set Vx to NN
			regs[(opcode & 0x0f00) >> 8] = opcode & 0x00ff;
			pc += 2;
			break;
		case 0x7000:
			// add NN to Vx
			regs[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
			pc += 2;
			break;
		case 0x8000:
			switch (opcode & 0x000f)
			{
				case 0x0000:
					//	Vx = Vy	
					regs[(opcode & 0x0f00) >> 8] = regs[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;
				case 0x0001:
					// Vx = Vx | Vy	
					regs[(opcode & 0x0f00) >> 8] |= regs[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;
				case 0x0002:
					// Vx = Vx & Vy	
					regs[(opcode & 0x0f00) >> 8] &= regs[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;
				case 0x0003:
					// Vx = Vx^Vy
					regs[(opcode & 0x0f00) >> 8] ^= regs[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;
				case 0x0004:
					// Vx += Vy 	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
					if (regs[(opcode & 0x00f0) >> 4] > (0xFF - regs[(opcode & 0x0f00) >> 8]))
					{
						regs[0xf] = 1; //carry
					}
					else
					{
						regs[0xf] = 0;
					}
					regs[(opcode & 0x0f00) >> 8] += regs[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;
				case 0x0005:
					//Vx -= Vy	VY is subtracted from VX.VF is set to 0 when there's a borrow, and 1 when there isn't.
					if (regs[(opcode & 0x00f0) >> 4] > regs[(opcode & 0x0f00) >> 8])
					{
						regs[0xf] = 0; //borrow
					}
					else
					{
						regs[0xf] = 1;
					}
					regs[(opcode & 0x0f00) >> 8] -= regs[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;
				case 0x0006:
					// Vx >>= 1	Stores the least significant bit of VX in VF and then shifts VX to the right by 1
					regs[0xf] = regs[(opcode & 0x0f00) >> 8] & 0x1;
					regs[(opcode & 0x0f00) >> 8] >>= 1;
					pc += 2;
					break;
				case 0x0007:
					//Vx = Vy - Vx	Sets VX to VY minus VX.VF is set to 0 when there's a borrow, and 1 when there isn't.
					if (regs[(opcode & 0x0f00) >> 8] > regs[(opcode & 0x00f0) >> 4])
					{
						regs[0xf] = 0; //borrow
					}
					else
					{
						regs[0xf] = 1;
					}
					regs[(opcode & 0x0f00) >> 8] = regs[(opcode & 0x00f0) >> 4] - regs[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;
				case 0x000e:
					// Vx <<= 1	Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
					regs[0xf] = regs[(opcode & 0x0f00) >> 8] >> 7;
					regs[(opcode & 0x0f00) >> 8] <<= 1;
					pc += 2;
					break;
				default:
					std::cout << "Unknown opcode " << std::hex << opcode << std::endl;
					break;
			}
			break;
		case 0x9000:
			// if (Vx != Vy)	Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
			if (regs[(opcode & 0x0f00) >> 8] != regs[(opcode & 0x00f0) >> 4])
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;
		case 0xa000:
			//I = NNN	Sets I to the address NNN.
			I = opcode & 0x0fff;
			pc += 2;
			break;
		case 0xb000:
			//PC=V0+NNN	Jumps to the address NNN plus V0.
			pc = (opcode & 0x0fff) + regs[0];
			break;
		case 0xc000:
			// Vx=rand()&NN	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
			regs[(opcode & 0x0f00) >> 8] = (rand() % 255) & (opcode & 0x00ff);
			pc += 2;
			break;
		case 0xd000:
		{
			// Draws a sprite at coordinate (VX, VY)
			unsigned short x = regs[(opcode & 0x0f00) >> 8];
			unsigned short y = regs[(opcode & 0x00f0) >> 4];
			unsigned short height = opcode & 0x000f;
			
			regs[0xF] = 0;
			for (int yLine = 0; yLine < height; yLine++)
			{
				unsigned short pixel = memory[I + yLine];
				for (int xLine = 0; xLine < 8; xLine++)
				{
					if ((pixel & (0x80 >> xLine)) != 0)
					{
						if (gfx[(x + xLine + ((y + yLine) * 64))] == 1)
						{
							regs[0xF] = 1;
						}
						gfx[x + xLine + ((y + yLine) * 64)] ^= 1;
					}
				}
			}

			drawFlag = true;
			pc += 2;
		}
			break;
		case 0xe000:
			switch (opcode & 0x00ff)
			{
				case 0x009e:
				// if (key() == Vx)	Skips the next instruction if the key stored in VX is pressed.
					if (keys[regs[(opcode & 0x0f00) >> 8]] == 1)
					{
						pc += 4;
					}
					else
					{
						pc += 2;
					}
					break;
				case 0x00a1:
				// if(key()!=Vx)	Skips the next instruction if the key stored in VX isn't pressed.
					if (keys[regs[(opcode & 0x0f00) >> 8]] == 0)
					{
						pc += 4;
					}
					else
					{
						pc += 2;
					}
					break;
				default:
					std::cout << "Unknown opcode " << std::hex << opcode << std::endl;
					break;
			}
			break;
		case 0xf000:
			switch (opcode & 0x00ff)
			{
				case 0x0007:
					// Vx = get_delay()	Sets VX to the value of the delay time
					regs[(opcode & 0x0f00) >> 8] = delayTimer;
					pc += 2;
					break;
				case 0x000a:
				{
					// Vx = get_key()	A key press is awaited, and then stored in VX. (Blocking Operation.All instruction halted until next key event)
					bool keyPress = false;
					for (int i = 0; i < 16; ++i)
					{
						if (keys[i] != 0)
						{
							regs[(opcode & 0x0f00) >> 8] = i;
							keyPress = true;
						}
					}
					// dont move on until we've had a keypress
					if (keyPress)
					{
						pc += 2;
					}
				}
					break;
				case 0x0015:
					// delay_timer(Vx)	Sets the delay timer to VX.
					delayTimer = regs[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;
				case 0x0018:
					// sound_timer(Vx)	Sets the sound timer to VX.
					soundTimer = regs[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;
				case 0x001e:
					// I += Vx	Adds VX to I
					// VF is set to 1 when range overflow (I+VX > 0xFFF), and 0 when there isn't.
					if (I + regs[(opcode & 0x0f00) >> 8] > 0xFFF)
					{
						regs[0xF] = 1;
					}
					else
					{
						regs[0xF] = 0;
					}
					I += regs[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;
				case 0x0029:
					//I = sprite_addr[Vx]	Sets I to the location of the sprite for the character in VX
					I =  fontBase + (((opcode & 0x0f00) >> 8) * 5);
					pc += 2;
					break;
				case 0x0033:
					// bcd
					memory[I] = regs[(opcode & 0x0f00) >> 8] / 100;
					memory[I + 1] = (regs[(opcode & 0x0f00) >> 8] / 10) % 10;
					memory[I + 2] = (regs[(opcode & 0x0f00) >> 8] % 100) % 10;
					pc += 2;
					break;
				case 0x0055:
					// reg_dump(Vx,&I)	Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
					for (int j = 0; j <= ((opcode & 0x0f00) >> 8); j++)
					{
						memory[I + j] = regs[j];
					}

					// On the original interpreter, when the operation is done, I = I + X + 1.
					I += ((opcode & 0x0f00) >> 8) + 1;
					pc += 2;
					break;
				case 0x0065:
					// reg_load(Vx,&I)	Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
					for (int j = 0; j <= ((opcode & 0x0f00) >> 8); j++)
					{
						regs[j] = memory[I + j];
					}

					// On the original interpreter I = I + X + 1.
					I += ((opcode & 0x0f00) >> 8) + 1;
					pc += 2;
					break;
				default:
					std::cout << "Unknown opcode " << std::hex << opcode << std::endl;
					break;
			}
			break;

	default:
		std::cout << "Unknown opcode " << std::hex << opcode << std::endl;
	}
}
