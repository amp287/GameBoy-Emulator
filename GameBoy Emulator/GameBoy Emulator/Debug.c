#include "Debug.h"
#include <string.h>

#define LINE_MAX 500000
#define MAX_FILES 30
int lines, file_num;

void debug_log(const char *fmt, ...) {

	if (debug == NULL || file_num > MAX_FILES)
		return;

	if (lines > LINE_MAX) {
		fclose(debug);
		debug = NULL;
		if (file_num > MAX_FILES)
			return;
		char new_file_name[20];
		sprintf(new_file_name, "log/Debug%d.txt", file_num++);
		debug = fopen(new_file_name, "w");
		lines = 0;
	}
	va_list args;
	va_start(args, fmt);
	int rc = vfprintf(debug, fmt, args);
	va_end(args);
	lines++;
}

void debug_init(int log) {
	lines = 0;
	file_num = 0;

	if (log)
		debug = fopen("log/Debug.txt", "w");
	else
		debug = NULL;
}