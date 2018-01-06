#include "Z80.h"
#include "Memory.h"
#include "stdio.h"

CPU *cpu;

unsigned char *reg_pointers[11];

void cpu_init() {
	cpu = malloc(sizeof(CPU));

	reg_pointers[0] = &cpu->a;
	reg_pointers[1] = &cpu->b;
	reg_pointers[2] = &cpu->c;
	reg_pointers[3] = &cpu->d;
	reg_pointers[4] = &cpu->e;
	reg_pointers[5] = &cpu->h;
	reg_pointers[6] = &cpu->l;
	reg_pointers[7] = &cpu->af;
	reg_pointers[8] = &cpu->bc;
	reg_pointers[9] = &cpu->de;
	reg_pointers[10] = &cpu->hl;

	cpu_reset();
}

int cpu_fetch() {

	return 0;
}

int cpu_execute() {
	*reg_pointers[HL] = 22;
	*reg_pointers[A] = 'b';
	LD_r1_r2(HL, A);

	unsigned char ans = read_8_bit(22);

	printf("Memory[22]: %c\n", ans);
	return 0;
}

void cpu_reset() {
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

void clear_flag(unsigned char flag) {
	unsigned char comp = ~flag;
	comp = comp & 0xF0;
	cpu->f = cpu->f & comp;
}

void set_flag(unsigned char flag) {
	cpu->f = cpu->f | flag;
}

//Opcodes

//8 bit Loads

void LD_nn_n(unsigned short r1, unsigned short immediate) {
	*reg_pointers[r1] = (unsigned char)immediate;
}

void LD_r1_r2(unsigned short r1, unsigned short r2) {
	if (r2 > L) {
		*reg_pointers[r1] = read_8_bit(*(unsigned short*)reg_pointers[r2]);
	} else if (r1 > L) {
		write_8_bit(*(unsigned short*)reg_pointers[r1], *reg_pointers[r2]);
	} else {
		*reg_pointers[r1] = *reg_pointers[r2];
	}
}

void LD_A_n(unsigned short a, unsigned short r2) {
	switch (r2) {
		case BC:
			cpu->a = read_8_bit(cpu->bc);
			break;
		case DE:
			cpu->a = read_8_bit(cpu->de);
			break;
		case HL:
			cpu->a = read_8_bit(cpu->hl);
			break;
		default:
			cpu->a = *reg_pointers[r2];
	}
}

void LD_A_nn(unsigned short a, unsigned short nn) {
	cpu->a = read_8_bit(nn);
}

void LD_A_imm(unsigned short a, unsigned short immediate) {
	cpu->a = immediate;
}

void LD_n_A(unsigned short n, unsigned short a) {
	switch (n) {
		case BC:
			write_8_bit(cpu->bc, cpu->a);
			break;
		case DE:
			write_8_bit(cpu->de, cpu->a);
			break;
		case HL:
			write_8_bit(cpu->hl, cpu->a);
			break;
		default:
			*reg_pointers[n] = cpu->a; 
	}
}

void LD_nn_A(unsigned short nn, unsigned short A) {
	write_8_bit(nn, cpu->a);
}

void LD_A_C(unsigned short A, unsigned short C) {
	cpu->a = read_8_bit(0xFF00 + cpu->c);
}

void LD_C_A(unsigned short C, unsigned short A) {
	write_8_bit(cpu->c + 0xFF00, cpu->a);
}

void LD_A_HLD(unsigned short A, unsigned short HL) {
	cpu->a = read_8_bit(cpu->hl--);
}

void LD_HLD_A(unsigned short HL, unsigned short A) {
	write_8_bit(cpu->hl--, cpu->a);
}

void LD_A_HLI(unsigned short A, unsigned short HL) {
	cpu->a = read_8_bit(cpu->hl++);
}

void LD_HLI_A(unsigned short hl , unsigned short a) {
	write_8_bit(cpu->hl++, cpu->a);
}

void LDH_n_A(unsigned short n, unsigned short A) {
	write_8_bit(0xFF00 + n, cpu->a);
}

void LDH_A_n(unsigned short A, unsigned short n) {
	cpu->a = read_8_bit(0xFF00 + n);
}

//16-Bit Loads
void LD_n_nn(unsigned short n, unsigned short nn) {
	switch (n) {
		case BC:
			*reg_pointers[BC] = nn;
			break;
		case DE:
			*reg_pointers[DE] = nn;
			break;
		case HL:
			*reg_pointers[HL] = nn;
			break;
		case SP:
			cpu->sp = nn;
	}
}

void LD_SP_HL(unsigned short sp, unsigned short hl) {
	cpu->sp = cpu->hl;
}

void LDHL_SP_n(unsigned short sp, unsigned short n) {
	int result = cpu->sp + (signed char)n;

	if (result & 0xffff0000) set_flag(CARRY_FLAG);
	else clear_flag(CARRY_FLAG);

	if (((cpu->sp & 0x0f) + (n & 0x0f)) > 0x0f) set_flag(HALF_CARRY_FLAG);
	else clear_flag(HALF_CARRY_FLAG);

	clear_flag(ZERO_FLAG | SUBTRACT_FLAG);

	cpu->hl = (unsigned short)(result & 0xffff);
}

void LD_nn_SP(unsigned short nn, unsigned short sp) {
	write_16_bit(nn, cpu->sp);
}

void PUSH_nn(unsigned short nn, unsigned short NA) {
	write_16_bit(cpu->sp, nn);
	cpu->sp-=2;
}

void POP_nn(unsigned short nn, unsigned short NA) {
	unsigned short stack_val = read_16_bit(cpu->sp);
	cpu->sp += 2;

	switch (nn) {
		case AF:
			cpu->af = stack_val;
			break;
		case BC:
			cpu->bc = stack_val;
			break;
		case DE:
			cpu->de = stack_val;
			break;
		case HL:
			cpu->hl = stack_val;
	}
}

// 8 bit ALU
//when using check whether immediate can be one of the register numbers
void ADD_A_n(unsigned short a, unsigned short n) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	unsigned int result = cpu->a + n;

	if (result & 0xFF00)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	cpu->a = cpu->a + n;

	if (cpu->a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (((cpu->a & 0x0F) + (n & 0x0F)) > 0x0F)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);
}

void ADC_A_n(unsigned short a, unsigned short n) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	n += cpu->f & CARRY_FLAG ? 1 : 0;

	unsigned int result = cpu->a + n;

	if (result & 0xFF00)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	cpu->a = cpu->a + n;

	if (cpu->a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (((cpu->a & 0x0F) + (n & 0x0F)) > 0x0F)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);

}

void SUB_n(unsigned short n, unsigned short NA) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	set_flag(SUBTRACT_FLAG);

	if (n > cpu->a)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if ((n & 0x0f) > (cpu->a & 0x0f)) 
		set_flag(HALF_CARRY_FLAG);
	else 
		clear_flag(HALF_CARRY_FLAG);

	cpu->a -= n;

	if (cpu->a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void SBC_A_n(unsigned short a, unsigned short n) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	n += cpu->f & CARRY_FLAG ? 1 : 0;

	set_flag(SUBTRACT_FLAG);

	if (n > cpu->a)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if (cpu->a == 0)
		set_flag(ZERO_FLAG);
	else
		clear_flag(ZERO_FLAG);

	if (((cpu->a & 0x0F) + (n & 0x0F)) > 0x0F)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);

	cpu->a = cpu->a + n;
}

void AND_n(unsigned short n, unsigned short NA) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	clear_flag(SUBTRACT_FLAG | CARRY_FLAG);

	set_flag(HALF_CARRY_FLAG);

	cpu->a &= n;

	if (cpu->a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void OR_n(unsigned short n, unsigned short NA) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | CARRY_FLAG);

	cpu->a |= n;

	if (cpu->a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void XOR_n(unsigned short n, unsigned short NA) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | CARRY_FLAG);

	cpu->a ^= n;

	if (cpu->a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void CP_n(unsigned short n, unsigned short NA) {
	if (n == HL)
		n = read_8_bit(cpu->hl);
	else
		n = *reg_pointers[n];

	set_flag(SUBTRACT_FLAG);

	if (n > cpu->a)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if ((n & 0x0f) > (cpu->a & 0x0f))
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);


	if (cpu->a == n)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);
}

void INC_n(unsigned short n, unsigned short NA) {
	unsigned char val;

	if (n == HL)
		val = read_8_bit(cpu->hl);
	else
		val = *reg_pointers[n];

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
		write_8_bit(cpu->hl, val);
	else
		*reg_pointers[n] = val;

}

void DEC_n(unsigned short n, unsigned short NA) {
	unsigned char val;

	if (n == HL)
		val = read_8_bit(cpu->hl);
	else
		val = *reg_pointers[n];

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
		write_8_bit(cpu->hl, val);
	else
		*reg_pointers[n] = val;
}

//16-Bit Arithmetic
void ADD_HL_n(unsigned short HL, unsigned short n) {
		n = *reg_pointers[n];

		clear_flag(SUBTRACT_FLAG);

		unsigned long res = cpu->hl + n;

		if (res & 0xFFFF0000)
			set_flag(CARRY_FLAG);
		else
			clear_flag(CARRY_FLAG);

		cpu->hl = (unsigned short)res;

		if (((cpu->hl & 0x0F) + (n & 0x0F)) > 0x0F)
			set_flag(HALF_CARRY_FLAG);
		else
			clear_flag(HALF_CARRY_FLAG);
}

void ADD_SP_n(unsigned short sp, unsigned short n) {
	clear_flag(SUBTRACT_FLAG | ZERO_FLAG);

	unsigned long res = cpu->sp + n;

	if (res & 0xFFFF0000)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	cpu->sp = (unsigned short)res;

	if (((cpu->sp & 0x0F) + (n & 0x0F)) > 0x0F)
		set_flag(HALF_CARRY_FLAG);
	else
		clear_flag(HALF_CARRY_FLAG);
}

void INC_nn(unsigned short nn, unsigned short NA) {
	*reg_pointers[nn]++;
}

void DEC_nn(unsigned short nn, unsigned short NA) {
	*reg_pointers[nn]--;
}

//Miscellaneous
void SWAP_n(unsigned short n, unsigned short NA) {
	unsigned char upper, lower;

	upper = 0xF0 & *reg_pointers[n];
	lower = 0x0F & *reg_pointers[n];

	*reg_pointers[n] = lower << 4;
	*reg_pointers[n] &= upper >> 4;

	if (*reg_pointers[n])
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | CARRY_FLAG);
}

void DAA(unsigned short NA_1, unsigned short NA_2) {
	unsigned short s = cpu->a;

	if (cpu->f & SUBTRACT_FLAG) {
		if (cpu->f & HALF_CARRY_FLAG) s = (s - 0x06) & 0xFF;
		if (cpu->f & CARRY_FLAG) s -= 0x60;
	}
	else {
		if ((cpu->f & HALF_CARRY_FLAG) || (s & 0xF) > 9) s += 0x06;
		if ((cpu->f & CARRY_FLAG) || s > 0x9F) s += 0x60;
	}

	cpu->a = s;
	clear_flag(HALF_CARRY_FLAG);

	if (cpu->a) clear_flag(ZERO_FLAG);
	else set_flag(ZERO_FLAG);

	if (s >= 0x100) set_flag(CARRY_FLAG);
}

void CPL(unsigned short NA_1, unsigned short NA_2) {
	cpu->a = ~cpu->a;

	set_flag(HALF_CARRY_FLAG | SUBTRACT_FLAG);
}

void CCF(unsigned short NA_1, unsigned short NA_2) {
	if (cpu->f & CARRY_FLAG)
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
//TODO: Set up interrupts and fill in this opcode
void HALT(unsigned short NA_1, unsigned short NA_2) {

}

//Halt CPU & LCD display until button pressed
//TODO: Set up interrupts and fill in this opcode
void STOP(unsigned short NA_1, unsigned short NA_2) {

}

//This instruction disables interrupts but not
//immediately.Interrupts are disabled after
//instruction after DI is executed.
//TODO: This
void DI(unsigned short NA_1, unsigned short NA_2) {

}

//Enable interrupts. This intruction enables interrupts
//but not immediately.Interrupts are enabled after
//instruction after EI is executed.
//TODO: This
void EI(unsigned short NA_1, unsigned short NA_2) {

}

//Rotates & Shifts

//This may be wrong (different than Cinoop but should be right according to GBCPUman doc)
void RLCA(unsigned short A, unsigned short NA) {
	unsigned char carry = (cpu->a & 0x80) >> 7;
	if (carry) {
		set_flag(CARRY_FLAG);
	} else {
		clear_flag(CARRY_FLAG);
		set_flag(ZERO_FLAG);
	}
	cpu->a <<= 1;
	cpu->a += carry;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

//Followed Cinoop implementation for following rotations and shifts
void RLA(unsigned short A, unsigned short NA) {
	int carry = cpu->f & CARRY_FLAG ? 1 : 0;

	if (cpu->a & 0x80) set_flag(CARRY_FLAG);
	else clear_flag(CARRY_FLAG);

	cpu->a <<= 1;
	cpu->a += carry;

	clear_flag(SUBTRACT_FLAG | ZERO_FLAG | HALF_CARRY_FLAG);
}

void RRCA(unsigned short A, unsigned short NA) {
	unsigned char carry = cpu->a & 0x01;
	if (carry) set_flag(CARRY_FLAG);
	else clear_flag(CARRY_FLAG);

	cpu->a >>= 1;
	if (carry) cpu->a |= ZERO_FLAG;

	FLAGS_CLEAR(SUBTRACT_FLAG | ZERO_FLAG | HALF_CARRY_FLAG);
}

void RRA(unsigned short A, unsigned short NA) {
	int carry = (cpu->f & CARRY_FLAG ? 1 : 0) << 7;

	if (cpu->a & 0x01) set_flag(CARRY_FLAG);
	else clear_flag(CARRY_FLAG);

	cpu->a >>= 1;
	cpu->a += carry;

	clear_flag(SUBTRACT_FLAG | ZERO_FLAG | HALF_CARRY_FLAG);
}

void RLC_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char carry;

	if(n == HL)
		num = read_8_bit(cpu->hl);
	else
		num = *reg_pointers[n];

	carry = (num & 0x80) >> 7;
	
	if (num & 0x80)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	num << 1;
	num += carry;

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu->hl, num);
	else
		*reg_pointers[n] = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void RL_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char carry;

	if (n == HL)
		num = read_8_bit(cpu->hl);
	else
		num = *reg_pointers[n];

	carry = cpu->f & CARRY_FLAG ? 1 : 0;

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
		write_8_bit(cpu->hl, num);
	else
		*reg_pointers[n] = num;
}

void RRC_n(unsigned short n, unsigned short NA) {
	unsigned char num;
	unsigned char carry;

	if (n == HL)
		num = read_8_bit(cpu->hl);
	else
		num = *reg_pointers[n];

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
		write_8_bit(cpu->hl, num);
	else
		*reg_pointers[n] = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void RR_n(unsigned short n, unsigned short NA) {
	unsigned char num;

	if (n == HL)
		num = read_8_bit(cpu->hl);
	else
		num = *reg_pointers[n];

	num >>= 1;

	if (cpu->f & CARRY_FLAG)
		num |= 0x80;

	if (num & 0x01)
		set_flag(CARRY_FLAG);
	else
		clear_flag(CARRY_FLAG);

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu->hl, num);
	else
		*reg_pointers[n] = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void SLA_n(unsigned short n, unsigned short NA) {
	unsigned char num;

	if (n == HL)
		num = read_8_bit(cpu->hl);
	else
		num = *reg_pointers[n];

	num <<= 1;

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu->hl, num);
	else
		*reg_pointers[n] = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void SRA_n(unsigned short n, unsigned short NA) {
	unsigned char num;

	if (n == HL)
		num = read_8_bit(cpu->hl);
	else
		num = *reg_pointers[n];
	
	if (num & 0x01)
		clear_flag(CARRY_FLAG);
	else
		set_flag(CARRY_FLAG);

	num = (num & 0x80) | (num >> 1);

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu->hl, num);
	else
		*reg_pointers[n] = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}

void SRL_n(unsigned short n, unsigned short NA) {
	unsigned char num;

	if (n == HL)
		num = read_8_bit(cpu->hl);
	else
		num = *reg_pointers[n];

	if (num & 0x01)
		clear_flag(CARRY_FLAG);
	else
		set_flag(CARRY_FLAG);

	num >>= 1;

	if (num)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (n == HL)
		write_8_bit(cpu->hl, num);
	else
		*reg_pointers[n] = num;

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG);
}