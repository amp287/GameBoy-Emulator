#include "Memory.h"

unsigned char read_8_bit(unsigned short addr) {
	return memory[addr];
}
unsigned short read_16_bit(unsigned short addr) {
	unsigned short temp = 0;
	temp = memory[addr];
	temp << 8;
	temp & memory[addr + 1];
	return temp;
}

void write_8_bit(unsigned short addr, unsigned char val) {
	memory[addr] = val;
	return 0;
}

// Assumes that when type casting higher order bits are discarded
// unsure which byte goes where 
void write_16_bit(unsigned short addr, unsigned short val) {
	unsigned char one = (char)val;
	unsigned char two = val >> 8;

	memory[addr] = two;
	memory[addr + 1] = one;
}