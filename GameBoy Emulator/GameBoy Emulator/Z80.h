#include <stdlib.h>

#define ZERO_FLAG 0x80
#define SUBTRACT_FLAG 0x40
#define HALF_CARRY_FLAG 0x20
#define CARRY_FLAG 0x10

//cf is carry flag
enum Regval { A, B, C, D, E, H, L, AF, BC, DE, HL, Z, Cf, NC, NZ, SP};

typedef struct {
	// Program Counter
	unsigned short pc;
	// Stack Pointer
	unsigned short sp;

	// CPU total time
	long clock_m, clock_t;

	// Last instruction time 
	long m, t;

	unsigned char flags;
	
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

void cpu_init();
void cpu_reset();
int cpu_fetch();
int cpu_execute();

//Function Pointer
typedef void (*OPCODE_OPERATION)(int arg1, int arg2);

typedef struct {
	char *disassembly;
	OPCODE_OPERATION execute;
	void *r1;
	void *r2;
}INSTR;

//8-Bit Loads
void LD_nn_n(int arg1, int arg2);
void LD_r1_r2(int arg1, int arg2);
void LD_A_n(int arg1, int arg2);
void LD_n_A(int arg1, int arg2);
void LD_A_C(int arg1, int arg2);
void LD_C_A(int arg1, int arg2);
void LDD_A_HL(int arg1, int arg2);
void LDD_HL_A(int arg1, int arg2);
void LDI_A_HL(int arg1, int arg2);
void LDI_HL_A(int arg1, int arg2);
void LDH_n_A(int arg1, int arg2);
void LDH_A_n(int arg1, int arg2);
void LDI_A_HL(int arg1, int arg2);

//16-Bit Loads
void LD_n_nn(int arg1, int arg2);

static INSTR Opcodes[] = {
	{"NOP", NULL, NULL, NULL},			//0x00
	{"LD BC, nn", LD_n_nn, BC, NULL},	//0x01
	{"LD BC, A", NULL, BC, A},		//0x02
	{"INC BC", NULL, BC, NULL},		//0x03
	{"INC B", NULL, B, NULL},			//0x04
	{"DEC B", NULL, B, NULL},			//0x05
	{"LD B, n", NULL, B, NULL},		//0x06
	{"RLC A", NULL, A, NULL},			//0x07
	{"LD nn, SP", NULL, NULL, SP},	//0x08
	{"ADD HL, BC", NULL, HL, BC},	//0x09
	{"LD A, BC", NULL, A, BC},		//0x0A
	{"DEC BC", NULL, BC, NULL},		//0x0B
	{"INC C", NULL, C, NULL},			//0x0C
	{"INC D", NULL, D, NULL},			//0x0D
	{"LD C, n", NULL, C, NULL},		//0x0E
	{"RRC A", NULL, A, NULL},			//0x0F
	{"STOP", NULL, NULL, NULL},			//0x10
	{"LD DE, nn", NULL, DE, NULL},	//0x11
	{"LD DE, A", NULL, DE, A},		//0x12
	{"INC DE", NULL, DE, NULL},		//0x13
	{"INC D", NULL, D, NULL},			//0x14
	{"DEC D", NULL, D, NULL},			//0x15
	{"LD D, n", NULL, D, NULL},		//0x16
	{"RL A", NULL, A, NULL},			//0x17
	{"JR n", NULL, NULL, NULL},			//0x18
	{"ADD HL, DE", NULL, HL, DE},	//0x19
	{"LD A, DE", NULL, A, DE},		//0x1A
	{"DEC DE", NULL, DE, NULL},		//0x1B
	{"INC E", NULL, E, NULL},			//0x1C
	{"DEC E", NULL, E, NULL},			//0x1D
	{"LD E, n", NULL, E, NULL},		//0x1E
	{"RR A", NULL, A, NULL},			//0x1F
	{"JR NZ, n", NULL, NZ, NULL},		//0x20		//NZ is check for Not zero
	{"LD HL, nn", NULL, HL, NULL},	//0x21
	{"LDI HL, A", NULL, HL, A},		//0x22
	{"INC HL", NULL, HL, NULL},		//0x23
	{"INC H", NULL, H, NULL},			//0x24
	{"DEC H", NULL, H, NULL},			//0x25
	{"LD H, n", NULL, H, NULL},		//0x26
	{"DAA", NULL, NULL, NULL},			//0x27
	{"JR Z, n", NULL, Z, NULL},		//0x28		//Z is check for zero
	{"ADD HL, HL", NULL, HL, HL},	//0x29
	{"LDI A, HL", NULL, A, HL},		//0x2A
	{"DEC HL", NULL, HL, NULL},		//0x2B
	{"INC L", NULL, L, NULL},			//0x2C
	{"DEC L", NULL, L, NULL},			//0x2D
	{"LD L, n", NULL, L, NULL},		//0x2E
	{"CPL", NULL, NULL, NULL},			//0x2F
	{"JR NC, n", NULL, NC, NULL},		//0x30		//NC is check for no carry
	{"LD SP, nn", NULL, SP, NULL},	//0x31
	{"LDD HL, A", NULL, HL, A},		//0x32
	{"INC SP", NULL, SP, NULL},		//0x33
	{"INC HL", NULL, HL, NULL},		//0x34
	{"DEC HL", NULL, HL, NULL},		//0x35
	{"LD HL, n", NULL, HL, NULL},		//0x36
	{"SCF", NULL, NULL, NULL},			//0x37
	{"JR C, n", NULL, C, NULL},		//0x38		//C is check for carry
	{"ADD HL, SP", NULL, HL, SP},	//0x39
	{"LDD A, HL", NULL, A, HL},		//0x3A
	{"DEC SP", NULL, SP, NULL},		//0x3B
	{"INC A", NULL, A, NULL},			//0x3C
	{"DEC A", NULL, A, NULL},			//0x3D
	{"LD A, n", NULL, A, NULL},		//0x3E
	{"CCF", NULL, NULL, NULL},			//0x3F
	{"LD B, B", NULL, B, B},		//0x40
	{"LD B, C", NULL, B, C},		//0x41
	{"LD B, D", NULL, B, D},		//0x42
	{"LD B, E", NULL, B, E},		//0x43
	{"LD B, H", NULL, B, H},		//0x44
	{"LD B, L", NULL, B, L},		//0x45
	{"LD B, HL", NULL, B, HL},		//0x46
	{"LD B, A", NULL, B, A},		//0x47
	{"LD C, B", NULL, C, B},		//0x48
	{"LD C, C", NULL, C, C},		//0x49
	{"LD C, D", NULL, C, D},		//0x4A
	{"LD C, E", NULL, C, E},		//0x4B
	{"LD C, H", NULL, C, H},		//0x4C
	{"LD C, L", NULL, C, L},		//0x4D
	{"LD C, HL", NULL, C, HL},		//0x4E
	{"LD C, A", NULL, C, A},		//0x4F
	{"LD D, B", NULL, D, B},		//0x50
	{"LD D, C", NULL, D, C},		//0x51
	{"LD D, D", NULL, D, D},		//0x52
	{"LD D, E", NULL, D, E},		//0x53
	{"LD D, H", NULL, D, H},		//0x54
	{"LD D, L", NULL, D, L},		//0x55
	{"LD D, HL", NULL, D, HL},		//0x56
	{"LD D, A", NULL, D, A},		//0x57
	{"LD E, B", NULL, E, B},		//0x58
	{"LD E, C", NULL, E, C},		//0x59
	{"LD E, D", NULL, E, D},		//0x5A
	{"LD E, E", NULL, E, E},		//0x5B
	{"LD E, H", NULL, E, H},		//0x5C
	{"LD E, L", NULL, E, L},		//0x5D
	{"LD E, HL", NULL, E, HL},		//0x5E
	{"LD E, A", NULL, E, A},		//0x5F
	{"LD H, B", NULL, H, B},		//0x60
	{"LD H, C", NULL, H, C},		//0x61
	{"LD H, D", NULL, H, D},		//0x62
	{"LD H, E", NULL, H, E},		//0x63
	{"LD H, H", NULL, H, H},		//0x64
	{"LD H, L", NULL, H, L},		//0x65
	{"LD H, HL", NULL, H, HL},		//0x66
	{"LD H, A", NULL, H, A},		//0x67
	{"LD L, B", NULL, L, B},		//0x68
	{"LD L, C", NULL, L, C},		//0x69
	{"LD L, D", NULL, L, D},		//0x6A
	{"LD L, E", NULL, L, E},		//0x6B
	{"LD L, H", NULL, L, H},		//0x6C
	{"LD L, L", NULL, L, L},		//0x6D
	{"LD L, HL", NULL, L, HL},		//0x6E
	{"LD L, A", NULL, L, A},		//0x6F
	{"LD HL, B", NULL, HL, B},		//0x70
	{"LD HL, C", NULL, HL, C},		//0x71
	{"LD HL, D", NULL, HL, D},		//0x72
	{"LD HL, E", NULL, HL, E},		//0x73
	{"LD HL, H", NULL, HL, H},		//0x74
	{"LD HL, L", NULL, HL, L},		//0x75
	{"HALT", NULL, NULL, NULL},		//0x76
	{"LD HL, A", NULL, HL, A},		//0x77
	{"LD A, B", NULL, A, B},		//0x78
	{"LD A, C", NULL, A, C},		//0x79
	{"LD A, D", NULL, A, D},		//0x7A
	{"LD A, E", NULL, A, E},		//0x7B
	{"LD A, H", NULL, A, H},		//0x7C
	{"LD A, L", NULL, A, L},		//0x7D
	{"LD A, HL", NULL, A, HL},		//0x7E
	{"LD A, A", NULL, A, A},		//0x7F
	{"ADD A, B", NULL, A, B},		//0x80
	{"ADD A, C", NULL, A, C},		//0x81
	{"ADD A, D", NULL, A, D},		//0x82
	{"ADD A, E", NULL, A, E},		//0x83
	{"ADD A, H", NULL, A, H},		//0x84
	{"ADD A, L", NULL, A, L },		//0x85
	{"ADD A, HL", NULL, A, HL},		//0x86
	{"ADD A, A", NULL, A, A},		//0x87
	{"ADC A, B", NULL, A, B},		//0x88
	{"ADC A, C", NULL, A, C},		//0x89
	{"ADC A, D", NULL, A, D},		//0x8A
	{"ADC A, E", NULL, A, E},		//0x8B
	{"ADC A, H", NULL, A, H},		//0x8C
	{"ADC A, L", NULL, A, L},		//0x8D
	{"ADC A, HL", NULL, A, HL},		//0x8E
	{"ADC A, A", NULL, A, A},		//0x8F
	{"SUB A, B", NULL, A, B},		//0x90
	{"SUB A, C", NULL, A, C},		//0x91
	{"SUB A, D", NULL, A, D},		//0x92
	{"SUB A, E", NULL, A, E},		//0x93
	{"SUB A, H", NULL, A, H},		//0x94
	{"SUB A, L", NULL, A, L},		//0x95
	{"SUB A, HL", NULL, A, HL},		//0x96
	{"SUB A, A", NULL, A, A},		//0x97
	{"SBC A, B", NULL, A, B},		//0x98
	{"SBC A, C", NULL, A, C},		//0x99
	{"SBC A, D", NULL, A, D},		//0x9A
	{"SBC A, E", NULL, A, E},		//0x9B
	{"SBC A, H", NULL, A, H},		//0x9C
	{"SBC A, L", NULL, A, L },		//0x9D
	{"SBC A, HL", NULL, A, HL},		//0x9E
	{"SBC A, A", NULL, A, A},		//0x9F
	{"AND B", NULL, B, NULL},		//0xA0
	{"AND C", NULL, C, NULL},		//0xA1
	{"AND D", NULL, D, NULL},		//0xA2
	{"AND E", NULL, E, NULL},		//0xA3
	{"AND H", NULL, H, NULL},		//0xA4
	{"AND L", NULL, L, NULL},		//0xA5
	{"AND HL", NULL, HL, NULL},		//0xA6
	{"AND A", NULL, A, NULL},		//0xA7
	{"XOR B", NULL, B, NULL},		//0xA8
	{"XOR C", NULL, C, NULL},		//0xA9
	{"XOR D", NULL, D, NULL},		//0xAA
	{"XOR E", NULL, E, NULL},		//0xAB
	{"XOR H", NULL, H, NULL},		//0xAC
	{"XOR L", NULL, L, NULL },		//0xAD
	{"XOR HL", NULL, HL, NULL},		//0xAE
	{"XOR A", NULL, A, NULL},		//0xAF
	{"OR B", NULL, B, NULL},		//0xB0
	{"OR C", NULL, C, NULL},		//0xB1
	{"OR D", NULL, D, NULL},		//0xB2
	{"OR E", NULL, E, NULL},		//0xB3
	{"OR H", NULL, H, NULL},		//0xB4
	{"OR L", NULL, L, NULL},		//0xB5
	{"OR HL", NULL, HL, NULL},		//0xB6
	{"OR A", NULL, A, NULL},		//0xB7
	{"CP B", NULL, B, NULL},		//0xB8
	{"CP C", NULL, C, NULL},		//0xB9
	{"CP D", NULL, D, NULL},		//0xBA
	{"CP E", NULL, E, NULL},		//0xBB
	{"CP H", NULL, H, NULL},		//0xBC
	{"CP L", NULL, L, NULL},		//0xBD
	{"CP HL", NULL, HL, NULL},		//0xBE
	{"CP A", NULL, A, NULL},		//0xBF
	{"RET NZ", NULL, NZ, NULL},		//0xC0		//NZ means check for not zero
	{"POP BC", NULL, BC, NULL},		//0xC1
	{"JP NZ, nn", NULL, NZ, NULL},	//0xC2
	{"JP nn", NULL, NULL, NULL },	//0xC3
	{"CALL NZ, nn", NULL, NZ, NULL},//0xC4
	{"PUSH BC", NULL, BC, NULL},	//0xC5
	{"ADD A, n", NULL, A, NULL},	//0xC6
	{"RST 0", NULL, "0", NULL},		//0xC7
	{"RET Z", NULL, Z, NULL },		//0xC8
	{"RET", NULL, NULL, NULL },		//0xC9
	{"JP Z, nn", NULL, Z, NULL},	//0xCA
	{"Ext Ops (CB)", NULL, NULL, NULL},	//0xCB
	{"CALL Z, nn", NULL, Z, NULL},	//0xCC
	{"CALL nn", NULL, NULL, NULL},		//0xCD
	{"ADC A, n", NULL, A, NULL},		//0xCE
	{"RST 8", NULL, "8", NULL},		//0xCF
	{"RFT NC", NULL, NC, NULL},		//0xD0
	{"POP DE", NULL, DE, NULL},		//0xD1
	{"JP NC, nn", NULL, NC, NULL},	//0xD2
	{"XX", NULL, NULL, NULL},			//0xD3
	{"CALL NC, nn", NULL, NC, NULL},	//0xD4
	{"PUSH DE", NULL, DE, NULL},		//0xD5
	{"SUB A, n", NULL, A, NULL},		//0xD6
	{"RST 10", NULL, "10", NULL},		//0xD7
	{"RET C", NULL, C, NULL},			//0xD8
	{"RETI", NULL, NULL, NULL},			//0xD9
	{"JP C, nn", NULL, C, NULL},		//0xDA
	{"XX", NULL, NULL, NULL},			//0xDB
	{"JP C, nn", NULL, C, NULL},		//0xDC
	{"XX", NULL, NULL, NULL},			//0xDD
	{"SBC A, n", NULL, A, NULL},		//0xDE
	{"RST 18", NULL, "18", NULL},		//0xDF
	{"LDH n, A", NULL, NULL, A},		//0xE0
	{"POP HL", NULL, HL, NULL},		//0xE1
	{"LDH C, A", NULL, C, A},		//0xE2
	{"XX", NULL, NULL, NULL},			//0xE3
	{"XX", NULL, NULL, NULL},			//0xE4
	{"PUSH HL", NULL, HL, NULL},		//0xE5
	{"AND n", NULL, NULL, NULL},		//0xE6
	{"RST 20", NULL, "20", NULL},		//0xE7
	{"ADD SP, d", NULL, SP, NULL},	//0xE8
	{"JP HL", NULL, HL, NULL},		//0xE9
	{"LD nn, A", NULL, NULL, A},		//0xEA
	{"XX", NULL, NULL, NULL},			//0xEB
	{"XX", NULL, NULL, NULL},			//0xEC
	{"XX", NULL, NULL, NULL},			//0xED
	{"XOR n", NULL, NULL, NULL},		//0xEE
	{"RST 28", NULL, "28", NULL},		//0xEF
	{"LDH A, n", NULL, A, NULL},		//0xF0
	{"POP AF", NULL, AF, NULL},		//0xF1
	{"XX", NULL, NULL, NULL},			//0xF2
	{"DI", NULL, NULL, NULL},			//0xF3
	{"XX", NULL, NULL, NULL},			//0xF4
	{"PUSH AF", NULL, AF, NULL},		//0xF5
	{"OR n", NULL, NULL, NULL},			//0xF6
	{"RST 30", NULL, "30", NULL},		//0xF7
	{"LDHL SP, d", NULL, SP, NULL},	//0xF8
	{"LD SP, HL", NULL, SP, HL},	//0xF9
	{"LD A, nn", NULL, A, NULL},		//0xFA
	{"EI", NULL, NULL, NULL},			//0xFB
	{"XX", NULL, NULL, NULL},			//0xFC
	{"XX", NULL, NULL, NULL},			//0xFD
	{"CP n", NULL, NULL, NULL},			//0xFE
	{"RST 38", NULL, "28", NULL},		//0xFF
};