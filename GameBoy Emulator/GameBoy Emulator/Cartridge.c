#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CART_TYPE_MBC1 1
#define CART_TYPE_MBC1_RAM 2
#define CART_TYPE_MBC1_RAM_BATT 3
#define CART_TYPE_MBC3_TIMER_BATT 0xF
#define CART_TYPE_MBC3_TIMER_RAM_BATT 0x10
#define CART_TYPE_MBC3 0x11
#define CART_TYPE_MBC3_RAM 0x12
#define CART_TYPE_MBC3_RAM_BATT 13

#define CART_MODE_MBC1_16MBIT_8KB 0
#define CART_MODE_MBC1_4MBIT_32KB 1

char name[17];
unsigned char cartridge_type;
unsigned char rom_size;
unsigned char ram_size;
unsigned char (*rom_banks)[0x8000];
unsigned char (*ram_banks)[0x2000];
unsigned short current_rom_bank;

// can be 0->[1,31], 1->[32, 63], 2->[64, 95], 3->[96,127]
unsigned char current_rom_set;
unsigned char current_ram_bank;
unsigned char current_mode;

int set_rom_size(unsigned char size_code) {
	if (size_code < 7){
		rom_size = 2 << size_code;
		rom_banks = malloc(sizeof(unsigned char[0x8000]) * rom_size);
	} else {
		printf("ROM SIZE UNSUPPORTED:%x\n", size_code);
		return -1;
	}

	return 0;
}

void set_ram_size(unsigned char size_code) {
	switch (size_code) {
		case 0:
			ram_size = 0;
			return;
		case 1:
		case 2:
			ram_size = 1;
			return;
		case 3:
			ram_size = 4;
			return;
		case 4:
			ram_size = 16;
	}

	ram_banks = malloc(sizeof(unsigned char[0x2000]) * ram_size);
}

int load_rom(char *path) {
	FILE *rom = fopen("../Roms/cpu_instrs.gb", "rb");
	unsigned char buffer[0x8000];
	int i = 0;

	fread(&buffer, 0x8000, 1, rom);

	memcpy(name, &buffer[0x134], 16);
	cartridge_type = buffer[0x147];

	if (set_rom_size(buffer[0x148]) != 0) {
		fclose(rom);
		return -1;
	}

	

	fclose(rom);
	return 0;
}

void switch_rom_bank(unsigned char bank) {
	//add in checks
	switch (cartridge_type) {
		case CART_TYPE_MBC1:
		case CART_TYPE_MBC1_RAM:
		case CART_TYPE_MBC1_RAM_BATT:
			current_rom_bank = current_rom_set + bank;
			break;
		
		case CART_TYPE_MBC3_TIMER_BATT:
		case CART_TYPE_MBC3_TIMER_RAM_BATT:
		case CART_TYPE_MBC3:
		case CART_TYPE_MBC3_RAM:
		case CART_TYPE_MBC3_RAM_BATT:
			current_rom_bank = bank;
	}
}

void switch_rom_set(unsigned char set) {
	if (current_mode == CART_MODE_MBC1_16MBIT_8KB) {
		switch (set) {
			case 0:
				current_rom_set = 1;
				return;
			case 1:
				current_rom_set = 32;
				return;
			case 2:
				current_rom_set = 64;
				return;
			case 3:
				current_rom_set = 96;
		}
	}
}

// switches the ram bank or rom set depending on the Cartridge type and mode
void switch_ram_bank_rom_set(unsigned char bank) {
	if(current_mode == CART_MODE_MBC1_4MBIT_32KB)
		current_ram_bank = bank;
}

unsigned char read_rom_bank_8_bit(unsigned short addr) {
		return rom_banks[current_rom_bank][addr];
}

unsigned short read_rom_bank_16_bit(unsigned short addr) {
	return read_rom_bank_8_bit(addr) | (read_rom_bank_8_bit(addr + 1) << 8);
}

unsigned char read_ram_bank_8_bit(unsigned short addr) {
	return ram_banks[current_ram_bank][addr];
}

unsigned short read_ram_bank_16_bit(unsigned short addr) {
	return read_ram_bank_8_bit(addr) | (read_ram_bank_8_bit(addr + 1) << 8);
}

void write_ram_bank_8_bit(unsigned short addr, unsigned char val) {
	ram_banks[current_ram_bank][addr] = val;
}

void write_ram_bank_16_bit(unsigned short addr, unsigned short val) {
	unsigned char one = (unsigned char)(val & 0x00ff);
	unsigned char two = (unsigned char)(val >> 8);

	write__ram_bank_8_bit(addr, one);
	write_ram__bank_8_bit(addr + 1, two);
}