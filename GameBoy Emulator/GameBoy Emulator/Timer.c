#include "Timer.h"
#include "Z80.h"
#include "Memory.h"

int current_freq = TIMER_CONTROL_FREQ_4096;
// default amount of cycles before timer goes off
int timer_cycles = CPU_CLOCK_SPEED/4096;
int divider_cycles = 0;

void divider_register_update(int cycles) {
	divider_cycles += cycles;

	if (divider_cycles >= 255) {
		divider_cycles = 0;
		io[DIVIDER_REGISTER - 0xFF00]++;
	}
}

void timer_update(int cycles) {
	unsigned char timer = read_8_bit(TIMER);
	
	divider_register_update(cycles);

	if (read_8_bit(TIMER_CONTROL) & TIMER_CONTROL_ENABLED) {
		timer_cycles -= cycles;

		if (timer_cycles <= 0) {
			//set freq
			if (timer == 255)
				write_8_bit(INTERRUPT_FLAGS, INTERRUPT_TIMER | read_8_bit(INTERRUPT_FLAGS));
			else
				write_8_bit(TIMER, timer + 1);
		}
	}
}

void change_freq(int freq) {

	if (freq == current_freq)
		return;

	switch (freq) {
		case TIMER_CONTROL_FREQ_16384:
			current_freq = TIMER_CONTROL_FREQ_16384;
			timer_cycles = CPU_CLOCK_SPEED / 16384;
			break;
		case TIMER_CONTROL_FREQ_65536:
			current_freq = TIMER_CONTROL_FREQ_65536;
			timer_cycles = CPU_CLOCK_SPEED / 65536;
			break;
		case TIMER_CONTROL_FREQ_262144:
			current_freq = TIMER_CONTROL_FREQ_262144;
			timer_cycles = CPU_CLOCK_SPEED / 262144;
			break;
		case TIMER_CONTROL_FREQ_4096:
			current_freq = TIMER_CONTROL_FREQ_4096;
			timer_cycles = CPU_CLOCK_SPEED / 4096;
			break;
	}
}
