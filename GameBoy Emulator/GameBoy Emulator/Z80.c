#include "Z80.h"
#include "Memory.h"
#include "stdio.h"

int fetch(CPU *cpu) {

	return 0;
}

int execute(CPU *cpu) {

	return 0;
}

void reset(CPU* cpu) {
	cpu->pc = 0;
	cpu->sp = 0;
	cpu->clock_m = 0;
	cpu->clock_t = 0;
	cpu->m = 0;
	cpu->t = 0;
	cpu->af = 0;
	cpu->bc = 0;
	cpu->de = 0;
	cpu->hl = 0;
}



//Opcodes

//8 bit Loads

void LD_nn_n(CPU *cpu, unsigned short *pc, unsigned short *n) {
	*n = read_8_bit(cpu->pc);
}


//16-Bit Loads
void LD_n_nn(CPU *cpu, int arg1, int arg2) {

}