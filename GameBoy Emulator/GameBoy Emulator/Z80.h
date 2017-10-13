#include <stdlib.h>

#define ZERO_FLAG 0x80
#define OPERATION_FLAG 0x40
#define HALF_CARRY_FLAG 0x20
#define CARRY_FLAG 0x10

enum Regval { A, B, C, D, E, H, L, AF, BC, DE, HL };

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

static INSTR OpcodesCD[] = {
        {"RLC B", NULL, B, NULL},         //CDx00
        {"RLC C", NULL, C, NULL},         //CDx01
        {"RLC D", NULL, D, NULL},         //CDx02
        {"RLC E", NULL, E, NULL},         //CDx03
        {"RLC H", NULL, H, NULL},         //CDx04
        {"RLC L", NULL, L, NULL},         //CDx05
        {"RLC HL", NULL, HL, NULL},       //CDx06
        {"RLC A", NULL, A, NULL},         //CDx07
        {"RRC B", NULL, B, NULL},         //CDx08
        {"RRC C", NULL, C, NULL},         //CDx09
        {"RRC D", NULL, D, NULL},         //CDx0A
        {"RRC E", NULL, E, NULL},         //CDx0B
        {"RRC H", NULL, H, NULL},         //CDx0C
        {"RRC L", NULL, L, NULL},         //CDx0D
        {"RRC HL", NULL, HL, NULL},       //CDx0E
        {"RRC A", NULL, A, NULL},         //CDx0F

        {"RL B", NULL, B, NULL},            //CDx10
        {"RL C", NULL, C, NULL},            //CDx11
        {"RL D", NULL, D, NULL},            //CDx12
        {"RL E", NULL, E, NULL},            //CDx13
        {"RL H", NULL, H, NULL},            //CDx14
        {"RL L", NULL, L, NULL},            //CDx15
        {"RL HL", NULL, HL, NULL},          //CDx16
        {"RL A", NULL, A, NULL},            //CDx17
        {"RR B", NULL, B, NULL},            //CDx18
        {"RR C", NULL, C, NULL},            //CDx19
        {"RR D", NULL, D, NULL},            //CDx1A
        {"RR E", NULL, E, NULL},            //CDx1B
        {"RR H", NULL, H, NULL},            //CDx1C
        {"RR L", NULL, L, NULL},            //CDx1D
        {"RR HL", NULL, HL, NULL},          //CDx1E
        {"RR A", NULL, A, NULL},            //CDx1F

        {"SLA B", NULL, B, NULL},           //CDx20
        {"SLA C", NULL, C, NULL},           //CDx21
        {"SLA D", NULL, D, NULL},           //CDx22
        {"SLA E", NULL, E, NULL},           //CDx23
        {"SLA H", NULL, H, NULL},           //CDx24
        {"SLA L", NULL, L, NULL},           //CDx25
        {"SLA HL", NULL, HL, NULL},         //CDx26
        {"SLA A", NULL, A, NULL},           //CDx27
        {"SRA B", NULL, B, NULL},           //CDx28
        {"SRA C", NULL, C, NULL},           //CDx29
        {"SRA D", NULL, D, NULL},           //CDx2A
        {"SRA E", NULL, E, NULL},           //CDx2B
        {"SRA H", NULL, H, NULL},           //CDx2C
        {"SRA L", NULL, L, NULL},           //CDx2D
        {"SRA HL", NULL, HL, NULL},         //CDx2E
        {"SRA A", NULL, A, NULL},           //CDx2F

        {"SWAP B", NULL, B, NULL},          //CDx30
        {"SWAP C", NULL, C, NULL},          //CDx31
        {"SWAP D", NULL, D, NULL},          //CDx32
        {"SWAP E", NULL, E, NULL},          //CDx33
        {"SWAP H", NULL, H, NULL},          //CDx34
        {"SWAP L", NULL, L, NULL},          //CDx35
        {"SWAP HL", NULL, HL, NULL},        //CDx36
        {"SWAP A", NULL, A, NULL},          //CDx37
        {"SRL B", NULL, B, NULL},           //CDx38
        {"SRL C", NULL, C, NULL},           //CDx39
        {"SRL D", NULL, D, NULL},           //CDx3A
        {"SRL E", NULL, E, NULL},           //CDx3B
        {"SRL H", NULL, H, NULL},           //CDx3C
        {"SRL L", NULL, L, NULL},           //CDx3D
        {"SRL HL", NULL, HL, NULL},         //CDx3E
        {"SRL A", NULL, A, NULL},           //CDx3F

        {"BIT 0 B", NULL, 0, B},            //CDx40
        {"BIT 0 C", NULL, 0, C},            //CDx41
        {"BIT 0 D", NULL, 0, D},            //CDx42
        {"BIT 0 E", NULL, 0, E},            //CDx43
        {"BIT 0 H", NULL, 0, H},            //CDx44
        {"BIT 0 L", NULL, 0, L},            //CDx45
        {"BIT 0 HL", NULL, 0, HL},          //CDx46
        {"BIT 0 A", NULL, 0, A},            //CDx47
        {"BIT 1 B", NULL, 1, B},            //CDx48
        {"BIT 1 C", NULL, 1, C},            //CDx49
        {"BIT 1 D", NULL, 1, D},            //CDx4A
        {"BIT 1 E", NULL, 1, E},            //CDx4B
        {"BIT 1 H", NULL, 1, H},            //CDx4C
        {"BIT 1 L", NULL, 1, L},            //CDx4D
        {"BIT 1 HL", NULL, 1, HL},          //CDx4E
        {"BIT 1 A", NULL, 1, A},            //CDx4F

        {"BIT 2 B", NULL, 2, B},            //CDx50
        {"BIT 2 C", NULL, 2, C},            //CDx51
        {"BIT 2 D", NULL, 2, D},            //CDx52
        {"BIT 2 E", NULL, 2, E},            //CDx53
        {"BIT 2 H", NULL, 2, H},            //CDx54
        {"BIT 2 L", NULL, 2, L},            //CDx55
        {"BIT 2 HL", NULL, 2, HL},          //CDx56
        {"BIT 2 A", NULL, 2, A},            //CDx57
        {"BIT 3 B", NULL, 3, B},            //CDx58
        {"BIT 3 C", NULL, 3, C},            //CDx59
        {"BIT 3 D", NULL, 3, D},            //CDx5A
        {"BIT 3 E", NULL, 3, E},            //CDx5B
        {"BIT 3 H", NULL, 3, H},            //CDx5C
        {"BIT 3 L", NULL, 3, L},            //CDx5D
        {"BIT 3 HL", NULL, 3, HL},          //CDx5E
        {"BIT 3 A", NULL, 3, A},            //CDx5F

        {"BIT 4 B", NULL, 4, B},            //CDx60
        {"BIT 4 C", NULL, 4, C},            //CDx61
        {"BIT 4 D", NULL, 4, D},            //CDx62
        {"BIT 4 E", NULL, 4, E},            //CDx63
        {"BIT 4 H", NULL, 4, H},            //CDx64
        {"BIT 4 L", NULL, 4, L},            //CDx65
        {"BIT 4 HL", NULL, 4, HL},          //CDx66
        {"BIT 4 A", NULL, 4, A},            //CDx67
        {"BIT 5 B", NULL, 5, B},            //CDx68
        {"BIT 5 C", NULL, 5, C},            //CDx69
        {"BIT 5 D", NULL, 5, D},            //CDx6A
        {"BIT 5 E", NULL, 5, E},            //CDx6B
        {"BIT 5 H", NULL, 5, H},            //CDx6C
        {"BIT 5 L", NULL, 5, L},            //CDx6D
        {"BIT 5 HL", NULL, 5, HL},          //CDx6E
        {"BIT 5 A", NULL, 5, A},            //CDx6F

        {"BIT 6 B", NULL, 6, B},            //CDx70
        {"BIT 6 C", NULL, 6, C},            //CDx71
        {"BIT 6 D", NULL, 6, D},            //CDx72
        {"BIT 6 E", NULL, 6, E},            //CDx73
        {"BIT 6 H", NULL, 6, H},            //CDx74
        {"BIT 6 L", NULL, 6, L},            //CDx75
        {"BIT 6 HL", NULL, 6, HL},          //CDx76
        {"BIT 6 A", NULL, 6, A},            //CDx77
        {"BIT 7 B", NULL, 7, B},            //CDx78
        {"BIT 7 C", NULL, 7, C},            //CDx79
        {"BIT 7 D", NULL, 7, D},            //CDx7A
        {"BIT 7 E", NULL, 7, E},            //CDx7B
        {"BIT 7 H", NULL, 7, H},            //CDx7C
        {"BIT 7 L", NULL, 7, L},            //CDx7D
        {"BIT 7 HL", NULL, 7, HL},          //CDx7E
        {"BIT 7 A", NULL, 7, A},            //CDx7F

        {"RES 0 B", NULL, 0, B},            //CDx80
        {"RES 0 C", NULL, 0, C},            //CDx81
        {"RES 0 D", NULL, 0, D},            //CDx82
        {"RES 0 E", NULL, 0, E},            //CDx83
        {"RES 0 H", NULL, 0, H},            //CDx84
        {"RES 0 L", NULL, 0, L},            //CDx85
        {"RES 0 HL", NULL, 0, HL},          //CDx86
        {"RES 0 A", NULL, 0, A},            //CDx87
        {"RES 1 B", NULL, 1, B},            //CDx88
        {"RES 1 C", NULL, 1, C},            //CDx89
        {"RES 1 D", NULL, 1, D},            //CDx8A
        {"RES 1 E", NULL, 1, E},            //CDx8B
        {"RES 1 H", NULL, 1, H},            //CDx8C
        {"RES 1 L", NULL, 1, L},            //CDx8D
        {"RES 1 HL", NULL, 1, HL},          //CDx8E
        {"RES 1 A", NULL, 1, A},            //CDx8F

        {"RES 2 B", NULL, 2, B},            //CDx90
        {"RES 2 C", NULL, 2, C},            //CDx91
        {"RES 2 D", NULL, 2, D},            //CDx92
        {"RES 2 E", NULL, 2, E},            //CDx93
        {"RES 2 H", NULL, 2, H},            //CDx94
        {"RES 2 L", NULL, 2, L},            //CDx95
        {"RES 2 HL", NULL, 2, HL},          //CDx96
        {"RES 2 A", NULL, 2, A},            //CDx97
        {"RES 3 B", NULL, 3, B},            //CDx98
        {"RES 3 C", NULL, 3, C},            //CDx99
        {"RES 3 D", NULL, 3, D},            //CDx9A
        {"RES 3 E", NULL, 3, E},            //CDx9B
        {"RES 3 H", NULL, 3, H},            //CDx9C
        {"RES 3 L", NULL, 3, L},            //CDx9D
        {"RES 3 HL", NULL, 3, HL},          //CDx9E
        {"RES 3 A", NULL, 3, A},            //CDx9F

        






};


static INSTR Opcodes[] = {
	{"NOP", NULL, NULL, NULL},
	{"LD BC,nn", NULL, "BC", NULL},
	{"LD "}
};