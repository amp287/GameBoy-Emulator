#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CART_TYPE_ROM_ONLY 0
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
#define CART_MODE_ROM_ONLY 3

char name[17];
unsigned char cartridge_type;
unsigned char rom_size;
unsigned char ram_size;
unsigned char (*rom_banks)[0x4000];
unsigned char (*ram_banks)[0x2000];
unsigned short current_rom_bank;

// can be 0->[1,31], 1->[32, 63], 2->[64, 95], 3->[96,127]
unsigned char current_rom_set;
unsigned char current_ram_bank;
unsigned char current_mode;
unsigned char ram_enabled;

int set_rom_size(unsigned char size_code) {
	rom_size = -1;
	rom_banks = NULL;

	if (size_code < 7){
		rom_size = 2 << size_code;
		rom_banks = malloc(sizeof(unsigned char[0x4000]) * rom_size);
	} else {
		printf("ROM SIZE UNSUPPORTED:%x\n", size_code);
		return -1;
	}

	return 0;
}

int set_ram_size(unsigned char size_code) {
	ram_size = -1;
	ram_banks = NULL;

	switch (size_code) {
		case 0:
			ram_size = 0;
			break;
		case 1:
		case 2:
			ram_size = 1;
			break;
		case 3:
			ram_size = 4;
			break;
		case 4:
			ram_size = 16;
	}

	if (ram_size == -1)
		return -1;

	if (ram_size == 0)
		return 0;

	ram_banks = malloc(sizeof(unsigned char[0x2000]) * ram_size);
	
	return 0;
}

int cart_check(unsigned char cart_type) {
	switch (cartridge_type) {
		case CART_TYPE_ROM_ONLY :
			current_mode = CART_MODE_ROM_ONLY;
			current_rom_bank = 1;
			printf("Cartridge type: CART_TYPE_ROM_ONLY\n");
			break;
		case CART_TYPE_MBC1 :
			current_mode = CART_MODE_MBC1_16MBIT_8KB;
			current_rom_bank = 1;
			printf("Cartridge type: CART_TYPE_MBC1\n");
			break;
		default:
			printf("ERROR cart type (%d) not supported\n", cart_type);
			return -1;
	}

	return 0;
}

int load_rom(char *path) {
	FILE *rom;
	unsigned char buffer[0x4000];
	int i = 0;

	rom = fopen(path, "rb");

	if(rom == NULL)
		return -1;
		
	fread(buffer, 0x4000, 1, rom);

	memcpy(name, &buffer[0x134], 16);

	cartridge_type = buffer[0x147];
	
	if (cart_check(cartridge_type) != 0)
		return -1;

	if (set_rom_size(buffer[0x148]) != 0) {
		fclose(rom);
		return -1;
	}

	if (set_ram_size(buffer[0x149]) != 0) {
		free(rom_banks);
		fclose(rom);
		return -1;
	}

	memcpy(rom_banks[0], buffer, 0x4000);

	for (i = 1; i < rom_size; i++) {
		fread(rom_banks[i], 1, 0x4000, rom);
	}

	fclose(rom);
	return 0;
}

unsigned char read_rom_bank_8_bit(unsigned short addr, int bank_0) {
	if (bank_0)
		return rom_banks[0][addr];

	return rom_banks[current_rom_bank][addr];
}

unsigned char read_ram_bank_8_bit(unsigned short addr) {
	if (ram_enabled && ram_size != 0)
		return ram_banks[current_ram_bank][addr];
	else
		return 0;
}

void write_ram_bank_8_bit(unsigned short addr, unsigned char val) {
	if (ram_enabled && ram_size != 0)
		ram_banks[current_ram_bank][addr] = val;
}

void enable_disable_cart_ram(unsigned char val) {
	if (val == 0x0A)
		ram_enabled = 1;
	else
		ram_enabled = 0;
}

// Called on write to 6000-7FFF
void switch_cart_mode(unsigned char mode) {
	if (mode)
		current_mode = CART_MODE_MBC1_16MBIT_8KB;
	else
		current_mode = CART_MODE_MBC1_4MBIT_32KB;
}

// Called on write to 2000-3FFF
// switches ROM bank loaded into 4000-7FFF
void switch_rom_bank(unsigned char bank) {
	//add in checks
	if (bank == 0)
		bank = 1;

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

// Switches the rom set (the two most significant ROM address lines)
void switch_rom_set(unsigned char set) {
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

// Called upon write to 4000-5FFF
// Depending on the current mode, will either:
// - switch the current ram bank loaded on 4/32 mode, 
// - switch the current rom address significant bits on 16/8 mode.
void switch_ram_bank_rom_set(unsigned char bank) {
	if (current_mode == CART_MODE_MBC1_4MBIT_32KB)
		current_ram_bank = bank;
	else if (current_mode == CART_MODE_MBC1_16MBIT_8KB)
		switch_rom_set(bank);
}