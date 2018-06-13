#include "Memory.h"
#include "PPU.h"

#define BG_PALETTE 0xFF47
#define PALETTE_00 0x03
#define PALETTE_01 0x0C
#define PALETTE_10 0x30
#define PALETTE_11 0xC0

#define TILE_BYTES 16
#define TILE_ROW_BYTES 2
#define MAP_TILE_SIZE 32

#define TILE_MAP_0 0x9800
#define TILE_MAP_1 0x9C00
#define TILE_SET_1 0x8000
#define TILE_SET_0 0x887F

static unsigned char default_palette[4] = { 255, 192, 96, 0 };

unsigned short get_tile_address(unsigned short map_index) {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned short tile_map = lcd_control & BG_TILE_MAP_SELECT ? TILE_MAP_1 : TILE_MAP_0;
	unsigned short tile_set = lcd_control & BG_AND_WINDOW_TILE_DATA_SELECT ? TILE_SET_1 : TILE_SET_0;


	char tile_id = read_8_bit(tile_map + map_index);

	// get unsigned value (Check if this works)
	if (tile_set == TILE_SET_0)
		tile_id += 128;

	return tile_set + (tile_id * TILE_BYTES);
}

// Returns tile in tile_out
void get_tile(unsigned short *tile_out, unsigned char map_x, unsigned char map_y) {
	int i;
	unsigned short tile_addr = get_tile_address(map_x + map_y * MAP_TILE_SIZE);

	for (i = 0; i < 8; i++)
		tile_out[i] = read_16_bit(tile_addr + (i * TILE_ROW_BYTES));
}

// Returns color 
unsigned char get_pixel(unsigned short tile_row) {
	unsigned short color = tile_row & 0x8080;
	unsigned char palette = read_8_bit(BG_PALETTE);

	if (color == 0x0)
		return default_palette[palette & PALETTE_00];
	else if (color == 0x8000)
		return default_palette[palette & PALETTE_01];
	else if (color == 0x0008)
		return default_palette[(palette & PALETTE_10) >> 4];
	else
		return default_palette[(palette & PALETTE_11) >> 6];
}
