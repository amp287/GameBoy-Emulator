#include "Memory.h"

unsigned char rom_cartidge[32768];
unsigned char vram[8192];
unsigned char ext_ram[8192];	// RAM on cartridge
unsigned char working_ram[8192];
unsigned char sprite_info[160];
unsigned char io[128];
unsigned char zero_pg_ram[128];

unsigned char read_8_bit(unsigned short addr) {
	if(addr < 0x8000)
		return rom_cartidge[addr];
	if (addr < 0xA000)
		return vram[addr - 0x8000];
	if (addr < 0xC000)
		return ext_ram[addr - 0xA000];
	if (addr < 0xE000)
		return working_ram[addr - 0xC000];
	if (addr < 0xFE00)
		return working_ram[addr - 0xE000];
	if (addr < 0xFF00)
		return sprite_info[addr - 0xFE00];
	if (addr < 0xFF80)
		return io[addr - 0xFF00];
	if (addr < 0x10000)
		return zero_pg_ram[addr - 0xFF80];
	return 0;
}

unsigned short read_16_bit(unsigned short addr) {
	return read_8_bit(addr) | (read_8_bit(addr + 1) << 8);
}

void write_8_bit(unsigned short addr, unsigned char val) {
	if (addr < 0x8000)
		rom_cartidge[addr] = val;
	else if (addr < 0xA000)
		vram[addr - 0x8000] = val;
	else if (addr < 0xC000)
		ext_ram[addr - 0xA000] = val;
	else if (addr < 0xE000)
		working_ram[addr - 0xC000] = val;
	else if (addr < 0xFE00)
		working_ram[addr - 0xE000] = val;
	else if (addr < 0xFF00)
		sprite_info[addr - 0xFE00] = val;
	else if (addr < 0xFF80)
		io[addr - 0xFF00] = val;
	else if (addr < 0x10000)
		zero_pg_ram[addr - 0xFF80] = val;

}

// Assumes that when type casting higher order bits are discarded
// unsure which byte goes where 
void write_16_bit(unsigned short addr, unsigned short val) {
	unsigned char one = (char)val;
	unsigned char two = val >> 8;

	write_8_bit(addr, two);
	write_8_bit(addr + 1, one);
}