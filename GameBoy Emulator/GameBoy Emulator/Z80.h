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

static INSTR Opcodes[] = {
	{"NOP", NULL, NULL, NULL},			//0x00
	{"LD BC, nn", NULL, "BC", NULL},		//0x01
	{"LD BC, A", NULL, "BC", "A"},		//0x02
	{"INC BC", NULL, "BC", NULL},		//0x03
	{"INC B", NULL, "B", NULL},			//0x04
	{"DEC B", NULL, "B", NULL},			//0x05
	{"LD B, n", NULL, "B", NULL},		//0x06
	{"RLC A", NULL, "A", NULL},			//0x07
	{"LD nn, SP", NULL, NULL, "SP"},	//0x08
	{"ADD HL, BC", NULL, "HL", "BC"},	//0x09
	{"LD A, BC", NULL, "A", "BC"},		//0x0A
	{"DEC BC", NULL, "BC", NULL},		//0x0B
	{"INC C", NULL, "C", NULL},			//0x0C
	{"INC D", NULL, "D", NULL},			//0x0D
	{"LD C, n", NULL, "C", NULL},		//0x0E
	{"RRC A", NULL, "A", NULL},			//0x0F
	{"STOP", NULL, NULL, NULL},			//0x10
	{"LD DE, nn", NULL, "DE", NULL},	//0x11
	{"LD DE, A", NULL, "DE", "A"},		//0x12
	{"INC DE", NULL, "DE", NULL},		//0x13
	{"INC D", NULL, "D", NULL},			//0x14
	{"DEC D", NULL, "D", NULL},			//0x15
	{"LD D, n", NULL, "D", NULL},		//0x16
	{"RL A", NULL, "A", NULL},			//0x17
	{"JR n", NULL, NULL, NULL},			//0x18
	{"ADD HL, DE", NULL, "HL", "DE"},	//0x19
	{"LD A, DE", NULL, "A", "DE"},		//0x1A
	{"DEC DE", NULL, "DE", NULL},		//0x1B
	{"INC E", NULL, "E", NULL},			//0x1C
	{"DEC E", NULL, "E", NULL},			//0x1D
	{"LD E, n", NULL, "E", NULL},		//0x1E
	{"RR A", NULL, "A", NULL},			//0x1F
	{"JR NZ, n", NULL, "NZ", NULL},		//0x20		//NZ is check for Not zero
	{"LD HL, nn", NULL, "HL", NULL},	//0x21
	{"LDI HL, A", NULL, "HL", "A"},		//0x22
	{"INC HL", NULL, "HL", NULL},		//0x23
	{"INC H", NULL, "H", NULL},			//0x24
	{"DEC H", NULL, "H", NULL},			//0x25
	{"LD H, n", NULL, "H", NULL},		//0x26
	{"DAA", NULL, NULL, NULL},			//0x27
	{"JR Z, n", NULL, "Z", NULL},		//0x28		//Z is check for zero
	{"ADD HL, HL", NULL, "HL", "HL"},	//0x29
	{"LDI A, HL", NULL, "A", "HL"},		//0x2A
	{"DEC HL", NULL, "HL", NULL},		//0x2B
	{"INC L", NULL, "L", NULL},			//0x2C
	{"DEC L", NULL, "L", NULL},			//0x2D
	{"LD L, n", NULL, "L", NULL},		//0x2E
	{"CPL", NULL, NULL, NULL},			//0x2F
	{"JR NC, n", NULL, "NC", NULL},		//0x30		//NC is check for no carry
	{"LD SP, nn", NULL, "SP", NULL},	//0x31
	{"LDD HL, A", NULL, "HL", "A"},		//0x32
	{"INC SP", NULL, "SP", NULL},		//0x33
	{"INC HL", NULL, "HL", NULL},		//0x34
	{"DEC HL", NULL, "HL", NULL},		//0x35
	{"LD HL, n", NULL, "HL", NULL},		//0x36
	{"SCF", NULL, NULL, NULL},			//0x37
	{"JR C, n", NULL, "C", NULL},		//0x38		//C is check for carry
	{"ADD HL, SP", NULL, "HL", "SP"},	//0x39
	{"LDD A, HL", NULL, "A", "HL"},		//0x3A
	{"DEC SP", NULL, "SP", NULL},		//0x3B
	{"INC A", NULL, "A", NULL},			//0x3C
	{"DEC A", NULL, "A", NULL},			//0x3D
	{"LD A, n", NULL, "A", NULL},		//0x3E
	{"CCF", NULL, NULL, NULL},			//0x3F
	{"LD B, B", NULL, "B", "B"},		//0x40
	{"LD B, C", NULL, "B", "C"},		//0x41
	{"LD B, D", NULL, "B", "D"},		//0x42
	{"LD B, E", NULL, "B", "E"},		//0x43
	{"LD B, H", NULL, "B", "H"},		//0x44
	{"LD B, L", NULL, "B", "L"},		//0x45
	{"LD B, HL", NULL, "B", "HL"},		//0x46
	{"LD B, A", NULL, "B", "A"},		//0x47
	{"LD C, B", NULL, "C", "B"},		//0x48
	{"LD C, C", NULL, "C", "C"},		//0x49
	{"LD C, D", NULL, "C", "D"},		//0x4A
	{"LD C, E", NULL, "C", "E"},		//0x4B
	{"LD C, H", NULL, "C", "H"},		//0x4C
	{"LD C, L", NULL, "C", "L"},		//0x4D
	{"LD C, HL", NULL, "C", "HL"},		//0x4E
	{"LD C, A", NULL, "C", "A"},		//0x4F
	{"LD D, B", NULL, "D", "B"},		//0x50
	{"LD D, C", NULL, "D", "C"},		//0x51
	{"LD D, D", NULL, "D", "D"},		//0x52
	{"LD D, E", NULL, "D", "E"},		//0x53
	{"LD D, H", NULL, "D", "H"},		//0x54
	{"LD D, L", NULL, "D", "L"},		//0x55
	{"LD D, HL", NULL, "D", "HL"},		//0x56
	{"LD D, A", NULL, "D", "A"},		//0x57
	{"LD E, B", NULL, "E", "B"},		//0x58
	{"LD E, C", NULL, "E", "C"},		//0x59
	{"LD E, D", NULL, "E", "D"},		//0x5A
	{"LD E, E", NULL, "E", "E"},		//0x5B
	{"LD E, H", NULL, "E", "H"},		//0x5C
	{"LD E, L", NULL, "E", "L"},		//0x5D
	{"LD E, HL", NULL, "E", "HL"},		//0x5E
	{"LD E, A", NULL, "E", "A"},		//0x5F
	{"LD H, B", NULL, "H", "B"},		//0x60
	{"LD H, C", NULL, "H", "C"},		//0x61
	{"LD H, D", NULL, "H", "D"},		//0x62
	{"LD H, E", NULL, "H", "E"},		//0x63
	{"LD H, H", NULL, "H", "H"},		//0x64
	{"LD H, L", NULL, "H", "L"},		//0x65
	{"LD H, HL", NULL, "H", "HL"},		//0x66
	{"LD H, A", NULL, "H", "A"},		//0x67
	{"LD L, B", NULL, "L", "B" },		//0x68
	{"LD L, C", NULL, "L", "C" },		//0x68
};