#include "Debug.h"
void debug_log(char *text) {
	//fprintf(debug, "")
}
void debug_init(int log) {
	debug = fopen(DEBUG_FILE_NAME, "w");
}