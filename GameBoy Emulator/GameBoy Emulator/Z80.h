#include <stdlib.h>

#define ZERO_FLAG 0x80
#define SUBTRACT_FLAG 0x40
#define HALF_CARRY_FLAG 0x20
#define CARRY_FLAG 0x10

//cf is carry flag
enum Regval { A, B, C, D, E, H, L, AF, BC, DE, HL, Z, Cf, NC, NZ, SP, READ_8, READ_16, NA};

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
// arg1 = Regval (for most instructions put this NA unless it takes 2 registers)
// arg2 = Either immediate or Regval
typedef void (*OPCODE_OPERATION)(unsigned short arg1, unsigned short arg2);

typedef struct {
	char *disassembly;
	OPCODE_OPERATION execute;
	int r1;
	int r2;
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


static INSTR opcodes[] = {
	{"NOP", NULL, NULL, NULL},			//0x00
	{"LD BC, nn", LD_n_nn, BC, READ_16},	//0x01
	{"LD BC, A", LD_n_A, BC, A},		//0x02
	{"INC BC", INC_nn, BC, NA},		//0x03
	{"INC B", INC_n, NA, B},			//0x04
	{"DEC B", DEC_n, NA, B},			//0x05
	{"LD B, n", LD_nn_n, B, READ_8},		//0x06
	{"RLC A", NULL, A, NULL},			//0x07
	{"LD nn, SP", LD_nn_SP, READ_16, SP},	//0x08
	{"ADD HL, BC", ADD_HL_n, HL, BC},	//0x09
	{"LD A, BC", LD_A_n, A, BC},		//0x0A
	{"DEC BC", DEC_nn, BC, NA},		//0x0B
	{"INC C", INC_n, NA, C},			//0x0C
	{"DEC C", DEC_n, NA, C},			//0x0D
	{"LD C, n", LD_nn_n, C, READ_8},		//0x0E
	{"RRC A", NULL, A, NULL},			//0x0F
	{"STOP", NULL, NULL, NULL},			//0x10
	{"LD DE, nn", LD_n_nn, DE, READ_16},	//0x11
	{"LD DE, A", LD_n_A, DE, A},		//0x12
	{"INC DE", INC_nn, DE, NA},		//0x13
	{"INC D", INC_n, NA, D},			//0x14
	{"DEC D", DEC_n, NA, D},			//0x15
	{"LD D, n", LD_nn_n, D, READ_8},		//0x16
	{"RL A", NULL, A, NULL},			//0x17
	{"JR n", NULL, NULL, NULL},			//0x18
	{"ADD HL, DE", ADD_HL_n, HL, DE},	//0x19
	{"LD A, DE", LD_A_n, A, DE},		//0x1A
	{"DEC DE", DEC_nn, DE, NA},		//0x1B
	{"INC E", INC_n, NA, E},			//0x1C
	{"DEC E", DEC_n, NA, E},			//0x1D
	{"LD E, n", LD_nn_n, E, READ_8},		//0x1E
	{"RR A", NULL, A, NULL},			//0x1F
	{"JR NZ, n", NULL, NZ, NULL},		//0x20		//NZ is check for Not zero
	{"LD HL, nn", LD_n_nn, HL, READ_16},	//0x21
	{"LDI HL, A", LD_HLI_A, HL, A},		//0x22
	{"INC HL", INC_nn, HL, NA},		//0x23
	{"INC H", INC_n, NA, H},			//0x24
	{"DEC H", DEC_n, NA, H},			//0x25
	{"LD H, n", LD_nn_n, H, READ_8},		//0x26
	{"DAA", DAA, NA, NA},			//0x27
	{"JR Z, n", NULL, Z, NULL},		//0x28		//Z is check for zero
	{"ADD HL, HL", ADD_HL_n, HL, HL},	//0x29
	{"LDI A, HL", LD_A_HLI, A, HL},		//0x2A
	{"DEC HL", DEC_nn, HL, NA},		//0x2B
	{"INC L", INC_n, NA, L},			//0x2C
	{"DEC L", DEC_n, NA, L},			//0x2D
	{"LD L, n", LD_nn_n, L, READ_8},		//0x2E
	{"CPL", NULL, NULL, NULL},			//0x2F
	{"JR NC, n", NULL, NC, NULL},		//0x30		//NC is check for no carry
	{"LD SP, nn", LD_n_nn, SP, READ_16},	//0x31
	{"LDD HL, A", LD_HLD_A, HL, A},		//0x32
	{"INC SP", INC_nn, SP, NA},		//0x33
	{"INC HL", INC_n, NA, HL},		//0x34
	{"DEC HL", DEC_n, NA, HL},		//0x35
	{"LD HL, n", LD_r1_r2, HL, READ_8},		//0x36 CHECK IF READ 8
	{"SCF", NULL, NULL, NULL},			//0x37
	{"JR C, n", NULL, C, NULL},		//0x38		//C is check for carry
	{"ADD HL, SP", ADD_HL_n, HL, SP},	//0x39
	{"LDD A, HL", LD_A_HLD, A, HL},		//0x3A
	{"DEC SP", DEC_nn, SP, NA},		//0x3B
	{"INC A", INC_n, NA, A},			//0x3C
	{"DEC A", DEC_n, NA, A},			//0x3D
	{"LD A, n", LD_A_imm, A, READ_8},		//0x3E
	{"CCF", NULL, NULL, NULL},			//0x3F
	{"LD B, B", LD_r1_r2, B, B},		//0x40
	{"LD B, C", LD_r1_r2, B, C},		//0x41
	{"LD B, D", LD_r1_r2, B, D},		//0x42
	{"LD B, E", LD_r1_r2, B, E},		//0x43
	{"LD B, H", LD_r1_r2, B, H},		//0x44
	{"LD B, L", LD_r1_r2, B, L},		//0x45
	{"LD B, HL", LD_r1_r2, B, HL},		//0x46
	{"LD B, A", LD_n_A, B, A},		//0x47
	{"LD C, B", LD_r1_r2, C, B},		//0x48
	{"LD C, C", LD_r1_r2, C, C},		//0x49
	{"LD C, D", LD_r1_r2, C, D},		//0x4A
	{"LD C, E", LD_r1_r2, C, E},		//0x4B
	{"LD C, H", LD_r1_r2, C, H},		//0x4C
	{"LD C, L", LD_r1_r2, C, L},		//0x4D
	{"LD C, HL", LD_r1_r2, C, HL},		//0x4E
	{"LD C, A", LD_n_A, C, A},		//0x4F
	{"LD D, B", LD_r1_r2, D, B},		//0x50
	{"LD D, C", LD_r1_r2, D, C},		//0x51
	{"LD D, D", LD_r1_r2, D, D},		//0x52
	{"LD D, E", LD_r1_r2, D, E},		//0x53
	{"LD D, H", LD_r1_r2, D, H},		//0x54
	{"LD D, L", LD_r1_r2, D, L},		//0x55
	{"LD D, HL", LD_r1_r2, D, HL},		//0x56
	{"LD D, A", LD_n_A, D, A},		//0x57
	{"LD E, B", LD_r1_r2, E, B},		//0x58
	{"LD E, C", LD_r1_r2, E, C},		//0x59
	{"LD E, D", LD_r1_r2, E, D},		//0x5A
	{"LD E, E", LD_r1_r2, E, E},		//0x5B
	{"LD E, H", LD_r1_r2, E, H},		//0x5C
	{"LD E, L", LD_r1_r2, E, L},		//0x5D
	{"LD E, HL", LD_r1_r2, E, HL},		//0x5E
	{"LD E, A", LD_n_A, E, A},		//0x5F
	{"LD H, B", LD_r1_r2, H, B},		//0x60
	{"LD H, C", LD_r1_r2, H, C},		//0x61
	{"LD H, D", LD_r1_r2, H, D},		//0x62
	{"LD H, E", LD_r1_r2, H, E},		//0x63
	{"LD H, H", LD_r1_r2, H, H},		//0x64
	{"LD H, L", LD_r1_r2, H, L},		//0x65
	{"LD H, HL", LD_r1_r2, H, HL},		//0x66
	{"LD H, A", LD_n_A, H, A},		//0x67
	{"LD L, B", LD_r1_r2, L, B},		//0x68
	{"LD L, C", LD_r1_r2, L, C},		//0x69
	{"LD L, D", LD_r1_r2, L, D},		//0x6A
	{"LD L, E", LD_r1_r2, L, E},		//0x6B
	{"LD L, H", LD_r1_r2, L, H},		//0x6C
	{"LD L, L", LD_r1_r2, L, L},		//0x6D
	{"LD L, HL", LD_r1_r2, L, HL},		//0x6E
	{"LD L, A", LD_n_A, L, A},		//0x6F
	{"LD HL, B", LD_r1_r2, HL, B},		//0x70
	{"LD HL, C", LD_r1_r2, HL, C},		//0x71
	{"LD HL, D", LD_r1_r2, HL, D},		//0x72
	{"LD HL, E", LD_r1_r2, HL, E},		//0x73
	{"LD HL, H", LD_r1_r2, HL, H},		//0x74
	{"LD HL, L", LD_r1_r2, HL, L},		//0x75
	{"HALT", NULL, NULL, NULL},		//0x76
	{"LD HL, A", LD_n_A, HL, A},		//0x77
	{"LD A, B", LD_r1_r2, A, B},		//0x78
	{"LD A, C", LD_r1_r2, A, C},		//0x79
	{"LD A, D", LD_r1_r2, A, D},		//0x7A
	{"LD A, E", LD_r1_r2, A, E},		//0x7B
	{"LD A, H", LD_r1_r2, A, H},		//0x7C
	{"LD A, L", LD_r1_r2, A, L},		//0x7D
	{"LD A, HL", LD_r1_r2, A, HL},		//0x7E
	{"LD A, A", LD_r1_r2, A, A},		//0x7F
	{"ADD A, B", ADD_A_n, A, B},		//0x80
	{"ADD A, C", ADD_A_n, A, C},		//0x81
	{"ADD A, D", ADD_A_n, A, D},		//0x82
	{"ADD A, E", ADD_A_n, A, E},		//0x83
	{"ADD A, H", ADD_A_n, A, H},		//0x84
	{"ADD A, L", ADD_A_n, A, L },		//0x85
	{"ADD A, HL", ADD_A_n, A, HL},		//0x86
	{"ADD A, A", ADD_A_n, A, A},		//0x87
	{"ADC A, B", ADC_A_n, A, B},		//0x88
	{"ADC A, C", ADC_A_n, A, C},		//0x89
	{"ADC A, D", ADC_A_n, A, D},		//0x8A
	{"ADC A, E", ADC_A_n, A, E},		//0x8B
	{"ADC A, H", ADC_A_n, A, H},		//0x8C
	{"ADC A, L", ADC_A_n, A, L},		//0x8D
	{"ADC A, HL", ADC_A_n, A, HL},		//0x8E
	{"ADC A, A", ADC_A_n, A, A},		//0x8F
	{"SUB A, B", SUB_n, A, B},		//0x90
	{"SUB A, C", SUB_n, A, C},		//0x91
	{"SUB A, D", SUB_n, A, D},		//0x92
	{"SUB A, E", SUB_n, A, E},		//0x93
	{"SUB A, H", SUB_n, A, H},		//0x94
	{"SUB A, L", SUB_n, A, L},		//0x95
	{"SUB A, HL", SUB_n, A, HL},		//0x96
	{"SUB A, A", SUB_n, A, A},		//0x97
	{"SBC A, B", SBC_A_n, A, B},		//0x98
	{"SBC A, C", SBC_A_n, A, C},		//0x99
	{"SBC A, D", SBC_A_n, A, D},		//0x9A
	{"SBC A, E", SBC_A_n, A, E},		//0x9B
	{"SBC A, H", SBC_A_n, A, H},		//0x9C
	{"SBC A, L", SBC_A_n, A, L },		//0x9D
	{"SBC A, HL", SBC_A_n, A, HL},		//0x9E
	{"SBC A, A", SBC_A_n, A, A},		//0x9F
	{"AND B", AND_n, B, B},		//0xA0
	{"AND C", AND_n, NA, C},		//0xA1
	{"AND D", AND_n, NA, D},		//0xA2
	{"AND E", AND_n, NA, E},		//0xA3
	{"AND H", AND_n, NA, H},		//0xA4
	{"AND L", AND_n, NA, L},		//0xA5
	{"AND HL", AND_n, NA, HL},		//0xA6
	{"AND A", AND_n, NA, A},		//0xA7
	{"XOR B", XOR_n, NA, B},		//0xA8
	{"XOR C", XOR_n, NA, C},		//0xA9
	{"XOR D", XOR_n, NA, D},		//0xAA
	{"XOR E", XOR_n, NA, E},		//0xAB
	{"XOR H", XOR_n, NA, H},		//0xAC
	{"XOR L", XOR_n, NA, L},		//0xAD
	{"XOR HL", XOR_n, NA, HL},		//0xAE
	{"XOR A", XOR_n, NA, A},		//0xAF
	{"OR B", OR_n, NA, B},		//0xB0
	{"OR C", OR_n, NA, C},		//0xB1
	{"OR D", OR_n, NA, D},		//0xB2
	{"OR E", OR_n, NA, E},		//0xB3
	{"OR H", OR_n, NA, H},		//0xB4
	{"OR L", OR_n, NA, L},		//0xB5
	{"OR HL", OR_n, NA, HL},		//0xB6
	{"OR A", OR_n, NA, A},		//0xB7
	{"CP B", CP_n, NA, B},		//0xB8
	{"CP C", CP_n, NA, C},		//0xB9
	{"CP D", CP_n, NA, D},		//0xBA
	{"CP E", CP_n, NA, E},		//0xBB
	{"CP H", CP_n, NA, H},		//0xBC
	{"CP L", CP_n, NA, L},		//0xBD
	{"CP HL", CP_n, NA, HL},		//0xBE
	{"CP A", CP_n, NA, A},		//0xBF
	{"RET NZ", NULL, NZ, NULL},		//0xC0		//NZ means check for not zero
	{"POP BC", POP_nn, BC, NA},		//0xC1
	{"JP NZ, nn", NULL, NZ, NULL},	//0xC2
	{"JP nn", NULL, NULL, NULL },	//0xC3
	{"CALL NZ, nn", NULL, NZ, NULL},//0xC4
	{"PUSH BC", PUSH_nn, BC, NA},	//0xC5
	{"ADD A, n", ADD_A_n, READ_8, READ_8},	//0xC6
	{"RST 0", NULL, "0", NULL},		//0xC7
	{"RET Z", NULL, Z, NULL },		//0xC8
	{"RET", NULL, NULL, NULL },		//0xC9
	{"JP Z, nn", NULL, Z, NULL},	//0xCA
	{"Ext Ops (CB)", NULL, NULL, NULL},	//0xCB
	{"CALL Z, nn", NULL, Z, NULL},	//0xCC
	{"CALL nn", NULL, NULL, NULL},		//0xCD
	{"ADC A, n", ADC_A_n, READ_8, READ_8},		//0xCE
	{"RST 8", NULL, "8", NULL},		//0xCF
	{"RFT NC", NULL, NC, NULL},		//0xD0
	{"POP DE", POP_nn, DE, NA},		//0xD1
	{"JP NC, nn", NULL, NC, NULL},	//0xD2
	{"XX", NULL, NULL, NULL},			//0xD3
	{"CALL NC, nn", NULL, NC, NULL},	//0xD4
	{"PUSH DE", PUSH_nn, DE, NA},		//0xD5
	{"SUB A, n", SUB_n, READ_8, READ_8},		//0xD6
	{"RST 10", NULL, "10", NULL},		//0xD7
	{"RET C", NULL, C, NULL},			//0xD8
	{"RETI", NULL, NULL, NULL},			//0xD9
	{"JP C, nn", NULL, C, NULL},		//0xDA
	{"XX", NULL, NULL, NULL},			//0xDB
	{"JP C, nn", NULL, C, NULL},		//0xDC
	{"XX", NULL, NULL, NULL},			//0xDD
	{"SBC A, n", SBC_A_n, READ_8, READ_8},		//0xDE
	{"RST 18", NULL, "18", NULL},		//0xDF
	{"LDH n, A", LDH_n_A, READ_8, A},		//0xE0
	{"POP HL", POP_nn, HL, NA},		//0xE1
	{"LD (C), A", LD_C_A, C, A},		//0xE2
	{"XX", NULL, NULL, NULL},			//0xE3
	{"XX", NULL, NULL, NULL},			//0xE4
	{"PUSH HL", PUSH_nn, HL, NA},		//0xE5
	{"AND n", AND_n, READ_8, READ_8},		//0xE6
	{"RST 20", NULL, "20", NULL},		//0xE7
	{"ADD SP, n", ADD_SP_n, SP, READ_8},	//0xE8
	{"JP HL", NULL, HL, NULL},		//0xE9
	{"LD nn, A", LD_n_A, READ_16, A},		//0xEA
	{"XX", NULL, NULL, NULL},			//0xEB
	{"XX", NULL, NULL, NULL},			//0xEC
	{"XX", NULL, NULL, NULL},			//0xED
	{"XOR n", XOR_n, READ_8, READ_8},		//0xEE
	{"RST 28", NULL, "28", NULL},		//0xEF
	{"LDH A, n", LDH_A_n, A, READ_8},		//0xF0
	{"POP AF", POP_nn, AF, NA},		//0xF1
	{"LD A, (C)", LD_A_C, A, C},			//0xF2
	{"DI", NULL, NULL, NULL},			//0xF3
	{"XX", NULL, NULL, NULL},			//0xF4
	{"PUSH AF", PUSH_nn, AF, NA},		//0xF5
	{"OR n", OR_n, READ_8, READ_8},			//0xF6
	{"RST 30", NULL, "30", NULL},		//0xF7
	{"LDHL SP, n", LDHL_SP_n, SP, READ_8},	//0xF8
	{"LD SP, HL", LD_SP_HL, SP, HL},	//0xF9
	{"LD A, nn", LD_A_nn, A, READ_16},		//0xFA
	{"EI", NULL, NULL, NULL},			//0xFB
	{"XX", NULL, NULL, NULL},			//0xFC
	{"XX", NULL, NULL, NULL},			//0xFD
	{"CP n", CP_n, READ_8, READ_8},			//0xFE
	{"RST 38", NULL, "28", NULL},		//0xFF
};

static INSTR opcodesCB[] = {
	{ "RLC B", NULL, B, NULL },           //0xCB 00
	{ "RLC C", NULL, C, NULL },           //0xCB 01
	{ "RLC D", NULL, D, NULL },           //0xCB 02
	{ "RLC E", NULL, E, NULL },           //0xCB 03
	{ "RLC H", NULL, H, NULL },           //0xCB 04
	{ "RLC L", NULL, L, NULL },           //0xCB 05
	{ "RLC HL", NULL, HL, NULL },         //0xCB 06
	{ "RLC A", NULL, A, NULL },           //0xCB 07
	{ "RRC B", NULL, B, NULL },           //0xCB 08
	{ "RRC C", NULL, C, NULL },           //0xCB 09
	{ "RRC D", NULL, D, NULL },           //0xCB 0A
	{ "RRC E", NULL, E, NULL },           //0xCB 0B
	{ "RRC H", NULL, H, NULL },           //0xCB 0C
	{ "RRC L", NULL, L, NULL },           //0xCB 0D
	{ "RRC HL", NULL, HL, NULL },         //0xCB 0E
	{ "RRC A", NULL, A, NULL },           //0xCB 0F

	{ "RL B", NULL, B, NULL },            //0xCB 10
	{ "RL C", NULL, C, NULL },            //0xCB 11
	{ "RL D", NULL, D, NULL },            //0xCB 12
	{ "RL E", NULL, E, NULL },            //0xCB 13
	{ "RL H", NULL, H, NULL },            //0xCB 14
	{ "RL L", NULL, L, NULL },            //0xCB 15
	{ "RL HL", NULL, HL, NULL },          //0xCB 16
	{ "RL A", NULL, A, NULL },            //0xCB 17
	{ "RR B", NULL, B, NULL },            //0xCB 18
	{ "RR C", NULL, C, NULL },            //0xCB 19
	{ "RR D", NULL, D, NULL },            //0xCB 1A
	{ "RR E", NULL, E, NULL },            //0xCB 1B
	{ "RR H", NULL, H, NULL },            //0xCB 1C
	{ "RR L", NULL, L, NULL },            //0xCB 1D
	{ "RR HL", NULL, HL, NULL },          //0xCB 1E
	{ "RR A", NULL, A, NULL },            //0xCB 1F

	{ "SLA B", NULL, B, NULL },           //0xCB 20
	{ "SLA C", NULL, C, NULL },           //0xCB 21
	{ "SLA D", NULL, D, NULL },           //0xCB 22
	{ "SLA E", NULL, E, NULL },           //0xCB 23
	{ "SLA H", NULL, H, NULL },           //0xCB 24
	{ "SLA L", NULL, L, NULL },           //0xCB 25
	{ "SLA HL", NULL, HL, NULL },         //0xCB 26
	{ "SLA A", NULL, A, NULL },           //0xCB 27
	{ "SRA B", NULL, B, NULL },           //0xCB 28
	{ "SRA C", NULL, C, NULL },           //0xCB 29
	{ "SRA D", NULL, D, NULL },           //0xCB 2A
	{ "SRA E", NULL, E, NULL },           //0xCB 2B
	{ "SRA H", NULL, H, NULL },           //0xCB 2C
	{ "SRA L", NULL, L, NULL },           //0xCB 2D
	{ "SRA HL", NULL, HL, NULL },         //0xCB 2E
	{ "SRA A", NULL, A, NULL },           //0xCB 2F

	{ "SWAP B", SWAP_n, B, NA },          //0xCB 30
	{ "SWAP C", SWAP_n, C, NA },          //0xCB 31
	{ "SWAP D", SWAP_n, D, NA },          //0xCB 32
	{ "SWAP E", SWAP_n, E, NA },          //0xCB 33
	{ "SWAP H", SWAP_n, H, NA },          //0xCB 34
	{ "SWAP L", SWAP_n, L, NA },          //0xCB 35
	{ "SWAP HL", SWAP_n, HL, NA },        //0xCB 36
	{ "SWAP A", SWAP_n, A, NA },          //0xCB 37
	{ "SRL B", NULL, B, NULL },           //0xCB 38
	{ "SRL C", NULL, C, NULL },           //0xCB 39
	{ "SRL D", NULL, D, NULL },           //0xCB 3A
	{ "SRL E", NULL, E, NULL },           //0xCB 3B
	{ "SRL H", NULL, H, NULL },           //0xCB 3C
	{ "SRL L", NULL, L, NULL },           //0xCB 3D
	{ "SRL HL", NULL, HL, NULL },         //0xCB 3E
	{ "SRL A", NULL, A, NULL },           //0xCB 3F

	{ "BIT 0 B", NULL, 0, B },            //0xCB 40
	{ "BIT 0 C", NULL, 0, C },            //0xCB 41
	{ "BIT 0 D", NULL, 0, D },            //0xCB 42
	{ "BIT 0 E", NULL, 0, E },            //0xCB 43
	{ "BIT 0 H", NULL, 0, H },            //0xCB 44
	{ "BIT 0 L", NULL, 0, L },            //0xCB 45
	{ "BIT 0 HL", NULL, 0, HL },          //0xCB 46
	{ "BIT 0 A", NULL, 0, A },            //0xCB 47
	{ "BIT 1 B", NULL, 1, B },            //0xCB 48
	{ "BIT 1 C", NULL, 1, C },            //0xCB 49
	{ "BIT 1 D", NULL, 1, D },            //0xCB 4A
	{ "BIT 1 E", NULL, 1, E },            //0xCB 4B
	{ "BIT 1 H", NULL, 1, H },            //0xCB 4C
	{ "BIT 1 L", NULL, 1, L },            //0xCB 4D
	{ "BIT 1 HL", NULL, 1, HL },          //0xCB 4E
	{ "BIT 1 A", NULL, 1, A },            //0xCB 4F

	{ "BIT 2 B", NULL, 2, B },            //0xCB 50
	{ "BIT 2 C", NULL, 2, C },            //0xCB 51
	{ "BIT 2 D", NULL, 2, D },            //0xCB 52
	{ "BIT 2 E", NULL, 2, E },            //0xCB 53
	{ "BIT 2 H", NULL, 2, H },            //0xCB 54
	{ "BIT 2 L", NULL, 2, L },            //0xCB 55
	{ "BIT 2 HL", NULL, 2, HL },          //0xCB 56
	{ "BIT 2 A", NULL, 2, A },            //0xCB 57
	{ "BIT 3 B", NULL, 3, B },            //0xCB 58
	{ "BIT 3 C", NULL, 3, C },            //0xCB 59
	{ "BIT 3 D", NULL, 3, D },            //0xCB 5A
	{ "BIT 3 E", NULL, 3, E },            //0xCB 5B
	{ "BIT 3 H", NULL, 3, H },            //0xCB 5C
	{ "BIT 3 L", NULL, 3, L },            //0xCB 5D
	{ "BIT 3 HL", NULL, 3, HL },          //0xCB 5E
	{ "BIT 3 A", NULL, 3, A },            //0xCB 5F

	{ "BIT 4 B", NULL, 4, B },            //0xCB 60
	{ "BIT 4 C", NULL, 4, C },            //0xCB 61
	{ "BIT 4 D", NULL, 4, D },            //0xCB 62
	{ "BIT 4 E", NULL, 4, E },            //0xCB 63
	{ "BIT 4 H", NULL, 4, H },            //0xCB 64
	{ "BIT 4 L", NULL, 4, L },            //0xCB 65
	{ "BIT 4 HL", NULL, 4, HL },          //0xCB 66
	{ "BIT 4 A", NULL, 4, A },            //0xCB 67
	{ "BIT 5 B", NULL, 5, B },            //0xCB 68
	{ "BIT 5 C", NULL, 5, C },            //0xCB 69
	{ "BIT 5 D", NULL, 5, D },            //0xCB 6A
	{ "BIT 5 E", NULL, 5, E },            //0xCB 6B
	{ "BIT 5 H", NULL, 5, H },            //0xCB 6C
	{ "BIT 5 L", NULL, 5, L },            //0xCB 6D
	{ "BIT 5 HL", NULL, 5, HL },          //0xCB 6E
	{ "BIT 5 A", NULL, 5, A },            //0xCB 6F

	{ "BIT 6 B", NULL, 6, B },            //0xCB 70
	{ "BIT 6 C", NULL, 6, C },            //0xCB 71
	{ "BIT 6 D", NULL, 6, D },            //0xCB 72
	{ "BIT 6 E", NULL, 6, E },            //0xCB 73
	{ "BIT 6 H", NULL, 6, H },            //0xCB 74
	{ "BIT 6 L", NULL, 6, L },            //0xCB 75
	{ "BIT 6 HL", NULL, 6, HL },          //0xCB 76
	{ "BIT 6 A", NULL, 6, A },            //0xCB 77
	{ "BIT 7 B", NULL, 7, B },            //0xCB 78
	{ "BIT 7 C", NULL, 7, C },            //0xCB 79
	{ "BIT 7 D", NULL, 7, D },            //0xCB 7A
	{ "BIT 7 E", NULL, 7, E },            //0xCB 7B
	{ "BIT 7 H", NULL, 7, H },            //0xCB 7C
	{ "BIT 7 L", NULL, 7, L },            //0xCB 7D
	{ "BIT 7 HL", NULL, 7, HL },          //0xCB 7E
	{ "BIT 7 A", NULL, 7, A },            //0xCB 7F

	{ "RES 0 B", NULL, 0, B },            //0xCB 80
	{ "RES 0 C", NULL, 0, C },            //0xCB 81
	{ "RES 0 D", NULL, 0, D },            //0xCB 82
	{ "RES 0 E", NULL, 0, E },            //0xCB 83
	{ "RES 0 H", NULL, 0, H },            //0xCB 84
	{ "RES 0 L", NULL, 0, L },            //0xCB 85
	{ "RES 0 HL", NULL, 0, HL },          //0xCB 86
	{ "RES 0 A", NULL, 0, A },            //0xCB 87
	{ "RES 1 B", NULL, 1, B },            //0xCB 88
	{ "RES 1 C", NULL, 1, C },            //0xCB 89
	{ "RES 1 D", NULL, 1, D },            //0xCB 8A
	{ "RES 1 E", NULL, 1, E },            //0xCB 8B
	{ "RES 1 H", NULL, 1, H },            //0xCB 8C
	{ "RES 1 L", NULL, 1, L },            //0xCB 8D
	{ "RES 1 HL", NULL, 1, HL },          //0xCB 8E
	{ "RES 1 A", NULL, 1, A },            //0xCB 8F

	{ "RES 2 B", NULL, 2, B },            //0xCB 90
	{ "RES 2 C", NULL, 2, C },            //0xCB 91
	{ "RES 2 D", NULL, 2, D },            //0xCB 92
	{ "RES 2 E", NULL, 2, E },            //0xCB 93
	{ "RES 2 H", NULL, 2, H },            //0xCB 94
	{ "RES 2 L", NULL, 2, L },            //0xCB 95
	{ "RES 2 HL", NULL, 2, HL },          //0xCB 96
	{ "RES 2 A", NULL, 2, A },            //0xCB 97
	{ "RES 3 B", NULL, 3, B },            //0xCB 98
	{ "RES 3 C", NULL, 3, C },            //0xCB 99
	{ "RES 3 D", NULL, 3, D },            //0xCB 9A
	{ "RES 3 E", NULL, 3, E },            //0xCB 9B
	{ "RES 3 H", NULL, 3, H },            //0xCB 9C
	{ "RES 3 L", NULL, 3, L },            //0xCB 9D
	{ "RES 3 HL", NULL, 3, HL },          //0xCB 9E
	{ "RES 3 A", NULL, 3, A },            //0xCB 9F

	{ "RES 4 B", NULL, 4, B },			//0xCB A0
	{ "RES 4 C", NULL, 4, C },			//0xCB A1
	{ "RES 4 D", NULL, 4, D },			//0xCB A2
	{ "RES 4 E", NULL, 4, E },			//0xCB A3
	{ "RES 4 H", NULL, 4, H },			//0xCB A4
	{ "RES 4 L", NULL, 4, L },			//0xCB A5
	{ "RES 4 HL", NULL, 4, HL },			//0xCB A6
	{ "RES 4 A", NULL, 4, A },			//0xCB A7
	{ "RES 5 B", NULL, 5, B },			//0xCB A8
	{ "RES 5 C", NULL, 5, C },			//0xCB A9
	{ "RES 5 D", NULL, 5, D },			//0xCB AA
	{ "RES 5 E", NULL, 5, E },			//0xCB AB
	{ "RES 5 H", NULL, 5, H },			//0xCB AC
	{ "RES 5 L", NULL, 5, L },			//0xCB AD
	{ "RES 5 HL", NULL, 5, HL },			//0xCB AE
	{ "RES 5 A", NULL, 5, A },			//0xCB AF

	{ "RES 6 B", NULL, 6, B },			//0xCB B0
	{ "RES 6 C", NULL, 6, C },			//0xCB B1
	{ "RES 6 D", NULL, 6, D },			//0xCB B2
	{ "RES 6 E", NULL, 6, E },			//0xCB B3
	{ "RES 6 H", NULL, 6, H },			//0xCB B4
	{ "RES 6 L", NULL, 6, L },			//0xCB B5
	{ "RES 6 HL", NULL, 6, HL },			//0xCB B6
	{ "RES 6 A", NULL, 6, A },			//0xCB B7
	{ "RES 7 B", NULL, 7, B },			//0xCB B8
	{ "RES 7 C", NULL, 7, C },			//0xCB B9
	{ "RES 7 D", NULL, 7, D },			//0xCB BA
	{ "RES 7 E", NULL, 7, E },			//0xCB BB
	{ "RES 7 H", NULL, 7, H },			//0xCB BC
	{ "RES 7 L", NULL, 7, L },			//0xCB BD
	{ "RES 7 HL", NULL, 7, HL },			//0xCB BE
	{ "RES 7 A", NULL, 7, A },			//0xCB BF

	{ "SET 0 B", NULL, 0, B },			//0xCB C0
	{ "SET 0 C", NULL, 0, C },			//0xCB C1
	{ "SET 0 D", NULL, 0, D },			//0xCB C2
	{ "SET 0 E", NULL, 0, E },			//0xCB C3
	{ "SET 0 H", NULL, 0, H },			//0xCB C4
	{ "SET 0 L", NULL, 0, L },			//0xCB C5
	{ "SET 0 HL", NULL, 0, HL },			//0xCB C6
	{ "SET 0 A", NULL, 0, A },			//0xCB C7
	{ "SET 1 B", NULL, 1, B },			//0xCB C8
	{ "SET 1 C", NULL, 1, C },			//0xCB C9
	{ "SET 1 D", NULL, 1, D },			//0xCB CA
	{ "SET 1 E", NULL, 1, E },			//0xCB CB
	{ "SET 1 H", NULL, 1, H },			//0xCB CC
	{ "SET 1 L", NULL, 1, L },			//0xCB CD
	{ "SET 1 HL", NULL, 1, HL },			//0xCB CE
	{ "SET 1 A", NULL, 1, A },			//0xCB CF

	{ "SET 2 B", NULL, 2, B },            //0xCB D0
	{ "SET 2 C", NULL, 2, C },            //0xCB D1
	{ "SET 2 D", NULL, 2, D },            //0xCB D2
	{ "SET 2 E", NULL, 2, E },            //0xCB D3
	{ "SET 2 H", NULL, 2, H },            //0xCB D4
	{ "SET 2 L", NULL, 2, L },            //0xCB D5
	{ "SET 2 HL", NULL, 2, HL },          //0xCB D6
	{ "SET 2 A", NULL, 2, A },            //0xCB D7
	{ "SET 3 B", NULL, 3, B },            //0xCB D8
	{ "SET 3 C", NULL, 3, C },            //0xCB D9
	{ "SET 3 D", NULL, 3, D },            //0xCB DA
	{ "SET 3 E", NULL, 3, E },            //0xCB DB
	{ "SET 3 H", NULL, 3, H },            //0xCB DC
	{ "SET 3 L", NULL, 3, L },            //0xCB DD
	{ "SET 3 HL", NULL, 3, HL },          //0xCB DE
	{ "SET 3 A", NULL, 3, A },            //0xCB DF

	{ "SET 4 B", NULL, 4, B },            //0xCB E0
	{ "SET 4 C", NULL, 4, C },            //0xCB E1
	{ "SET 4 D", NULL, 4, D },            //0xCB E2
	{ "SET 4 E", NULL, 4, E },            //0xCB E3
	{ "SET 4 H", NULL, 4, H },            //0xCB E4
	{ "SET 4 L", NULL, 4, L },            //0xCB E5
	{ "SET 4 HL", NULL, 4, HL },          //0xCB E6
	{ "SET 4 A", NULL, 4, A },            //0xCB E7
	{ "SET 5 B", NULL, 5, B },            //0xCB E8
	{ "SET 5 C", NULL, 5, C },            //0xCB E9
	{ "SET 5 D", NULL, 5, D },            //0xCB EA
	{ "SET 5 E", NULL, 5, E },            //0xCB EB
	{ "SET 5 H", NULL, 5, H },            //0xCB EC
	{ "SET 5 L", NULL, 5, L },            //0xCB ED
	{ "SET 5 HL", NULL, 5, HL },          //0xCB EE
	{ "SET 5 A", NULL, 5, A },            //0xCB EF

	{ "SET 6 B", NULL, 6, B },            //0xCB F0
	{ "SET 6 C", NULL, 6, C },            //0xCB F1
	{ "SET 6 D", NULL, 6, D },            //0xCB F2
	{ "SET 6 E", NULL, 6, E },            //0xCB F3
	{ "SET 6 H", NULL, 6, H },            //0xCB F4
	{ "SET 6 L", NULL, 6, L },            //0xCB F5
	{ "SET 6 HL", NULL, 6, HL },          //0xCB F6
	{ "SET 6 A", NULL, 6, A },            //0xCB F7
	{ "SET 7 B", NULL, 7, B },            //0xCB F8
	{ "SET 7 C", NULL, 7, C },            //0xCB F9
	{ "SET 7 D", NULL, 7, D },            //0xCB FA
	{ "SET 7 E", NULL, 7, E },            //0xCB FB
	{ "SET 7 H", NULL, 7, H },            //0xCB FC
	{ "SET 7 L", NULL, 7, L },            //0xCB FD
	{ "SET 7 HL", NULL, 7, HL },          //0xCB FE
	{ "SET 7 A", NULL, 7, A },            //0xCB FF
};
