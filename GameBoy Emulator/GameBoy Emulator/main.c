#include "Z80.h"
#include <stdio.h>
#include "Memory.h"
#include "GPU.h"
#include "Timer.h"
#include "Debug.h"
#include "Cartridge.h"

int main() {

	if (load_rom(NULL) != 0) {
		printf("Error loading rom\n");
		getchar();
		return -1;
	}
	cpu_init();
	gpu_init();
	// clock cycles per second / FPS
	// 4194304/60
	
	debug_init(0);

	while(1) {
		int cycles = cpu_step();
		timer_update(cycles);
		gpu_update(cycles);
		check_interrupts();
	}
	gpu_stop();
	printf("Press a character and then enter to quit.\n");
	getchar();
	return 0;
}
