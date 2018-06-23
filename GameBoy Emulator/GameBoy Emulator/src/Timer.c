#include "Timer.h"
#include "Interrupts.h"
#include "Z80.h"
#include "Memory.h"

#define TIMER 0xFF05
#define TIMER_MODULATOR 0xFF06

int current_freq = TIMER_CONTROL_FREQ_4096;
// default amount of cycles before timer goes off
int timer_cycles = CPU_CLOCK_SPEED / 4096;
int divider_cycles = 0;

void reset_freq_timers() {
	switch (current_freq) {
	case TIMER_CONTROL_FREQ_16384:
		timer_cycles = 256;// CPU_CLOCK_SPEED / 16384;
		break;
	case TIMER_CONTROL_FREQ_65536:
		timer_cycles = 64;// CPU_CLOCK_SPEED / 65536;
		break;
	case TIMER_CONTROL_FREQ_262144:
		timer_cycles = 16;// CPU_CLOCK_SPEED / 262144;
		break;
	case TIMER_CONTROL_FREQ_4096:
		timer_cycles = 1024;// CPU_CLOCK_SPEED / 4096;
		break;
	}
}

void divider_register_update(int cycles) {
	divider_cycles += cycles;

	if (divider_cycles >= 0xFF) {
		divider_cycles = 0;
		io[DIVIDER_REGISTER - 0xFF00]++;
	}
}

void timer_update(int cycles) {
	unsigned char timer = read_8_bit(TIMER);

	cycles /= 4;
	
	divider_register_update(cycles);

	if (read_8_bit(TIMER_CONTROL) & TIMER_CONTROL_ENABLED) {
		timer_cycles -= cycles;

		if (timer_cycles <= 0) {

			reset_freq_timers();

			if (timer == 0xFF) {
				write_8_bit(TIMER_MODULATOR, timer);
				request_interrupt(INTERRUPT_TIMER);
			} else {
				write_8_bit(TIMER, timer + 1);
			}
		}
	}
}

void set_freq() {
	unsigned char freq = get_freq();

	if (freq == current_freq)
		return;

	switch (freq) {
		case TIMER_CONTROL_FREQ_16384:
			current_freq = TIMER_CONTROL_FREQ_16384;
			timer_cycles = 256;// CPU_CLOCK_SPEED / 16384;
			break;
		case TIMER_CONTROL_FREQ_65536:
			current_freq = TIMER_CONTROL_FREQ_65536;
			timer_cycles = 64;// CPU_CLOCK_SPEED / 65536;
			break;
		case TIMER_CONTROL_FREQ_262144:
			current_freq = TIMER_CONTROL_FREQ_262144;
			timer_cycles = 16;// CPU_CLOCK_SPEED / 262144;
			break;
		case TIMER_CONTROL_FREQ_4096:
			current_freq = TIMER_CONTROL_FREQ_4096;
			timer_cycles = 1024;// CPU_CLOCK_SPEED / 4096;
			break;
	}
}

unsigned char get_freq() {
	return read_8_bit(TIMER_CONTROL) & TIMER_CONTROL_FREQ_BITS;
}