#include "Z80.h"
#include "Memory.h"
#include "GPU.h"
#include "stdio.h"

CPU *cpu;

unsigned char *reg_pointers[16];

typedef struct INSTRUCTION_REGISTER {
	int instruction_index;
	unsigned char is_cb;
	OPCODE_OPERATION execute;
	unsigned short first_param;
	unsigned short second_param;
}INSTRUCTION_REGISTER;

INSTRUCTION_REGISTER ir;

void cpu_init() {
	cpu = malloc(sizeof(CPU));

	reg_pointers[0] = &cpu->a;
	reg_pointers[1] = &cpu->b;
	reg_pointers[2] = &cpu->c;
	reg_pointers[3] = &cpu->d;
	reg_pointers[4] = &cpu->e;
	reg_pointers[5] = &cpu->h;
	reg_pointers[6] = &cpu->l;
	reg_pointers[7] = (unsigned char*)&cpu->af;
	reg_pointers[8] = (unsigned char*)&cpu->bc;
	reg_pointers[9] = (unsigned char*)&cpu->de;
	reg_pointers[10] = (unsigned char*)&cpu->hl;
	//reg_pointers[11] = &cpu->;
	//reg_pointers[12] = &cpu->;
	reg_pointers[15] = (unsigned char*)&cpu->sp;
	load_bios();
	load_rom();
	cpu_reset();
}

void cpu_print_reg_stack() {
	int i;
	printf("Registers: A:%x B:%x C:%x\n\t\tD:%x E:%x H:%x L:%x\n", cpu->a, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l);
	printf("Stack:");
	for (i = 0xfffd; i >= cpu->sp; i--)
		printf("%x ", read_8_bit(i));
	printf("\n\n");
}

int cpu_step() {
	int cycles = 0;
	//printf("\t\tpc:%04x\n", cpu->pc);

	if (!halt_flag || !stop_flag) {
		cycles = cpu_fetch();
		if (cpu_execute()) {
			return -1;
		}
	}

	

	//cpu_print_reg_stack();

	// Remove this after testing
	if (cpu->pc == 0x6e)
		printf("wow i made it\n");
	return cycles;
}

int cpu_fetch() {
	int index = read_8_bit(cpu->pc++);
	INSTR *map;

	if (index == 0xCB) {
		map = opcodesCB;
		ir.is_cb = 1;
		index = read_8_bit(cpu->pc++);
	}
	else {
		map = opcodes;
		ir.is_cb = 0;
	}

	ir.instruction_index = index;

	if (map[index].r2 == READ_8) {
		ir.second_param = read_8_bit(cpu->pc++);
	} else if (map[index].r2 == READ_16) {
		ir.second_param = read_16_bit(cpu->pc); 
		cpu->pc += 2;
	} else {
		ir.second_param = map[index].r2;
	}

	ir.first_param = map[index].r1;

	ir.execute = map[index].execute;
	
	return map[index].cycles;
}

int cpu_execute() {
	//if DEBUG
	//display assembly or print log
	if (ir.execute == NULL) {
		printf("cpu_execute: Error unimplemented opcode [%s]\n", ir.is_cb ? opcodesCB[ir.instruction_index].disassembly : opcodes[ir.instruction_index].disassembly);
		getchar();
		return -1;
	}

	//printf("cpu_execute: [%s] ", ir.is_cb ? opcodesCB[ir.instruction_index].disassembly : opcodes[ir.instruction_index].disassembly);
	//printf("%x %x \n", ir.first_param, ir.second_param);
	(ir.execute)(ir.first_param, ir.second_param);

	return 0;
}

int check_interrupts() {
	unsigned char enabled = read_8_bit(INTERRUPT_ENABLE);
	unsigned char flags = read_8_bit(INTERRUPT_FLAGS);

	//used to enable interrupts after one instruction
	if (counter_interrupt > 0)
		enable_interrupt--;
	if (counter_interrupt == 0) {
		cpu->master_interrupt = enable_interrupt;
		enable_interrupt--;
	}

	if (cpu->master_interrupt && enabled && flags) {
		unsigned char fired = enabled & flags;

		if (halt_flag)
			halt_flag = 0;

		cpu->master_interrupt = 0;
		cpu->sp -= 2;
		write_16_bit(cpu->sp, cpu->pc);

		if (fired & INTERRUPT_VBLANK) {
			cpu->pc = 0x40;
			flags &= ~INTERRUPT_VBLANK;
		} else if (fired & INTERRUPT_LCD) {
			cpu->pc = 0x48;
			flags &= ~INTERRUPT_LCD;
		} else if (fired & INTERRUPT_TIMER) {
			cpu->pc = 0x50;
			flags &= ~INTERRUPT_TIMER;
		} else if (fired & INTERRUPT_SERIAL) {
			cpu->pc = 0x58;
			flags &= ~INTERRUPT_SERIAL;
		} else if (fired & INTERRUPT_JOYPAD) {
			cpu->pc = 0x60;
			stop_flag = 0;
			flags &= ~INTERRUPT_JOYPAD;
		}

		write_8_bit(INTERRUPT_FLAGS, flags);
		//add 12 ticks
	}
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

void LD_A_n(unsigned short type, unsigned short n) {

	switch (n) {
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
			cpu->a = *reg_pointers[n];
	}
}

void LD_A_nn(unsigned short a, unsigned short nn) {
	cpu->a = read_8_bit(nn);
}

void LD_A_imm(unsigned short a, unsigned short immediate) {
	cpu->a = (unsigned char)immediate;
}

void LD_n_A(unsigned short type, unsigned short n) {

	if (type == READ_16) {
		write_8_bit(n, cpu->a);
		return;
	}

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

void LDH_n_A(unsigned short NA, unsigned short n) {
	write_8_bit(0xFF00 + n, cpu->a);
}

void LDH_A_n(unsigned short A, unsigned short n) {
	cpu->a = read_8_bit(0xFF00 + n);
}

//16-Bit Loads
void LD_n_nn(unsigned short n, unsigned short nn) {
	switch (n) {
		case BC:
			cpu->bc = nn;
			break;
		case DE:
			cpu->de = nn;
			break;
		case HL:
			cpu->hl = nn;
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

void LD_nn_SP(unsigned short sp, unsigned short nn) {
	write_16_bit(nn, cpu->sp);
}

void PUSH_nn(unsigned short nn, unsigned short NA) {
	unsigned short val;

	cpu->sp -= 2;

	switch (nn) {
		case AF:
			val = cpu->af;
			break;
		case BC:
			val = cpu->bc;
			break;
		case DE:
			val = cpu->de;
			break;
		case HL:
			val = cpu->hl;
		}

	write_16_bit(cpu->sp, val);
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

void ADD_A_n(unsigned short type, unsigned short n) {
	// check if n is immediate
	if (type == READ_8)
		;
	else if (n == HL)
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

void ADC_A_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
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

void SUB_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
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

void SBC_A_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
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

void AND_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
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

void OR_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
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

void XOR_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
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

void CP_n(unsigned short type, unsigned short n) {
	if (type == READ_8)
		;
	else if (n == HL)
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
		set_flag(ZERO_FLAG);
	else
		clear_flag(ZERO_FLAG);
}

void INC_n(unsigned short type, unsigned short n) {
	unsigned char val;

	if (type == READ_8)
		;
	else if (n == HL)
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

void DEC_n(unsigned short NA, unsigned short n) {
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
	switch (nn) {
		case AF:
			cpu->af++;
			break;
		case BC:
			cpu->bc++;
			break;
		case DE:
			cpu->de++;
			break;
		case HL:
			cpu->hl++;
	}
}

void DEC_nn(unsigned short nn, unsigned short NA) {
	switch (nn) {
		case AF:
			cpu->af--;
			break;
		case BC:
			cpu->bc--;
			break;
		case DE:
			cpu->de--;
			break;
		case HL:
			cpu->hl--;
	}
}

//Miscellaneous
void SWAP_n(unsigned short n, unsigned short NA) {
	unsigned char upper, lower;
	unsigned val;

	if (n == HL)
		val = read_8_bit(cpu->hl);
	else
		val = *reg_pointers[n];

	upper = 0xF0 & val;
	lower = 0x0F & val;

	val = lower << 4;
	val &= upper >> 4;

	if (val)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	clear_flag(SUBTRACT_FLAG | HALF_CARRY_FLAG | CARRY_FLAG);

	if (n == HL)
		write_8_bit(cpu->hl, val);
	else
		*reg_pointers[n] = val;
}

void DAA(unsigned short NA_1, unsigned short NA_2) {
	unsigned char s = cpu->a;

	if (cpu->f & SUBTRACT_FLAG) {
		if (cpu->f & HALF_CARRY_FLAG) 
			s = (s - 0x06) & 0xFF;
		if (cpu->f & CARRY_FLAG) 
			s -= 0x60;
	} else {
		if ((cpu->f & HALF_CARRY_FLAG) || (s & 0xF) > 9) 
			s += 0x06;
		if ((cpu->f & CARRY_FLAG) || s > 0x9F) 
			s += 0x60;
	}

	cpu->a = s;
	clear_flag(HALF_CARRY_FLAG);

	if (cpu->a)
		clear_flag(ZERO_FLAG);
	else
		set_flag(ZERO_FLAG);

	if (s >= 0x100)
		set_flag(CARRY_FLAG);
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
void HALT(unsigned short NA_1, unsigned short NA_2) {
	halt_flag = 1;
}

//Halt CPU & LCD display until button pressed
void STOP(unsigned short NA_1, unsigned short NA_2) {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	stop_flag = 1;
	lcd_control |= ~LCD_ENABLED;
	write_8_bit(LCD_CONTROL, lcd_control);//TODO check this http://www.codeslinger.co.uk/pages/projects/gameboy/lcd.html
}

//This instruction disables interrupts but not
//immediately.Interrupts are disabled after
//instruction after DI is executed.
void DI(unsigned short NA_1, unsigned short NA_2) {
	enable_interrupt = 0;
	counter_interrupt = 1;
}

//Enable interrupts. This intruction enables interrupts
//but not immediately.Interrupts are enabled after
//instruction after EI is executed.
void EI(unsigned short NA_1, unsigned short NA_2) {
	enable_interrupt = 1;
	counter_interrupt = 1;
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

	clear_flag(SUBTRACT_FLAG | ZERO_FLAG | HALF_CARRY_FLAG);
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

	num <<= 1;
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

//Bit Opcodes

void BIT_b_r(unsigned short b, unsigned short r) {
	unsigned char bit_test = 1 << b;
	unsigned int res;
	
	if (r == HL) {
		res = read_8_bit(cpu->hl) & bit_test;
	} else {
		res = *reg_pointers[r] & bit_test;
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
		res = read_8_bit(cpu->hl) | bit_test;
		write_8_bit(cpu->hl, res);
	}
	else {
		*reg_pointers[r] |= bit_test;
	}
}

void RES_b_r(unsigned short b, unsigned short r) {
	unsigned char bit_test = ~(1 << b);
	unsigned int res;

	if (r == HL) {
		res = read_8_bit(cpu->hl) & bit_test;
		write_8_bit(cpu->hl, res);
	}
	else {
		*reg_pointers[r] &= bit_test;
	}
}

//Jumps
void JP_nn(unsigned short NA, unsigned short nn) {
	cpu->pc = nn;
}

void JP_cc_nn(unsigned short cc, unsigned short nn) {
	char jump = 0;

	switch (cc) {
		case NZ:
			jump = !(cpu->f & ZERO_FLAG);
			break;
		case Z:
			jump = cpu->f & ZERO_FLAG;
			break;
		case NC:
			jump = !(cpu->f & CARRY_FLAG);
			break;
		case C:
			jump = cpu->f & CARRY_FLAG;
			break;
		default:
			//Error here
			;
	}

	if (jump)
		cpu->pc = nn;
}

void JP_HL(unsigned short NA_1, unsigned short NA_2) {
	cpu->pc = cpu->hl;
}

void JR_n(unsigned short NA, unsigned short n) {
	cpu->pc += (signed char)n;
}

void JR_cc_n(unsigned short cc, unsigned short n) {
	char jump = 0;

	switch (cc) {
	case NZ:
		jump = !(cpu->f & ZERO_FLAG);
		break;
	case Z:
		jump = cpu->f & ZERO_FLAG;
		break;
	case NC:
		jump = !(cpu->f & CARRY_FLAG);
		break;
	case C:
		jump = cpu->f & CARRY_FLAG;
		break;
	default:
		//Error here
		;
	}

	if (jump)
		cpu->pc += (signed char)n;
}

//Calls

void CALL_nn(unsigned short NA, unsigned short nn) {
	cpu->sp -= 2;
	write_16_bit(cpu->sp, cpu->pc); // Put next instruction on stack
	cpu->pc = nn;
}

void CALL_cc_nn(unsigned short cc, unsigned short nn) {
	char jump = 0;

	switch (cc) {
	case NZ:
		jump = !(cpu->f & ZERO_FLAG);
		break;
	case Z:
		jump = cpu->f & ZERO_FLAG;
		break;
	case NC:
		jump = !(cpu->f & CARRY_FLAG);
		break;
	case C:
		jump = cpu->f & CARRY_FLAG;
		break;
	default:
		//Error here
		;
	}
	CALL_nn(nn, 0);
}

//Restarts 

//n = 0x0, 0x8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 
void RST_n(unsigned short n, unsigned short NA) {
	cpu->sp -= 2;
	write_16_bit(cpu->sp, cpu->pc);
	cpu->pc = n;
}

//Returns 
void RET(unsigned short NA_1, unsigned short NA_2) {
	cpu->pc = read_16_bit(cpu->sp);
	cpu->sp += 2;
}

void RET_cc(unsigned short cc, unsigned short NA) {
	char ret = 0;

	switch (cc) {
	case NZ:
		ret = !(cpu->f & ZERO_FLAG);
		break;
	case Z:
		ret = cpu->f & ZERO_FLAG;
		break;
	case NC:
		ret = !(cpu->f & CARRY_FLAG);
		break;
	case C:
		ret = cpu->f & CARRY_FLAG;
		break;
	default:
		//Error here
		;
	}

	if (ret)
		RET(0, 0);
}

void RETI(unsigned short NA_1, unsigned short NA_2) {
	cpu->master_interrupt = 1;
	cpu->pc = read_16_bit(cpu->sp);
	cpu->sp += 2;
}