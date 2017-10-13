#include <stdlib.h>

#define ZERO_FLAG 0x80
#define OPERATION_FLAG 0x40
#define HALF_CARRY_FLAG 0x20
#define CARRY_FLAG 0x10


typedef struct {
	// Program Counter
	unsigned short pc;
	// Stack Pointer
	unsigned short sp;

	// CPU total time
	long clock_m, clock_t;

	// Last instruction time 
	long m, t;
	
	// Registers
	union {
		struct {
			unsigned char f;
			unsigned char a;
		};
		unsigned short af;
	};

	union {
		struct {
			unsigned char c;
			unsigned char b;
		};
		unsigned short bc;
	};

	union {
		struct {
			unsigned char e;
			unsigned char d;
		};
		unsigned short de;
	};

	union {
		struct {
			unsigned char l;
			unsigned char h;
		};
		unsigned short hl;
	};

}CPU;

void reset(CPU *cpu);
int fetch(CPU *cpu);
int execute(CPU *cpu);

//Function Pointer
typedef void (*OPCODE_OPERATION)(CPU* cpu, unsigned short *r1, unsigned short *r2);

typedef struct {
	char *disassembly;
	OPCODE_OPERATION execute;
	char r1[2];
	char r2[2];
}INSTR;

const INSTR Opcodes[] = {
	{"NOP", NULL, NULL, NULL},
	{"LD BC,nn", NULL, "BC", },
	{"LD "}
};