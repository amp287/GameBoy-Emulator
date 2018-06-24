#include <string.h>
#include "Debug.h"

#define LINE_MAX 180000
#define MAX_FILES 1
FILE *debug, *serial_output;
int lines, file_num;
int log_flag = 0;
int map_change;

void enable_logging() {
	log_flag = 1;
}
void disable_logging() {
	log_flag = 0;
}

void debug_log(const char *fmt, ...) {
	if (debug == NULL || file_num > MAX_FILES || !log_flag)
		return;

	/*if (lines > LINE_MAX) {
		fclose(debug);
		debug = NULL;
		if (file_num > MAX_FILES)
			return;
		char new_file_name[20];
		sprintf(new_file_name, "log/Debug%d.txt", file_num++);
		debug = fopen(new_file_name, "w");
		lines = 0;
	}*/
	va_list args;
	va_start(args, fmt);
	int rc = vfprintf(debug, fmt, args);
	va_end(args);
	lines++;
}

void debug_on_map_change(){
	map_change = 1;
}

void debug_log_on_map_change(int pc){
	if(map_change) {
		printf("%d\n", pc);
		map_change = 0;
	}
}

void debug_init(int log_arg) {
	lines = 0;
	file_num = 0;
	map_change = 0;
	log_flag = log_arg;

	if (log_arg)
		debug = fopen("log/Debug.txt", "w");
	else
		debug = NULL;

	serial_output = fopen("log/Serial.txt", "w");
}

void debug_log_serial_output(unsigned char byte) {
	if(log_flag)
		fprintf(serial_output, "%c", byte);
}