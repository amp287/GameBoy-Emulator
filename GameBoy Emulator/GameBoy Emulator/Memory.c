#include "Memory.h"
#include <string.h>
#include <stdio.h>

char in_bios;

unsigned char read_8_bit(unsigned short addr) {
	if (addr < 0x8000) {
		if (in_bios && addr < 0x100)
				return bios[addr];
		return rom_cartidge[addr];
	}
		
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
	// Last step in bios to unmap the boot rom (https://realboyemulator.wordpress.com/2013/01/03/a-look-at-the-game-boy-bootstrap-let-the-fun-begin/)
	if (addr == 0xFF50 && val == 1)
		in_bios = 0;
	
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

void load_bios() {
	in_bios = 1;
	memcpy(rom_cartidge, bios, sizeof(bios));
}

void load_rom() {
	FILE *rom = fopen("Pokemon Blue.gb", "r");
}