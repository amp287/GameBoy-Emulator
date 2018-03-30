#include "Z80.h"
#include <stdio.h>

int foo(int a) {
	printf("%d \n", a);
}

int main() {
	/*cpu_init();
	cpu_fetch();
	cpu_execute();
	check_interrupts();*/
	int a = 0;
	foo(a += 2);
	foo(a += 2);
	foo(a += 2);
	printf("Press a character and then enter to quit.\n");
	getchar();
	return 0;
}