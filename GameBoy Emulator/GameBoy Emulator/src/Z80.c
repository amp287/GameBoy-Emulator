#include <stdio.h>
#include "Z80.h"
#include "Memory.h"
#include "PPU.h"
#include "Debug.h"
#include "Interrupts.h"

typedef struct INSTRUCTION_REGISTER {
	int instruction_index;
	unsigned char is_cb;
	OPCODE_OPERATION execute;
	unsigned short first_param;
	unsigned short second_param;
}INSTRUCTION_REGISTER;

INSTRUCTION_REGISTER ir;

CPU cpu;

int print = 0;
int instr_count = 0;
int COUNTS = 0;

void cpu_fire_interrupt(unsigned short addr) {
	cpu.sp -= 2;
	write_16_bit(cpu.sp, cpu.pc);
	cpu.pc = addr;
}

void cpu_unhalt() {
	cpu.halt = 0;
}

unsigned char cpu_halt_status() {
	return cpu.halt;
}

void cpu_init(int show_bios) {
	load_bios();
	cpu_reset(show_bios);
}

void cpu_print_reg_stack() {
	debug_log("\t\tOpcode:%02x Registers: AF:%04x BC:%04x DE:%04x\n\t\tHL:%x\n", ir.instruction_index, cpu.af, cpu.bc, cpu.de, cpu.hl);
	debug_log("\t\tStack:%x", cpu.sp);
	//debug_log("\t\tStack:%x instr_count:%d cycles:%ld", cpu.sp, instr_count, cpu.clock_t);
	//debug_log("\t\tStack:%x instr_count:%d cycles:%ld LY:%d", cpu.sp, instr_count, cpu.clock_t, read_8_bit(LCD_SCANLINE));
	debug_log("\n\n");
}

int cpu_gpu_step(int cycles) {
	int cycles_before_exe;
	cpu.t = cycles;
	cpu.m = cycles / 4;

	debug_log("pc:%04x", cpu.pc);


	if (!cpu.halt) {
		cpu.t += cpu_fetch();

		cycles_before_exe = cpu.t;

		gpu_update(cpu.t);

		if (cpu_execute())
			return -1;

		// in case of jump or execution changes something (lcdc)
		gpu_update(cpu.t - cycles_before_exe);

		cpu.m = cpu.t / 4;
	} else {
		cpu.t += 4;
		cpu.m = cpu.t / 4;
	}

	cpu.clock_t += cpu.t;
	cpu.clock_m += cpu.m;

	debug_log(" af:%04x bc:%04x de:%04x hl:%04x step:%d ", cpu.af, cpu.bc, cpu.de, cpu.hl, instr_count++);
	debug_log("ly:%d PPU ticks:%d PPU mode: %d\n", ppu_scanline, 456 - ppu_ticks, ppu_mode);

	//cpu_print_reg_stack();

	return cpu.t;
}

// Fetches next instruction and places in the Instruction Register (ir)
long cpu_fetch() {
	int index = read_8_bit(cpu.pc++);
	INSTR *map;
	
	cpu.t = 0;

	if (index == 0xCB) {
		map = opcodesCB;
		ir.is_cb = 1;
		index = read_8_bit(cpu.pc++);
	}
	else {
		map = opcodes;
		ir.is_cb = 0;
	}

	ir.instruction_index = index;

	if (map[index].r2 == READ_8) {
		ir.second_param = read_8_bit(cpu.pc++);
	} else if (map[index].r2 == READ_16) {
		ir.second_param = read_16_bit(cpu.pc); 
		cpu.pc += 2;
	} else {
		ir.second_param = map[index].r2;
	}

	ir.first_param = map[index].r1;

	ir.execute = map[index].execute;
	
	return map[index].cycles;
}

int cpu_execute() {

	if (ir.execute == NULL) {
		printf("\t\tcpu_execute: Error unimplemented opcode [%s]\n", ir.is_cb ? opcodesCB[ir.instruction_index].disassembly : opcodes[ir.instruction_index].disassembly);
		getchar();
		return -1;
	}
	
		//printf("\t\tcpu_execute: [%s] ", ir.is_cb ? opcodesCB[ir.instruction_index].disassembly : opcodes[ir.instruction_index].disassembly);
		//printf("%x %x \n", ir.first_param, ir.second_param);

	(ir.execute)(ir.first_param, ir.second_param);

	return 0;
}

void cpu_reset(int show_bios) {
	cpu.a = 0x01;
	cpu.f = 0xB0;
	cpu.b = 0x00;
	cpu.c = 0x13;
	cpu.d = 0x00;
	cpu.e = 0xD8;
	cpu.h = 0x01;
	cpu.l = 0x4D;
	cpu.sp = 0xFFFE;
	if (show_bios) {
		cpu.pc = 0x0;
	} else {
		cpu.pc = 0x100;
		write_8_bit(0xFF50, 1); //turn bios map off
	}
		
	cpu.clock_m = 0;
	cpu.clock_t = 0;
	reset_master_interrupt(0);

	write_8_bit(0xFF05, 0);
	write_8_bit(0xFF06, 0);
	write_8_bit(0xFF07, 0);
	write_8_bit(0xFF10, 0x80);
	write_8_bit(0xFF11, 0xBF);
	write_8_bit(0xFF12, 0xF3);
	write_8_bit(0xFF14, 0xBF);
	write_8_bit(0xFF16, 0x3F);
	write_8_bit(0xFF17, 0x00);
	write_8_bit(0xFF19, 0xBF);
	write_8_bit(0xFF1A, 0x7F);
	write_8_bit(0xFF1B, 0xFF);
	write_8_bit(0xFF1C, 0x9F);
	write_8_bit(0xFF1E, 0xBF);
	write_8_bit(0xFF20, 0xFF);
	write_8_bit(0xFF21, 0x00);
	write_8_bit(0xFF22, 0x00);
	write_8_bit(0xFF23, 0xBF);
	write_8_bit(0xFF24, 0x77);
	write_8_bit(0xFF25, 0xF3);
	write_8_bit(0xFF26, 0xF1);
	write_8_bit(0xFF40, 0x91);
	write_8_bit(0xFF42, 0x00);
	write_8_bit(0xFF43, 0x00);
	write_8_bit(0xFF44, 0x00);
	write_8_bit(0xFF45, 0x00);
	write_8_bit(0xFF47, 0xFC);
	write_8_bit(0xFF48, 0xFF);
	write_8_bit(0xFF49, 0xFF);
	write_8_bit(0xFF4A, 0x00);
	write_8_bit(0xFF4B, 0x00);
	write_8_bit(0xFFFF, 0x00);
}

void clear_flag(unsigned char flag) {
	unsigned char comp = ~flag;
	comp = comp & 0xF0;
	cpu.f = cpu.f & comp;
}

void set_flag(unsigned char flag) {
	cpu.f = cpu.f | flag;
}

unsigned char* get_register(unsigned short reg) {
	switch (reg) {
		case A:
			return &cpu.a;
		case B:
			return &cpu.b;
		case C:
			return &cpu.c;
		case D:
			return &cpu.d;
		case E:
			return &cpu.e;
		case H:
			return &cpu.h;
		case L:
			return &cpu.l;
		default:
			return NULL;
	}
}

//Opcodes

//8 bit Loads

void LD_nn_n(unsigned short r1, unsigned short immediate) {
	*get_register(r1) = (unsigned char)immediate;
}

void LD_r1_r2(unsigned short r1, unsigned short r2) {
	if (r2 == HL) {
		*get_register(r1) = read_8_bit(cpu.hl);
	} else if (r1 == HL) {
		write_8_bit(cpu.hl, *get_register(r2));
	} else {
		*get_register(r1) = *get_register(r2);
	}
}

void LD_HL_n(unsigned short hl, unsigned short n) {
	write_8_bit(cpu.hl, (unsigned char)n);
}

void LD_A_n(unsigned short type, unsigned short n) {

	switch (n) {
		case BC:
			cpu.a = read_8_bit(cpu.bc);
			break;
		case DE:
			cpu.a = read_8_bit(cpu.de);
			break;
		case HL:
			cpu.a = read_8_bit(cpu.hl);
			break;
		default:
			cpu.a = *get_register(n);
	}
}

void LD_A_nn(unsigned short a, unsigned short nn) {
	cpu.a = read_8_bit(nn);
}

void LD_A_imm(unsigned short a, unsigned short immediate) {
	cpu.a = (unsigned char)immediate;
}

void LD_n_A(unsigned short type, unsigned short n) {

	if (type == READ_16) {
		write_8_bit(n, cpu.a);
		return;
	}

	switch (n) {
		case BC:
			write_8_bit(cpu.bc, cpu.a);
			break;
		case DE:
			write_8_bit(cpu.de, cpu.a);
			break;
		case HL:
			write_8_bit(cpu.hl, cpu.a);
			break;
		default:
			*get_register(n) = cpu.a; 
	}
}

void LD_nn_A(unsigned short nn, unsigned short A) {
	write_8_bit(nn, cpu.a);
}

void LD_A_C(unsigned short A, unsigned short C) {
	cpu.a = read_8_bit(0xFF00 + cpu.c);
}

void LD_C_A(unsigned short C, unsigned short A) {
	write_8_bit(cpu.c + 0xFF00, cpu.a);
}

void LD_A_HLD(unsigned short A, unsigned short HL) {
	cpu.a = read_8_bit(cpu.hl--);
}

void LD_HLD_A(unsigned short HL, unsigned short A) {
	write_8_bit(cpu.hl--, cpu.a);
}

void LD_A_HLI(unsigned short A, unsigned short HL) {
	cpu.a = read_8_bit(cpu.hl++);
}

void LD_HLI_A(unsigned short hl , unsigned short a) {
	write_8_bit(cpu.hl++, cpu.a);
}

void LDH_n_A(unsigned short NA, unsigned short n) {
	write_8_bit(0xFF00 + n, cpu.a);
}

void LDH_A_n(unsigned short A, unsigned short n) {

	cpu.a = read_8_bit(0xFF00 + n);
}

//16-Bit Loads
void LD_n_nn(unsigned short n, unsigned short nn) {
	switch (n) {
		case BC:
			cpu.bc = nn;
			break;
		case DE:
			cpu.de = nn;
			break;
		case HL:
			cpu.hl = nn;
			break;
		case SP:
			cpu.sp = nn;
	}
}

void LD_SP_HL(unsigned short sp, unsigned short hl) {
	cpu.sp = cpu.hl;
}

void LDHL_SP_n(unsigned short sp, unsigned short n) {

	if ((cpu.sp & 0x0FF) + (n & 0x0FF) & 0x100) 
		set_flag(CARRY_FLAG);
	else 
		clear_flag(CARRY_FLAG);

	if (((cpu.sp & 0x0f) + (n & 0x0f)) & 0x10) set_flag(HALF_CARRY_FLAG);
	else clear_flag(HALF_CARRY_FLAG);

	clear_flag(ZERO_FLAG | SUBTRACT_FLAG);

	cpu.hl = cpu.sp + (char)n;
}

void LD_nn_SP(unsigned short sp, unsigned short nn) {
	write_16_bit(nn, cpu.sp);
}

void PUSH_nn(unsigned short nn, unsigned short NA) {
	unsigned short val;

	cpu.sp -= 2;

	switch (nn) {
		case AF:
			val = cpu.af & (0xFFF0);
			break;
		case BC:
			val = cpu.bc;
			break;
		case DE:
			val = cpu.de;
			break;
		case HL:
			val = cpu.hl;
		}

	write_16_bit(cpu.sp, val);
}

void POP_nn(unsigned short nn, unsigned short NA) {
	unsigned short stack_val = read_16_bit(cpu.sp);
	cpu.sp += 2;

	switch (nn) {
		case AF:
			//f should only store top 4 bits
			cpu.af = stack_val & 0xFFF0;
			break;
		case BC:
			cpu.bc = stack_val;
			break;
		case DE:
			cpu.de = stack_val;
			break;
		case HL:
			cpu.hl = stack_val;
	}
}

// 8 bit ALU

void ADD_A_n(unsigned short type, unsigned short n) {
	// check if n is immediate
	if (type == READ_8)
		;
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);

	clear_flag(SUBTRACT_FLAG);

	unsigned int result = cpu.a + n;

	if (result & 0xFF00)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if (((cpu.a & 0x0F) + (n & 0x0F)) & 0x10)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);

	cpu.a = cpu.a + n;

	if (cpu.a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void ADC_A_n(unsigned short type, unsigned short n) {
	unsigned char carry;

	if (type == READ_8)
		; // n is an immediate 
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);

	carry = cpu.f & CARRY_FLAG ? 1 : 0;

	if (((cpu.a & 0x0F) + ((n + carry) & 0x0F)) & 0x10)
		set_flag(HALF_CARRY_FLAG);
	else if (((n & 0x0F) + carry) & 0x10)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);

	unsigned int result = cpu.a + n + carry;

	if (result & 0xFF00)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	cpu.a = cpu.a + n + carry;

	if (cpu.a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	clear_flag(SUBTRACT_FLAG);

}

void SUB_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);

	set_flag(SUBTRACT_FLAG);

	if (n > cpu.a)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if ((n & 0x0f) > (cpu.a & 0x0f)) 
		set_flag(HALF_CARRY_FLAG);
	else 
		clear_flag(HALF_CARRY_FLAG);

	cpu.a -= n;

	if (cpu.a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void SBC_A_n(unsigned short type, unsigned short n) {
	unsigned char flag;
	unsigned short res;
	if (type == READ_8)
		;
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);
	
	flag = cpu.f & CARRY_FLAG ? 1 : 0;

	set_flag(SUBTRACT_FLAG);

	res = cpu.a - n;

	if (cpu.a < n || cpu.a - n < flag)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if ((cpu.a & 0x0F) < (n & 0x0F))
		set_flag(HALF_CARRY_FLAG);
	else if (((cpu.a - n) & 0x0F) < flag)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);

	cpu.a = cpu.a - n - flag;

	if (cpu.a == 0)
		set_flag(ZERO_FLAG);
	else
		clear_flag(ZERO_FLAG);
}

void AND_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);

	clear_flag(SUBTRACT_FLAG | CARRY_FLAG);

	set_flag(HALF_CARRY_FLAG);

	cpu.a &= n;

	if (cpu.a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void OR_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | CARRY_FLAG);

	cpu.a |= n;

	if (cpu.a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void XOR_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | CARRY_FLAG);

	cpu.a ^= n;

	if (cpu.a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void CP_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
		n = read_8_bit(cpu.hl);
	else
		n = *get_register(n);

	set_flag(SUBTRACT_FLAG);

	if (n > cpu.a)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if ((n & 0x0f) > (cpu.a & 0x0f))
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);

	if (cpu.a == n)
		set_flag(ZERO_FLAG);
	else
		clear_flag(ZERO_FLAG);
}

void INC_n(unsigned short type, unsigned short n) {
	unsigned char val;

	if (type == READ_8)
		;
	else if (n == HL)
		val = read_8_bit(cpu.hl);
	else
		val = *get_register(n);

	if ((val &0x0f) == 0x0f)
		set_flag(HALF_CARRY_FLAG);
	else 
		clear_flag(HALF_CARRY_FLAG);

	val++;

	if (val)
		clear_flag(ZERO_FLAG);
	else 
		set_flag(ZERO_FLAG);

	clear_flag(SUBTRACT_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, val);
	else
		*get_register(n) = val;

}

void DEC_n(unsigned short NA, unsigned short n) {
	unsigned char val;

	if (n == HL)
		val = read_8_bit(cpu.hl);
	else
		val = *get_register(n);

	set_flag(SUBTRACT_FLAG);

	if (val & 0x0f)
		clear_flag(HALF_CARRY_FLAG);
	else 
		set_flag(HALF_CARRY_FLAG);

	val--;

	if (val)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
	
	if (n == HL)
		write_8_bit(cpu.hl, val);
	else
		*get_register(n) = val;
}

//16-Bit Arithmetic
void ADD_HL_n(unsigned short NA, unsigned short n) {
		switch (n) {
			case AF:
				// f bottom bits shouldnt be changed
				n = cpu.af & 0xFFF0;
				break;
			case BC:
				n = cpu.bc;
				break;
			case DE:
				n = cpu.de;
				break;
			case HL:
				n = cpu.hl;
				break;
			case SP:
				n = cpu.sp;
		}

		clear_flag(SUBTRACT_FLAG);

		unsigned long res = cpu.hl + n;

		if (res & 0xFFFF0000)
			set_flag(CARRY_FLAG);
		else
			clear_flag(CARRY_FLAG);

		if (((cpu.hl & 0x0FFF) + (n & 0x0FFF)) & 0x1000)
			set_flag(HALF_CARRY_FLAG);
		else
			clear_flag(HALF_CARRY_FLAG);

		cpu.hl = (unsigned short)res;
}

void ADD_SP_n(unsigned short sp, unsigned short n) {
	clear_flag(SUBTRACT_FLAG | ZERO_FLAG);

	if ((cpu.sp & 0x0FF) + (n & 0x0FF) & 0x100)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if (((cpu.sp & 0x0F) + ((char)n & 0x0F)) & 0x10)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);

	cpu.sp = cpu.sp + (char)n;
}

void INC_nn(unsigned short nn, unsigned short NA) {
	switch (nn) {
		case BC:
			cpu.bc++;
			break;
		case DE:
			cpu.de++;
			break;
		case HL:
			cpu.hl++;
			break;
		case SP:
			cpu.sp++;
	}
}

void DEC_nn(unsigned short nn, unsigned short NA) {
	switch (nn) {
		case BC:
			cpu.bc--;
			break;
		case DE:
			cpu.de--;
			break;
		case HL:
			cpu.hl--;
			break;
		case SP:
			cpu.sp--;
	}
}

//Miscellaneous
void SWAP_n(unsigned short n, unsigned short NA) {
	unsigned char upper, lower;
	unsigned val;

	if (n == HL)
		val = read_8_bit(cpu.hl);
	else
		val = *get_register(n);

	upper = 0xF0 & val;
	lower = 0x0F & val;

	val = lower << 4;
	val |= upper >> 4;

	if (val)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | CARRY_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, val);
	else
		*get_register(n) = val;
}

void DAA(unsigned short NA_1, unsigned short NA_2) {
	unsigned short s = cpu.a;

	if (cpu.f & SUBTRACT_FLAG) {
		if (cpu.f & HALF_CARRY_FLAG) 
			s = (s - 0x06) & 0xFF;
		if (cpu.f & CARRY_FLAG) 
			s -= 0x60;
	} else {
		if ((cpu.f & HALF_CARRY_FLAG) || (s & 0xF) > 9) 
			s += 0x06;
		if ((cpu.f & CARRY_FLAG) || s > 0x9F) 
			s += 0x60;
	}

	cpu.a = (unsigned char)s;
	clear_flag(HALF_CARRY_FLAG);

	if (cpu.a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (s >= 0x100)
		set_flag(CARRY_FLAG);
}

void CPL(unsigned short NA_1, unsigned short NA_2) {
	cpu.a = ~cpu.a;

	set_flag(HALF_CARRY_FLAG | SUBTRACT_FLAG);
}

void CCF(unsigned short NA_1, unsigned short NA_2) {
	if (cpu.f & CARRY_FLAG)
		clear_flag(CARRY_FLAG);
	else
		set_flag(CARRY_FLAG);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void SCF(unsigned short NA_1, unsigned short NA_2) {
	set_flag(CARRY_FLAG);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void NOP(unsigned short NA_1, unsigned short NA_2) {

}

//Power down (Stop) CPU until interrupt occurs
void HALT(unsigned short NA_1, unsigned short NA_2) {
	cpu.halt = 1;
}

//Halt CPU & LCD display until button pressed
void STOP(unsigned short NA_1, unsigned short NA_2) {
	printf("STOP command (Dont know how to implement yet)\n");
}

//This instruction disables interrupts but not
//immediately.Interrupts are disabled after
//instruction after DI is executed.
void DI(unsigned short NA_1, unsigned short NA_2) {
	reset_master_interrupt(0);
}

//Enable interrupts. This intruction enables interrupts
//but not immediately.Interrupts are enabled after
//instruction after EI is executed.
void EI(unsigned short NA_1, unsigned short NA_2) {
	set_master_interrupt(0);
}

//Rotates & Shifts

void RLCA(unsigned short A, unsigned short NA) {
	unsigned char carry = (cpu.a & 0x80) >> 7;
	if (carry)
		set_flag(CARRY_FLAG);
	 else
		clear_flag(CARRY_FLAG);

	cpu.a <<= 1;
	cpu.a &= 0xFE;
	cpu.a |= carry;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | ZERO_FLAG);
}

//Followed Cinoop implementation for following rotations and shifts
void RLA(unsigned short A, unsigned short NA) {
	int carry = cpu.f & CARRY_FLAG ? 1 : 0;

	if (cpu.a & 0x80) set_flag(CARRY_FLAG);
	else clear_flag(CARRY_FLAG);

	cpu.a <<= 1;
	cpu.a += carry;

	clear_flag(SUBTRACT_FLAG | ZERO_FLAG | HALF_CARRY_FLAG);
}

void RRCA(unsigned short A, unsigned short NA) {
	unsigned char carry = cpu.a & 0x01;
	if (carry) set_flag(CARRY_FLAG);
	else clear_flag(CARRY_FLAG);

	cpu.a >>= 1;
	if (carry) cpu.a |= ZERO_FLAG;

	clear_flag(SUBTRACT_FLAG | ZERO_FLAG | HALF_CARRY_FLAG);
}

void RRA(unsigned short A, unsigned short NA) {
	int carry = (cpu.f & CARRY_FLAG ? 1 : 0) << 7;

	if (cpu.a & 0x01) set_flag(CARRY_FLAG);
	else clear_flag(CARRY_FLAG);

	cpu.a >>= 1;
	cpu.a += carry;

	clear_flag(SUBTRACT_FLAG | ZERO_FLAG | HALF_CARRY_FLAG);
}

void RLC_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char carry;

	if(n == HL)
		num = read_8_bit(cpu.hl);
	else
		num = *get_register(n);

	carry = (num & 0x80) >> 7;
	
	if (num & 0x80)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	num <<= 1;
	num += carry;

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, num);
	else
		*get_register(n) = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void RL_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char carry;

	if (n == HL)
		num = read_8_bit(cpu.hl);
	else
		num = *get_register(n);

	carry = cpu.f & CARRY_FLAG ? 1 : 0;

	if (num & 0x80)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	num <<= 1;
	num += carry;

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, num);
	else
		*get_register(n) = num;
}

void RRC_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char carry;

	if (n == HL)
		num = read_8_bit(cpu.hl);
	else
		num = *get_register(n);

	carry = num & 0x01;
	num >>= 1;

	if (carry) {
		set_flag(CARRY_FLAG);
		num |= 0x80;
	} else {
		clear_flag(CARRY_FLAG);
	}

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, num);
	else
		*get_register(n) = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void RR_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char old;

	if (n == HL)
		num = read_8_bit(cpu.hl);
	else
		num = *get_register(n);

	old = num & 1;

	num >>= 1;

	if (cpu.f & CARRY_FLAG)
		num |= 0x80;

	if (old)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, num);
	else
		*get_register(n) = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void SLA_n(unsigned short n, unsigned short NA) {
	unsigned char num;

	if (n == HL)
		num = read_8_bit(cpu.hl);
	else
		num = *get_register(n);

	if (num & 0x80)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	num <<= 1;

	if (num == 0)
		set_flag(ZERO_FLAG);
	else
		clear_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, num);
	else
		*get_register(n) = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void SRA_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char shift_val;

	if (n == HL)
		num = read_8_bit(cpu.hl);
	else
		num = *get_register(n);
	
	if (num & 0x01)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);
	
	// Shift val is used to store the sign bit
	// for arithmetic shifts
	shift_val = num & 0x80;

	num = (num >> 1) | shift_val;

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, num);
	else
		*get_register(n) = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void SRL_n(unsigned short n, unsigned short NA) {
	unsigned char num;

	if (n == HL)
		num = read_8_bit(cpu.hl);
	else
		num = *get_register(n);

	if (num & 0x01)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	num >>= 1;

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu.hl, num);
	else
		*get_register(n) = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

//Bit Opcodes

void BIT_b_r(unsigned short b, unsigned short r) {
	unsigned char bit_test = 1 << b;
	unsigned int res;
	
	if (r == HL) {
		res = read_8_bit(cpu.hl) & bit_test;
	} else {
		res = *get_register(r) & bit_test;
	}

	if (res)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	clear_flag(SUBTRACT_FLAG);
	set_flag(HALF_CARRY_FLAG);
}

void SET_b_r(unsigned short b, unsigned short r) {
	unsigned char bit_test = 1 << b;
	unsigned int res;

	if (r == HL) {
		res = read_8_bit(cpu.hl) | bit_test;
		write_8_bit(cpu.hl, res);
	}
	else {
		*get_register(r) |= bit_test;
	}
}

void RES_b_r(unsigned short b, unsigned short r) {
	unsigned char bit_test = ~(1 << b);
	unsigned int res;

	if (r == HL) {
		res = read_8_bit(cpu.hl) & bit_test;
		write_8_bit(cpu.hl, res);
	}
	else {
		*get_register(r) &= bit_test;
	}
}

//Jumps
void JP_nn(unsigned short NA, unsigned short nn) {
	cpu.pc = nn;
}

void JP_cc_nn(unsigned short cc, unsigned short nn) {
	char jump = 0;

	switch (cc) {
		case NZ:
			jump = !(cpu.f & ZERO_FLAG);
			break;
		case Z:
			jump = cpu.f & ZERO_FLAG;
			break;
		case NC:
			jump = !(cpu.f & CARRY_FLAG);
			break;
		case C:
			jump = cpu.f & CARRY_FLAG;
			break;
		default:
			//Error here
			;
	}

	if (jump) {
		cpu.pc = nn;
		cpu.t += 4;
	}
}

void JP_HL(unsigned short NA_1, unsigned short NA_2) {
	cpu.pc = cpu.hl;
}

void JR_n(unsigned short NA, unsigned short n) {
	cpu.pc += (signed char)n;
}

void JR_cc_n(unsigned short cc, unsigned short n) {
	char jump = 0;

	switch (cc) {
	case NZ:
		jump = !(cpu.f & ZERO_FLAG);
		break;
	case Z:
		jump = cpu.f & ZERO_FLAG;
		break;
	case NC:
		jump = !(cpu.f & CARRY_FLAG);
		break;
	case C:
		jump = cpu.f & CARRY_FLAG;
		break;
	default:
		//Error here
		;
	}

	if (jump){
		cpu.pc += (signed char)n;
		cpu.t += 4;
	}
}

//Calls

void CALL_nn(unsigned short NA, unsigned short nn) {
	cpu.sp -= 2;
	write_16_bit(cpu.sp, cpu.pc); // Put next instruction on stack
	cpu.pc = nn;
}

void CALL_cc_nn(unsigned short cc, unsigned short nn) {
	char jump = 0;

	switch (cc) {
	case NZ:
		jump = !(cpu.f & ZERO_FLAG);
		break;
	case Z:
		jump = cpu.f & ZERO_FLAG;
		break;
	case NC:
		jump = !(cpu.f & CARRY_FLAG);
		break;
	case C:
		jump = cpu.f & CARRY_FLAG;
		break;
	default:
		//Error here
		;
	}
	if (jump) {
		cpu.t += 12;
		CALL_nn(0, nn);
	}
	
}

//Restarts 

//n = 0x0, 0x8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 
void RST_n(unsigned short n, unsigned short NA) {
	cpu.sp -= 2;
	write_16_bit(cpu.sp, cpu.pc);
	cpu.pc = n;
}

//Returns 
void RET(unsigned short NA_1, unsigned short NA_2) {
	cpu.pc = read_16_bit(cpu.sp);
	cpu.sp += 2;
}

void RET_cc(unsigned short cc, unsigned short NA) {
	char ret = 0;

	switch (cc) {
	case NZ:
		ret = !(cpu.f & ZERO_FLAG);
		break;
	case Z:
		ret = cpu.f & ZERO_FLAG;
		break;
	case NC:
		ret = !(cpu.f & CARRY_FLAG);
		break;
	case C:
		ret = cpu.f & CARRY_FLAG;
		break;
	default:
		//Error here
		;
	}

	if (ret) {
		RET(0, 0);
		cpu.t += 12;
	}
		
}

void RETI(unsigned short NA_1, unsigned short NA_2) {
	set_master_interrupt(0);
	cpu.pc = read_16_bit(cpu.sp);
	cpu.sp += 2;
}