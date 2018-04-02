#include "Memory.h"
#define SCROLL_Y 0xFF42
#define SCROLL_X 0xFF43
#define WINDOW_Y 0xFF4A
#define WINDOW_X 0xFF4B


void render_tiles() {
	unsigned char scroll_y = read_8_bit(SCROLL_Y);
	unsigned char scroll_x = read_8_bit(SCROLL_X);
	unsigned char window_y = read_8_bit(WINDOW_Y);
	unsigned char window_x = read_8_bit(WINDOW_X) - 7;


}