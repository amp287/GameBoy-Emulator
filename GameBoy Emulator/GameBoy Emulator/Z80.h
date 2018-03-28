#include <stdlib.h>

#define ZERO_FLAG 0x80
#define SUBTRACT_FLAG 0x40
#define HALF_CARRY_FLAG 0x20
#define CARRY_FLAG 0x10

//cf is carry flag
enum Regval { A, B, C, D, E, H, L, AF, BC, DE, HL, Z, Cf, NC, NZ, SP, READ_8, READ_16};

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
int check_interrupts();

//Function Pointer
typedef void (*OPCODE_OPERATION)(unsigned short arg1, unsigned short arg2);

typedef struct {
	char *disassembly;
	OPCODE_OPERATION execute;
	void *r1;
	void *r2;
}INSTR;

void LD_nn_n(unsigned short r1, unsigned short immediate);
void LD_r1_r2(unsigned short r1, unsigned short r2);
void LD_A_n(unsigned short a, unsigned short r2);
void LD_A_nn(unsigned short a, unsigned short nn);
void LD_A_imm(unsigned short a, unsigned short immediate);
void LD_n_A(unsigned short n, unsigned short a);
void LD_nn_A(unsigned short nn, unsigned short A);
void LD_A_C(unsigned short A, unsigned short C);
void LD_C_A(unsigned short C, unsigned short A);
void LD_A_HLD(unsigned short A, unsigned short HL);
void LD_HLD_A(unsigned short HL, unsigned short A);
void LD_A_HLI(unsigned short A, unsigned short HL);
void LD_HLI_A(unsigned short hl, unsigned short a);
void LDH_n_A(unsigned short n, unsigned short A);
void LDH_A_n(unsigned short A, unsigned short n);

//16-Bit Loads
void LD_n_nn(unsigned short n, unsigned short nn);
void LD_SP_HL(unsigned short sp, unsigned short hl);
void LDHL_SP_n(unsigned short sp, unsigned short n);
void LD_nn_SP(unsigned short nn, unsigned short sp);
void PUSH_nn(unsigned short nn, unsigned short NA);
void POP_nn(unsigned short nn, unsigned short NA);

// 8 bit ALU
//when using check whether immediate can be one of the register numbers
void ADD_A_n(unsigned short a, unsigned short n);

void ADC_A_n(unsigned short a, unsigned short n);
void SUB_n(unsigned short n, unsigned short NA);
void SBC_A_n(unsigned short a, unsigned short n);
void AND_n(unsigned short n, unsigned short NA);
void OR_n(unsigned short n, unsigned short NA);
void XOR_n(unsigned short n, unsigned short NA);
void CP_n(unsigned short n, unsigned short NA);

void INC_n(unsigned short n, unsigned short NA);
void DEC_n(unsigned short n, unsigned short NA);

//16-Bit Arithmetic
void ADD_HL_n(unsigned short HL, unsigned short n);
void ADD_SP_n(unsigned short sp, unsigned short n);
void INC_nn(unsigned short nn, unsigned short NA);
void DEC_nn(unsigned short nn, unsigned short NA);

//Miscellaneous
void SWAP_n(unsigned short n, unsigned short NA);
void DAA(unsigned short NA_1, unsigned short NA_2);
void CPL(unsigned short NA_1, unsigned short NA_2);
void CCF(unsigned short NA_1, unsigned short NA_2);
void SCF(unsigned short NA_1, unsigned short NA_2);
void NOP(unsigned short NA_1, unsigned short NA_2);
void HALT(unsigned short NA_1, unsigned short NA_2);
void STOP(unsigned short NA_1, unsigned short NA_2);
void DI(unsigned short NA_1, unsigned short NA_2);
void EI(unsigned short NA_1, unsigned short NA_2);

//Rotates & Shifts


void RLCA(unsigned short A, unsigned short NA);
void RLA(unsigned short A, unsigned short NA);
void RRCA(unsigned short A, unsigned short NA);
void RRA(unsigned short A, unsigned short NA);
void RLC_n(unsigned short n, unsigned short NA);
void RL_n(unsigned short n, unsigned short NA);
void RRC_n(unsigned short n, unsigned short NA);
void RR_n(unsigned short n, unsigned short NA);
void SLA_n(unsigned short n, unsigned short NA);
void SRA_n(unsigned short n, unsigned short NA);
void SRL_n(unsigned short n, unsigned short NA);

//Bit Opcodes
void BIT_b_r(unsigned short b, unsigned short r);
void SET_b_r(unsigned short b, unsigned short r);
void RES_b_r(unsigned short b, unsigned short r);

//Jumps
void JP_nn(unsigned short nn);
void JP_cc_nn(unsigned short cc, unsigned short nn);
void JP_HL(unsigned short NA_1, unsigned short NA_2);
void JR_n(unsigned short n, unsigned short NA_1);
void JR_cc_n(unsigned short cc, unsigned short n);

//Calls
void CALL_nn(unsigned short nn, unsigned short NA);
void CALL_cc_nn(unsigned short cc, unsigned short nn);

//Restarts 

//n = 0x0, 0x8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 
void RST_n(unsigned short n, unsigned short NA);

//Returns 
void RET(unsigned short NA_1, unsigned short NA_2);
void RET_cc(unsigned short cc, unsigned short NA);
void RETI(unsigned short NA_1, unsigned short NA_2);


static INSTR Opcodes[] = {
	{"NOP", NULL, NULL, NULL},			//0x00
	{"LD BC, nn", LD_n_nn, BC, NULL},	//0x01
	{"LD BC, A", NULL, BC, A},		//0x02
	{"INC BC", NULL, BC, NULL},		//0x03
	{"INC B", NULL, B, NULL},			//0x04
	{"DEC B", NULL, B, NULL},			//0x05
	{"LD B, n", LD_nn_n, B, READ_8},		//0x06
	{"RLC A", NULL, A, NULL},			//0x07
	{"LD nn, SP", NULL, NULL, SP},	//0x08
	{"ADD HL, BC", NULL, HL, BC},	//0x09
	{"LD A, BC", LD_A_n, A, BC},		//0x0A
	{"DEC BC", NULL, BC, NULL},		//0x0B
	{"INC C", NULL, C, NULL},			//0x0C
	{"INC D", NULL, D, NULL},			//0x0D
	{"LD C, n", LD_nn_n, C, READ_8},		//0x0E
	{"RRC A", NULL, A, NULL},			//0x0F
	{"STOP", NULL, NULL, NULL},			//0x10
	{"LD DE, nn", NULL, DE, NULL},	//0x11
	{"LD DE, A", NULL, DE, A},		//0x12
	{"INC DE", NULL, DE, NULL},		//0x13
	{"INC D", NULL, D, NULL},			//0x14
	{"DEC D", NULL, D, NULL},			//0x15
	{"LD D, n", LD_nn_n, D, READ_8},		//0x16
	{"RL A", NULL, A, NULL},			//0x17
	{"JR n", NULL, NULL, NULL},			//0x18
	{"ADD HL, DE", NULL, HL, DE},	//0x19
	{"LD A, DE", LD_A_n, A, DE},		//0x1A
	{"DEC DE", NULL, DE, NULL},		//0x1B
	{"INC E", NULL, E, NULL},			//0x1C
	{"DEC E", NULL, E, NULL},			//0x1D
	{"LD E, n", LD_nn_n, E, READ_8},		//0x1E
	{"RR A", NULL, A, NULL},			//0x1F
	{"JR NZ, n", NULL, NZ, NULL},		//0x20		//NZ is check for Not zero
	{"LD HL, nn", NULL, HL, NULL},	//0x21
	{"LDI HL, A", NULL, HL, A},		//0x22
	{"INC HL", NULL, HL, NULL},		//0x23
	{"INC H", NULL, H, NULL},			//0x24
	{"DEC H", NULL, H, NULL},			//0x25
	{"LD H, n", LD_nn_n, H, READ_8},		//0x26
	{"DAA", NULL, NULL, NULL},			//0x27
	{"JR Z, n", NULL, Z, NULL},		//0x28		//Z is check for zero
	{"ADD HL, HL", NULL, HL, HL},	//0x29
	{"LDI A, HL", NULL, A, HL},		//0x2A
	{"DEC HL", NULL, HL, NULL},		//0x2B
	{"INC L", NULL, L, NULL},			//0x2C
	{"DEC L", NULL, L, NULL},			//0x2D
	{"LD L, n", LD_nn_n, L, READ_8},		//0x2E
	{"CPL", NULL, NULL, NULL},			//0x2F
	{"JR NC, n", NULL, NC, NULL},		//0x30		//NC is check for no carry
	{"LD SP, nn", NULL, SP, NULL},	//0x31
	{"LDD HL, A", NULL, HL, A},		//0x32
	{"INC SP", NULL, SP, NULL},		//0x33
	{"INC HL", NULL, HL, NULL},		//0x34
	{"DEC HL", NULL, HL, NULL},		//0x35
	{"LD HL, n", LD_r1_r2, HL, READ_8},		//0x36 CHECK IF READ 8
	{"SCF", NULL, NULL, NULL},			//0x37
	{"JR C, n", NULL, C, NULL},		//0x38		//C is check for carry
	{"ADD HL, SP", NULL, HL, SP},	//0x39
	{"LDD A, HL", NULL, A, HL},		//0x3A
	{"DEC SP", NULL, SP, NULL},		//0x3B
	{"INC A", NULL, A, NULL},			//0x3C
	{"DEC A", NULL, A, NULL},			//0x3D
	{"LD A, n", LD_A_imm, A, READ_8},		//0x3E
	{"CCF", NULL, NULL, NULL},			//0x3F
	{"LD B, B", LD_r1_r2, B, B},		//0x40
	{"LD B, C", LD_r1_r2, B, C},		//0x41
	{"LD B, D", LD_r1_r2, B, D},		//0x42
	{"LD B, E", LD_r1_r2, B, E},		//0x43
	{"LD B, H", LD_r1_r2, B, H},		//0x44
	{"LD B, L", LD_r1_r2, B, L},		//0x45
	{"LD B, HL", LD_r1_r2, B, HL},		//0x46
	{"LD B, A", NULL, B, A},		//0x47
	{"LD C, B", LD_r1_r2, C, B},		//0x48
	{"LD C, C", LD_r1_r2, C, C},		//0x49
	{"LD C, D", LD_r1_r2, C, D},		//0x4A
	{"LD C, E", LD_r1_r2, C, E},		//0x4B
	{"LD C, H", LD_r1_r2, C, H},		//0x4C
	{"LD C, L", LD_r1_r2, C, L},		//0x4D
	{"LD C, HL", LD_r1_r2, C, HL},		//0x4E
	{"LD C, A", NULL, C, A},		//0x4F
	{"LD D, B", LD_r1_r2, D, B},		//0x50
	{"LD D, C", LD_r1_r2, D, C},		//0x51
	{"LD D, D", LD_r1_r2, D, D},		//0x52
	{"LD D, E", LD_r1_r2, D, E},		//0x53
	{"LD D, H", LD_r1_r2, D, H},		//0x54
	{"LD D, L", LD_r1_r2, D, L},		//0x55
	{"LD D, HL", LD_r1_r2, D, HL},		//0x56
	{"LD D, A", NULL, D, A},		//0x57
	{"LD E, B", LD_r1_r2, E, B},		//0x58
	{"LD E, C", LD_r1_r2, E, C},		//0x59
	{"LD E, D", LD_r1_r2, E, D},		//0x5A
	{"LD E, E", LD_r1_r2, E, E},		//0x5B
	{"LD E, H", LD_r1_r2, E, H},		//0x5C
	{"LD E, L", LD_r1_r2, E, L},		//0x5D
	{"LD E, HL", LD_r1_r2, E, HL},		//0x5E
	{"LD E, A", NULL, E, A},		//0x5F
	{"LD H, B", LD_r1_r2, H, B},		//0x60
	{"LD H, C", LD_r1_r2, H, C},		//0x61
	{"LD H, D", LD_r1_r2, H, D},		//0x62
	{"LD H, E", LD_r1_r2, H, E},		//0x63
	{"LD H, H", LD_r1_r2, H, H},		//0x64
	{"LD H, L", LD_r1_r2, H, L},		//0x65
	{"LD H, HL", LD_r1_r2, H, HL},		//0x66
	{"LD H, A", NULL, H, A},		//0x67
	{"LD L, B", LD_r1_r2, L, B},		//0x68
	{"LD L, C", LD_r1_r2, L, C},		//0x69
	{"LD L, D", LD_r1_r2, L, D},		//0x6A
	{"LD L, E", LD_r1_r2, L, E},		//0x6B
	{"LD L, H", LD_r1_r2, L, H},		//0x6C
	{"LD L, L", LD_r1_r2, L, L},		//0x6D
	{"LD L, HL", LD_r1_r2, L, HL},		//0x6E
	{"LD L, A", NULL, L, A},		//0x6F
	{"LD HL, B", LD_r1_r2, HL, B},		//0x70
	{"LD HL, C", LD_r1_r2, HL, C},		//0x71
	{"LD HL, D", LD_r1_r2, HL, D},		//0x72
	{"LD HL, E", LD_r1_r2, HL, E},		//0x73
	{"LD HL, H", LD_r1_r2, HL, H},		//0x74
	{"LD HL, L", LD_r1_r2, HL, L},		//0x75
	{"HALT", NULL, NULL, NULL},		//0x76
	{"LD HL, A", NULL, HL, A},		//0x77
	{"LD A, B", LD_r1_r2, A, B},		//0x78
	{"LD A, C", LD_r1_r2, A, C},		//0x79
	{"LD A, D", LD_r1_r2, A, D},		//0x7A
	{"LD A, E", LD_r1_r2, A, E},		//0x7B
	{"LD A, H", LD_r1_r2, A, H},		//0x7C
	{"LD A, L", LD_r1_r2, A, L},		//0x7D
	{"LD A, HL", LD_r1_r2, A, HL},		//0x7E
	{"LD A, A", LD_r1_r2, A, A},		//0x7F
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
	{"LD A, nn", LD_A_nn, A, READ_16},		//0xFA
	{"EI", NULL, NULL, NULL},			//0xFB
	{"XX", NULL, NULL, NULL},			//0xFC
	{"XX", NULL, NULL, NULL},			//0xFD
	{"CP n", NULL, NULL, NULL},			//0xFE
	{"RST 38", NULL, "28", NULL},		//0xFF
};

static INSTR OpcodesCD[] = {
	{ "RLC B", NULL, B, NULL },           //CDx00
	{ "RLC C", NULL, C, NULL },           //CDx01
	{ "RLC D", NULL, D, NULL },           //CDx02
	{ "RLC E", NULL, E, NULL },           //CDx03
	{ "RLC H", NULL, H, NULL },           //CDx04
	{ "RLC L", NULL, L, NULL },           //CDx05
	{ "RLC HL", NULL, HL, NULL },         //CDx06
	{ "RLC A", NULL, A, NULL },           //CDx07
	{ "RRC B", NULL, B, NULL },           //CDx08
	{ "RRC C", NULL, C, NULL },           //CDx09
	{ "RRC D", NULL, D, NULL },           //CDx0A
	{ "RRC E", NULL, E, NULL },           //CDx0B
	{ "RRC H", NULL, H, NULL },           //CDx0C
	{ "RRC L", NULL, L, NULL },           //CDx0D
	{ "RRC HL", NULL, HL, NULL },         //CDx0E
	{ "RRC A", NULL, A, NULL },           //CDx0F

	{ "RL B", NULL, B, NULL },            //CDx10
	{ "RL C", NULL, C, NULL },            //CDx11
	{ "RL D", NULL, D, NULL },            //CDx12
	{ "RL E", NULL, E, NULL },            //CDx13
	{ "RL H", NULL, H, NULL },            //CDx14
	{ "RL L", NULL, L, NULL },            //CDx15
	{ "RL HL", NULL, HL, NULL },          //CDx16
	{ "RL A", NULL, A, NULL },            //CDx17
	{ "RR B", NULL, B, NULL },            //CDx18
	{ "RR C", NULL, C, NULL },            //CDx19
	{ "RR D", NULL, D, NULL },            //CDx1A
	{ "RR E", NULL, E, NULL },            //CDx1B
	{ "RR H", NULL, H, NULL },            //CDx1C
	{ "RR L", NULL, L, NULL },            //CDx1D
	{ "RR HL", NULL, HL, NULL },          //CDx1E
	{ "RR A", NULL, A, NULL },            //CDx1F

	{ "SLA B", NULL, B, NULL },           //CDx20
	{ "SLA C", NULL, C, NULL },           //CDx21
	{ "SLA D", NULL, D, NULL },           //CDx22
	{ "SLA E", NULL, E, NULL },           //CDx23
	{ "SLA H", NULL, H, NULL },           //CDx24
	{ "SLA L", NULL, L, NULL },           //CDx25
	{ "SLA HL", NULL, HL, NULL },         //CDx26
	{ "SLA A", NULL, A, NULL },           //CDx27
	{ "SRA B", NULL, B, NULL },           //CDx28
	{ "SRA C", NULL, C, NULL },           //CDx29
	{ "SRA D", NULL, D, NULL },           //CDx2A
	{ "SRA E", NULL, E, NULL },           //CDx2B
	{ "SRA H", NULL, H, NULL },           //CDx2C
	{ "SRA L", NULL, L, NULL },           //CDx2D
	{ "SRA HL", NULL, HL, NULL },         //CDx2E
	{ "SRA A", NULL, A, NULL },           //CDx2F

	{ "SWAP B", NULL, B, NULL },          //CDx30
	{ "SWAP C", NULL, C, NULL },          //CDx31
	{ "SWAP D", NULL, D, NULL },          //CDx32
	{ "SWAP E", NULL, E, NULL },          //CDx33
	{ "SWAP H", NULL, H, NULL },          //CDx34
	{ "SWAP L", NULL, L, NULL },          //CDx35
	{ "SWAP HL", NULL, HL, NULL },        //CDx36
	{ "SWAP A", NULL, A, NULL },          //CDx37
	{ "SRL B", NULL, B, NULL },           //CDx38
	{ "SRL C", NULL, C, NULL },           //CDx39
	{ "SRL D", NULL, D, NULL },           //CDx3A
	{ "SRL E", NULL, E, NULL },           //CDx3B
	{ "SRL H", NULL, H, NULL },           //CDx3C
	{ "SRL L", NULL, L, NULL },           //CDx3D
	{ "SRL HL", NULL, HL, NULL },         //CDx3E
	{ "SRL A", NULL, A, NULL },           //CDx3F

	{ "BIT 0 B", NULL, 0, B },            //CDx40
	{ "BIT 0 C", NULL, 0, C },            //CDx41
	{ "BIT 0 D", NULL, 0, D },            //CDx42
	{ "BIT 0 E", NULL, 0, E },            //CDx43
	{ "BIT 0 H", NULL, 0, H },            //CDx44
	{ "BIT 0 L", NULL, 0, L },            //CDx45
	{ "BIT 0 HL", NULL, 0, HL },          //CDx46
	{ "BIT 0 A", NULL, 0, A },            //CDx47
	{ "BIT 1 B", NULL, 1, B },            //CDx48
	{ "BIT 1 C", NULL, 1, C },            //CDx49
	{ "BIT 1 D", NULL, 1, D },            //CDx4A
	{ "BIT 1 E", NULL, 1, E },            //CDx4B
	{ "BIT 1 H", NULL, 1, H },            //CDx4C
	{ "BIT 1 L", NULL, 1, L },            //CDx4D
	{ "BIT 1 HL", NULL, 1, HL },          //CDx4E
	{ "BIT 1 A", NULL, 1, A },            //CDx4F

	{ "BIT 2 B", NULL, 2, B },            //CDx50
	{ "BIT 2 C", NULL, 2, C },            //CDx51
	{ "BIT 2 D", NULL, 2, D },            //CDx52
	{ "BIT 2 E", NULL, 2, E },            //CDx53
	{ "BIT 2 H", NULL, 2, H },            //CDx54
	{ "BIT 2 L", NULL, 2, L },            //CDx55
	{ "BIT 2 HL", NULL, 2, HL },          //CDx56
	{ "BIT 2 A", NULL, 2, A },            //CDx57
	{ "BIT 3 B", NULL, 3, B },            //CDx58
	{ "BIT 3 C", NULL, 3, C },            //CDx59
	{ "BIT 3 D", NULL, 3, D },            //CDx5A
	{ "BIT 3 E", NULL, 3, E },            //CDx5B
	{ "BIT 3 H", NULL, 3, H },            //CDx5C
	{ "BIT 3 L", NULL, 3, L },            //CDx5D
	{ "BIT 3 HL", NULL, 3, HL },          //CDx5E
	{ "BIT 3 A", NULL, 3, A },            //CDx5F

	{ "BIT 4 B", NULL, 4, B },            //CDx60
	{ "BIT 4 C", NULL, 4, C },            //CDx61
	{ "BIT 4 D", NULL, 4, D },            //CDx62
	{ "BIT 4 E", NULL, 4, E },            //CDx63
	{ "BIT 4 H", NULL, 4, H },            //CDx64
	{ "BIT 4 L", NULL, 4, L },            //CDx65
	{ "BIT 4 HL", NULL, 4, HL },          //CDx66
	{ "BIT 4 A", NULL, 4, A },            //CDx67
	{ "BIT 5 B", NULL, 5, B },            //CDx68
	{ "BIT 5 C", NULL, 5, C },            //CDx69
	{ "BIT 5 D", NULL, 5, D },            //CDx6A
	{ "BIT 5 E", NULL, 5, E },            //CDx6B
	{ "BIT 5 H", NULL, 5, H },            //CDx6C
	{ "BIT 5 L", NULL, 5, L },            //CDx6D
	{ "BIT 5 HL", NULL, 5, HL },          //CDx6E
	{ "BIT 5 A", NULL, 5, A },            //CDx6F

	{ "BIT 6 B", NULL, 6, B },            //CDx70
	{ "BIT 6 C", NULL, 6, C },            //CDx71
	{ "BIT 6 D", NULL, 6, D },            //CDx72
	{ "BIT 6 E", NULL, 6, E },            //CDx73
	{ "BIT 6 H", NULL, 6, H },            //CDx74
	{ "BIT 6 L", NULL, 6, L },            //CDx75
	{ "BIT 6 HL", NULL, 6, HL },          //CDx76
	{ "BIT 6 A", NULL, 6, A },            //CDx77
	{ "BIT 7 B", NULL, 7, B },            //CDx78
	{ "BIT 7 C", NULL, 7, C },            //CDx79
	{ "BIT 7 D", NULL, 7, D },            //CDx7A
	{ "BIT 7 E", NULL, 7, E },            //CDx7B
	{ "BIT 7 H", NULL, 7, H },            //CDx7C
	{ "BIT 7 L", NULL, 7, L },            //CDx7D
	{ "BIT 7 HL", NULL, 7, HL },          //CDx7E
	{ "BIT 7 A", NULL, 7, A },            //CDx7F

	{ "RES 0 B", NULL, 0, B },            //CDx80
	{ "RES 0 C", NULL, 0, C },            //CDx81
	{ "RES 0 D", NULL, 0, D },            //CDx82
	{ "RES 0 E", NULL, 0, E },            //CDx83
	{ "RES 0 H", NULL, 0, H },            //CDx84
	{ "RES 0 L", NULL, 0, L },            //CDx85
	{ "RES 0 HL", NULL, 0, HL },          //CDx86
	{ "RES 0 A", NULL, 0, A },            //CDx87
	{ "RES 1 B", NULL, 1, B },            //CDx88
	{ "RES 1 C", NULL, 1, C },            //CDx89
	{ "RES 1 D", NULL, 1, D },            //CDx8A
	{ "RES 1 E", NULL, 1, E },            //CDx8B
	{ "RES 1 H", NULL, 1, H },            //CDx8C
	{ "RES 1 L", NULL, 1, L },            //CDx8D
	{ "RES 1 HL", NULL, 1, HL },          //CDx8E
	{ "RES 1 A", NULL, 1, A },            //CDx8F

	{ "RES 2 B", NULL, 2, B },            //CDx90
	{ "RES 2 C", NULL, 2, C },            //CDx91
	{ "RES 2 D", NULL, 2, D },            //CDx92
	{ "RES 2 E", NULL, 2, E },            //CDx93
	{ "RES 2 H", NULL, 2, H },            //CDx94
	{ "RES 2 L", NULL, 2, L },            //CDx95
	{ "RES 2 HL", NULL, 2, HL },          //CDx96
	{ "RES 2 A", NULL, 2, A },            //CDx97
	{ "RES 3 B", NULL, 3, B },            //CDx98
	{ "RES 3 C", NULL, 3, C },            //CDx99
	{ "RES 3 D", NULL, 3, D },            //CDx9A
	{ "RES 3 E", NULL, 3, E },            //CDx9B
	{ "RES 3 H", NULL, 3, H },            //CDx9C
	{ "RES 3 L", NULL, 3, L },            //CDx9D
	{ "RES 3 HL", NULL, 3, HL },          //CDx9E
	{ "RES 3 A", NULL, 3, A },            //CDx9F

	{ "RES 4 B", NULL, 4, B },			//CDxA0
	{ "RES 4 C", NULL, 4, C },			//CDxA1
	{ "RES 4 D", NULL, 4, D },			//CDxA2
	{ "RES 4 E", NULL, 4, E },			//CDxA3
	{ "RES 4 H", NULL, 4, H },			//CDxA4
	{ "RES 4 L", NULL, 4, L },			//CDxA5
	{ "RES 4 HL", NULL, 4, HL },			//CDxA6
	{ "RES 4 A", NULL, 4, A },			//CDxA7
	{ "RES 5 B", NULL, 5, B },			//CDxA8
	{ "RES 5 C", NULL, 5, C },			//CDxA9
	{ "RES 5 D", NULL, 5, D },			//CDxAA
	{ "RES 5 E", NULL, 5, E },			//CDxAB
	{ "RES 5 H", NULL, 5, H },			//CDxAC
	{ "RES 5 L", NULL, 5, L },			//CDxAD
	{ "RES 5 HL", NULL, 5, HL },			//CDxAE
	{ "RES 5 A", NULL, 5, A },			//CDxAF

	{ "RES 6 B", NULL, 6, B },			//CDxB0
	{ "RES 6 C", NULL, 6, C },			//CDxB1
	{ "RES 6 D", NULL, 6, D },			//CDxB2
	{ "RES 6 E", NULL, 6, E },			//CDxB3
	{ "RES 6 H", NULL, 6, H },			//CDxB4
	{ "RES 6 L", NULL, 6, L },			//CDxB5
	{ "RES 6 HL", NULL, 6, HL },			//CDxB6
	{ "RES 6 A", NULL, 6, A },			//CDxB7
	{ "RES 7 B", NULL, 7, B },			//CDxB8
	{ "RES 7 C", NULL, 7, C },			//CDxB9
	{ "RES 7 D", NULL, 7, D },			//CDxBA
	{ "RES 7 E", NULL, 7, E },			//CDxBB
	{ "RES 7 H", NULL, 7, H },			//CDxBC
	{ "RES 7 L", NULL, 7, L },			//CDxBD
	{ "RES 7 HL", NULL, 7, HL },			//CDxBE
	{ "RES 7 A", NULL, 7, A },			//CDxBF

	{ "SET 0 B", NULL, 0, B },			//CDxC0
	{ "SET 0 C", NULL, 0, C },			//CDxC1
	{ "SET 0 D", NULL, 0, D },			//CDxC2
	{ "SET 0 E", NULL, 0, E },			//CDxC3
	{ "SET 0 H", NULL, 0, H },			//CDxC4
	{ "SET 0 L", NULL, 0, L },			//CDxC5
	{ "SET 0 HL", NULL, 0, HL },			//CDxC6
	{ "SET 0 A", NULL, 0, A },			//CDxC7
	{ "SET 1 B", NULL, 1, B },			//CDxC8
	{ "SET 1 C", NULL, 1, C },			//CDxC9
	{ "SET 1 D", NULL, 1, D },			//CDxCA
	{ "SET 1 E", NULL, 1, E },			//CDxCB
	{ "SET 1 H", NULL, 1, H },			//CDxCC
	{ "SET 1 L", NULL, 1, L },			//CDxCD
	{ "SET 1 HL", NULL, 1, HL },			//CDxCE
	{ "SET 1 A", NULL, 1, A },			//CDxCF

	{ "SET 2 B", NULL, 2, B },            //CDxD0
	{ "SET 2 C", NULL, 2, C },            //CDxD1
	{ "SET 2 D", NULL, 2, D },            //CDxD2
	{ "SET 2 E", NULL, 2, E },            //CDxD3
	{ "SET 2 H", NULL, 2, H },            //CDxD4
	{ "SET 2 L", NULL, 2, L },            //CDxD5
	{ "SET 2 HL", NULL, 2, HL },          //CDxD6
	{ "SET 2 A", NULL, 2, A },            //CDxD7
	{ "SET 3 B", NULL, 3, B },            //CDxD8
	{ "SET 3 C", NULL, 3, C },            //CDxD9
	{ "SET 3 D", NULL, 3, D },            //CDxDA
	{ "SET 3 E", NULL, 3, E },            //CDxDB
	{ "SET 3 H", NULL, 3, H },            //CDxDC
	{ "SET 3 L", NULL, 3, L },            //CDxDD
	{ "SET 3 HL", NULL, 3, HL },          //CDxDE
	{ "SET 3 A", NULL, 3, A },            //CDxDF

	{ "SET 4 B", NULL, 4, B },            //CDxE0
	{ "SET 4 C", NULL, 4, C },            //CDxE1
	{ "SET 4 D", NULL, 4, D },            //CDxE2
	{ "SET 4 E", NULL, 4, E },            //CDxE3
	{ "SET 4 H", NULL, 4, H },            //CDxE4
	{ "SET 4 L", NULL, 4, L },            //CDxE5
	{ "SET 4 HL", NULL, 4, HL },          //CDxE6
	{ "SET 4 A", NULL, 4, A },            //CDxE7
	{ "SET 5 B", NULL, 5, B },            //CDxE8
	{ "SET 5 C", NULL, 5, C },            //CDxE9
	{ "SET 5 D", NULL, 5, D },            //CDxEA
	{ "SET 5 E", NULL, 5, E },            //CDxEB
	{ "SET 5 H", NULL, 5, H },            //CDxEC
	{ "SET 5 L", NULL, 5, L },            //CDxED
	{ "SET 5 HL", NULL, 5, HL },          //CDxEE
	{ "SET 5 A", NULL, 5, A },            //CDxEF

	{ "SET 6 B", NULL, 6, B },            //CDxF0
	{ "SET 6 C", NULL, 6, C },            //CDxF1
	{ "SET 6 D", NULL, 6, D },            //CDxF2
	{ "SET 6 E", NULL, 6, E },            //CDxF3
	{ "SET 6 H", NULL, 6, H },            //CDxF4
	{ "SET 6 L", NULL, 6, L },            //CDxF5
	{ "SET 6 HL", NULL, 6, HL },          //CDxF6
	{ "SET 6 A", NULL, 6, A },            //CDxF7
	{ "SET 7 B", NULL, 7, B },            //CDxF8
	{ "SET 7 C", NULL, 7, C },            //CDxF9
	{ "SET 7 D", NULL, 7, D },            //CDxFA
	{ "SET 7 E", NULL, 7, E },            //CDxFB
	{ "SET 7 H", NULL, 7, H },            //CDxFC
	{ "SET 7 L", NULL, 7, L },            //CDxFD
	{ "SET 7 HL", NULL, 7, HL },          //CDxFE
	{ "SET 7 A", NULL, 7, A },            //CDxFF
};
