#include <string.h>
#include <stdio.h>
#include "Memory.h"
#include "Timer.h"
#include "Cartridge.h"
#include "Debug.h"

#define ENABLE_EXTERNAL_RAM 0x2000
#define SWITCH_ROM_BANK 0x4000
#define SWITCH_RAM_BANK_ROM_SET 0x6000
#define SWITCH_CART_MODE 0x8000

char in_bios;

unsigned char read_8_bit(unsigned short addr) {
	if (addr < 0x4000) {
		if (in_bios && addr < 0x100)
			return bios[addr];
		return read_rom_bank_8_bit(addr, 1);
	}
	
	if (addr < 0x8000)
		return read_rom_bank_8_bit(addr - 0x4000, 0);
	if (addr < 0xA000)
		return vram[addr - 0x8000];
	if (addr < 0xC000)
		return read_ram_bank_8_bit(addr - 0xA000);
	if (addr < 0xE000)
		return internal_ram[addr - 0xC000];
	if (addr < 0xFE00)
		return internal_ram[addr - 0xE000];
	if (addr < 0xFF00)
		return sprite_info[addr - 0xFE00];	
	if (addr < 0xFF80)
		return io[addr - 0xFF00];
	
	return zero_pg_ram[addr - 0xFF80];
}

unsigned short read_16_bit(unsigned short addr) {
	return read_8_bit(addr) | (read_8_bit(addr + 1) << 8);
}

void write_8_bit(unsigned short addr, unsigned char val) {
	// Last step in bios to unmap the boot rom (https://realboyemulator.wordpress.com/2013/01/03/a-look-at-the-game-boy-bootstrap-let-the-fun-begin/)
	if (addr == 0xFF50 && val == 1)
		in_bios = 0;

	if (addr < ENABLE_EXTERNAL_RAM) {

		enable_disable_cart_ram(val);

	} else if(addr < SWITCH_ROM_BANK) {
		
		switch_rom_bank(val);

	} else if (addr < SWITCH_RAM_BANK_ROM_SET) {

		switch_ram_bank_rom_set(val);

	} else if (addr < SWITCH_CART_MODE) {

		switch_cart_mode(val);

	} else if (addr < 0xA000) {
		
		if(addr > 0x9800)
			debug_on_map_change();

		vram[addr - 0x8000] = val;

	} else if (addr < 0xC000) {

		write_ram_bank_8_bit(addr - 0xA000, val);

	} else if (addr < 0xE000) {

		internal_ram[addr - 0xC000] = val;

	} else if (addr < 0xFE00) {

		internal_ram[addr - 0xE000] = val;

	} else if (addr < 0xFF00) {

		sprite_info[addr - 0xFE00] = val;

	} else if (addr < 0xFF80) {

		//if (addr == 0xff0f && val == 0)
		//	printf("Interrupt Flag change!");

		switch (addr) {
			case 0xFF01:
				debug_log_serial_output(val);
				printf("%c", val);
				break;
			case DIVIDER_REGISTER:
				io[addr - 0xFF00] = 0;
				break;
			case TIMER_CONTROL:
				io[addr - 0xFF00] = val;
				set_freq();
				break;
			case LCD_SCANLINE:
				io[addr - 0xFF00] = 0;
				break;
			default:
				io[addr - 0xFF00] = val;
		}
	} else if (addr < 0x10000) {

		zero_pg_ram[addr - 0xFF80] = val;

	}
}

// Assumes that when type casting higher order bits are discarded
// unsure which byte goes where 
void write_16_bit(unsigned short addr, unsigned short val) {
	unsigned char one = (unsigned char)(val & 0x00ff);
	unsigned char two = (unsigned char)(val >> 8);

	write_8_bit(addr, one);
	write_8_bit(addr + 1, two);
}

void load_bios() {
	in_bios = 1;
}

// Writes to the IO space in memory without causing values to be set
// that would happen in a normal write operation. 
void memory_write_8_bit_io_no_side_effects(unsigned short addr, unsigned char val) {

}