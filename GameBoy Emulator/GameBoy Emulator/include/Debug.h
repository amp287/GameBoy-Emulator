#include <stdio.h>
#include <stdarg.h>

#define DEBUG_FILE_NAME "Debug.txt"

FILE *debug;
void debug_log(const char *fmt, ...);
//log: 1 = true 0 = false
void debug_init(int log);
void enable_logging();
void disable_logging();

void debug_log_on_map_change();
void debug_on_map_change();