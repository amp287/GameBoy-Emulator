#include "Z80.h"
#include <stdio.h>
#include "Memory.h"



int main() {
	int i;
	/*FILE *bios_asm = fopen("bios.txt", "w");

	for (i = 0; i < 0xff; i++)
	{
		INSTR *map;
		int index = bios[i];
		if (index == 0xCB) {
			fprintf(bios_asm, "%x %s", i, opcodesCB[index].disassembly);
			map = opcodesCB;

		} else {
			fprintf(bios_asm, "%x %s", i, opcodes[index].disassembly);
			map = opcodes;
		}

		if (map[index].r2 == READ_8) {
			fprintf(bios_asm, " %x\n", bios[++i]);
		} else if (map[index].r2 == READ_16) {
			fprintf(bios_asm, " %x\n", bios[++i] | bios[++i] << 8);
		} else {
			fprintf(bios_asm, "\n");
		}
	}
	fclose(bios_asm);*/
	cpu_init();
	// clock cycles per second / FPS
	// 4194304/60
	const int max_cycles = 69905;
	
	while(1) {
		int cycles_this_update = 0;
		// ensures 60 Fps
		while (cycles_this_update < max_cycles) {
			int cycles = cpu_step();
			cycles_this_update += cycles;
			//update_timers()
			//update_graphics()

		}
		//render_screen();
	}

	printf("Press a character and then enter to quit.\n");
	getchar();
	return 0;
}