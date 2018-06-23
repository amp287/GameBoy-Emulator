#include "Interrupts.h"
#include "Z80.h"
#include "Memory.h"

#define INTERRUPT_ENABLE 0xFFFF
#define INTERRUPT_FLAGS 0xFF0F

unsigned char master_interrupt;

int waiting_set = 0;
int waiting_reset = 0;

int check_interrupts() {
	unsigned char enabled = read_8_bit(INTERRUPT_ENABLE);
	unsigned char flags = read_8_bit(INTERRUPT_FLAGS);

	// Used to set/reset interrupt at the correct time
	// (instruction after EI or DI has finished)
	/*if (waiting_set && --waiting_set == 0)
		set_master_interrupt(0);
	if (waiting_reset && --waiting_reset == 0)
		reset_master_interrupt(0);*/

	if (cpu_halt_status() && !master_interrupt)
		cpu_unhalt();
		
	if (master_interrupt && (enabled & flags)) {
		unsigned char fired = enabled & flags;
		unsigned short pc = 0;

		master_interrupt = 0;

		cpu_unhalt();

		if (fired & INTERRUPT_VBLANK) {
			pc = 0x40;
			flags &= ~INTERRUPT_VBLANK;
		}
		else if (fired & INTERRUPT_LCD) {
			pc = 0x48;
			flags &= ~INTERRUPT_LCD;
		}
		else if (fired & INTERRUPT_TIMER) {
			pc = 0x50;
			flags &= ~INTERRUPT_TIMER;
		}
		else if (fired & INTERRUPT_SERIAL) {
			pc = 0x58;
			flags &= ~INTERRUPT_SERIAL;
		}
		else if (fired & INTERRUPT_JOYPAD) {
			pc = 0x60;
			flags &= ~INTERRUPT_JOYPAD;
		}

		cpu_fire_interrupt(pc);
		write_8_bit(INTERRUPT_FLAGS, flags);
		// cycles
		return 20;
	}
	return 0;
}

void request_interrupt(unsigned char type) {
	unsigned char i_flags = read_8_bit(INTERRUPT_FLAGS);
	i_flags |= type;
	write_8_bit(INTERRUPT_FLAGS, i_flags);
}

void reset_master_interrupt(int wait) {
	if (wait) {
		waiting_reset = 2;
		return;
	}
	master_interrupt = 0;
}

void set_master_interrupt(int wait) {
	if (wait) {
		waiting_set = 2;
		return;
	}
	master_interrupt = 1;
}