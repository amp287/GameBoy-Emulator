#include <stdio.h>
#include "Z80.h"
#include "Memory.h"
#include "GPU.h"
#include "Timer.h"
#include "Debug.h"
#include "Cartridge.h"
#include "Background_Viewer.h"
#include "Display.h"

int main(int argc, char *argv[]) {
	char *rom = NULL;

	if(argc > 1){
		rom = argv[1];
	}

	if (load_rom(rom) != 0) {
		printf("Error loading rom\n");
		return -1;
	}

	cpu_init();
	display_init();
	gpu_init();
	background_viewer_init();
	// clock cycles per second / FPS
	// 4194304/60
	
	debug_init(0);
	disable_logging();
	while(1) {
		int cycles = cpu_step();
		timer_update(cycles);
		gpu_update(cycles);
		//background_viewer_draw_screen();
		check_interrupts();

	}
	gpu_stop();
	background_viewer_quit();
	printf("Press a character and then enter to quit.\n");
	getchar();
	return 0;
}
