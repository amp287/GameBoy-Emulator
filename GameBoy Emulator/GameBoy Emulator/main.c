#include "Z80.h"

int main() {
	cpu_init();
	cpu_fetch();
	cpu_execute();
	check_interrupts();
	printf("Press a character and then enter to quit.\n");
	getchar();
	return 0;
}