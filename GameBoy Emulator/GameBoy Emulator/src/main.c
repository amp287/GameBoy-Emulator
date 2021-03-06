#include <stdio.h>
#include "Z80.h"
#include "Memory.h"
#include "PPU.h"
#include "Timer.h"
#include "Debug.h"
#include "Cartridge.h"
#include "Background_Viewer.h"
#include "Display.h"
#include "Interrupts.h"
#include "Tile_Viewer.h"

int main(int argc, char *argv[]) {
	char *rom = NULL;
	int cycles = 0;

	if(argc > 1)
		rom = argv[1];
	else
		rom = "../Roms/cpu_instrs.gb";

	if (load_rom(rom) != 0) {
		printf("Error loading rom\n");
		return -1;
	}

	cpu_init(1);
	display_init();
	gpu_init();
	//background_viewer_init();
	//tile_viewer_init();
	// clock cycles per second / FPS
	// 4194304/60
	
	debug_init(0);
	//enable_logging();
	while(1) {
		cycles = cpu_gpu_step(cycles);
		timer_update(cycles);

		// either returns 0 to reset cycles or
		// returns the number of cycles to process an interrupt
		cycles = check_interrupts();

		//background_viewer_update();
		//tile_viewer_update();
	}
	gpu_stop();
	background_viewer_quit();
	tile_viewer_quit();
	printf("Press a character and then enter to quit.\n");
	getchar();
	return 0;
}
