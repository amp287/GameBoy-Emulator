#include <stdio.h>
#define DEBUG_FILE_NAME "Debug.txt"

FILE *debug;

//log: 1 = true 0 = false
void debug_init(int log);