#include <stdio.h>
#include "Memory.h"
#include "GPU.h"
#include "Display.h"
#include "Debug.h"

#define SCROLL_Y 0xFF42
#define SCROLL_X 0xFF43
#define WINDOW_Y 0xFF4A
#define WINDOW_X 0xFF4B
#define TILE_SIZE 16

static const char *window_title = "Gameboy";
static int quit;
static GLFWwindow* gameboy_window;

int x = 0;
// It takes the GPU 456 cycles to draw one scanline
int scanline_cycles;

unsigned char screen_buffer[144][160][3];

void lcd_interrupt(char type) {
	unsigned char interrupt = read_8_bit(INTERRUPT_FLAGS);
	unsigned char stat = read_8_bit(LCD_STATUS_REG);
	unsigned char interrupt_enable = read_8_bit(INTERRUPT_ENABLE);

	// Check to see if Interrupt is enabled in Stat
	if (stat & type) {
		if (type & LCD_STATUS_VERTICAL_BLANK_INTERRUPT && interrupt_enable & INTERRUPT_VBLANK) {
			write_8_bit(INTERRUPT_FLAGS, interrupt | INTERRUPT_VBLANK);
		}
		if (interrupt_enable & INTERRUPT_LCD) {
			write_8_bit(INTERRUPT_FLAGS, interrupt | INTERRUPT_LCD);
		}
	}
}

// Returns color 
unsigned char get_pixel(unsigned short tile_row) {
	unsigned short color = tile_row & 0x8080;

	if (color == 0x0)
		return 255;
	else if (color == 0x8000)
		return 192;
	else if (color == 0x0008)
		return 96;
	else
		return 0; 
}

void get_tile(unsigned short addr, unsigned short *tile) {
	int i; 
	
	for (i = 0; i < 8; i++) {
		unsigned short shrt = read_16_bit(addr + (i * 2));
		tile[i] = shrt;
	}
}

void render_scanline_tiles() {

	//fill checkerboard for display test
	//test();

	//used to find starting point in Background map
	unsigned char scroll_y = read_8_bit(SCROLL_Y);
	unsigned char scroll_x = read_8_bit(SCROLL_X);

	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char window = lcd_control & WINDOW_DISP_ENABLE;
	unsigned char scanline = read_8_bit(LCD_SCANLINE);

	int pixel = 0;
	unsigned short tile_map = lcd_control & BG_TILE_MAP_SELECT ? TILE_MAP_1 : TILE_MAP_0;
	unsigned short tile_set = lcd_control & BG_AND_WINDOW_TILE_DATA_SELECT ? TILE_SET_1 : TILE_SET_0;
	

	// starting tile y 0-31
	unsigned char starting_tile_y = ((scroll_y + scanline) / 8) % 32;

	// get the x and y pixels to start at in the starting tile
	unsigned char tile_x = scroll_x % 8;
	unsigned char tile_y = (scroll_y + scanline) % 8;
	//TODO: check to see what tile source to use
	//printf("Scanline:%d\n", scanline);

	while(pixel < 160) {
		unsigned short tile[8];
		unsigned short starting_tile;
	
		//starting tile x : 0-31
		unsigned char starting_tile_x = scroll_x / 8;
		starting_tile_x = (starting_tile_x + (pixel / 8)) % 32;

		starting_tile = starting_tile_x + (starting_tile_y * 32);
		
		short tile_id = vram[(tile_map - 0x8000) + starting_tile];

		if (tile_set == TILE_SET_0)
			tile_id += 128; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		get_tile((tile_set) + (tile_id * 16), tile);
		unsigned char color;
		int i, i_start;
		if (pixel == 0)
			i_start = tile_x;
		else
			i_start = 0;

		for (i = i_start; i < 8; i++) {
			color = get_pixel(tile[tile_y] << i);

			screen_buffer[scanline][pixel][0] = color;
			screen_buffer[scanline][pixel][1] = color;
			screen_buffer[scanline][pixel][2] = color;
			pixel++;
		}
	}
}

void render_scanline() {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char current_scanline = read_8_bit(LCD_SCANLINE);

	if (lcd_control & BG_DISPLAY)
		render_scanline_tiles();

	if (lcd_control & SPRITE_DISPLAY)
		;//render_sprites();
}

void gpu_update(int cycles) {
	unsigned char current_scanline = read_8_bit(LCD_SCANLINE);
	unsigned char mode = read_8_bit(LCD_STATUS_REG) & LCD_STATUS_MODE;
	unsigned char compare = read_8_bit(LCD_SCANLINE_COMPARE);
	unsigned char interrupt = 0;

	// Make gameboy window current window 
	display_poll_events(gameboy_window);

	if (!(read_8_bit(LCD_CONTROL) & LCD_ENABLED)) {
		io[LCD_SCANLINE - 0xFF00] = 0;
		write_8_bit(LCD_STATUS_REG, (read_8_bit(LCD_STATUS_REG) & ~LCD_STATUS_MODE) | LCD_STATUS_VERTICAL_BLANK);
		return;
	}
	
	scanline_cycles += cycles;

	switch (mode) {
		case LCD_STATUS_ACCESS_OAM:
			if (scanline_cycles >= 80) {
				scanline_cycles -= 80;
				mode = LCD_STATUS_ACCESS_VRAM;
				//debug_log("ACCESS VRAM\n");
			}
			break;
		case LCD_STATUS_ACCESS_VRAM:
			if (scanline_cycles >= 172) {
				scanline_cycles -= 172;
				mode = LCD_STATUS_HORIZONTAL_BLANK;
				interrupt = LCD_STATUS_HORIZONTAL_BLANK_INTERRUPT;
				render_scanline();
				//debug_log("HBLANK\n");
			}
			break;
		case LCD_STATUS_HORIZONTAL_BLANK:
			if (scanline_cycles >= 204) {
				scanline_cycles -= 204;

				current_scanline++;
				io[LCD_SCANLINE - 0xFF00] = current_scanline;

				if (current_scanline == 144) {
					mode = LCD_STATUS_VERTICAL_BLANK;
					interrupt = LCD_STATUS_VERTICAL_BLANK_INTERRUPT;
					//debug_log("VBLANK\n");
					display_update_buffer(gameboy_window, screen_buffer, 160, 144);
				} else {
					mode = LCD_STATUS_ACCESS_OAM;
					interrupt = LCD_STATUS_OAM_INTERRUPT;
					//debug_log("ACCESS OAM\n");
				}
			}
			break;
		case LCD_STATUS_VERTICAL_BLANK:
			if (scanline_cycles >= 456) {
				scanline_cycles -= 456;
				current_scanline++;
				io[LCD_SCANLINE - 0xFF00] = current_scanline;

				if (current_scanline == 153) {
					current_scanline = 0;
					io[LCD_SCANLINE - 0xFF00] = 0;
					mode = LCD_STATUS_ACCESS_OAM;
					interrupt |= LCD_STATUS_OAM_INTERRUPT;
					//debug_log("ACCESS OAM\n");
				}
				//This is how CoffeeGB does it (IDK WHY)
				/*else if (current_scanline == 0) {
					scanline_cycles -= 456;
					current_scanline++;
					io[LCD_SCANLINE - 0xFF00] = current_scanline;
					mode = LCD_STATUS_ACCESS_OAM;
					interrupt |= LCD_STATUS_OAM_INTERRUPT;
					//debug_log("ACCESS OAM\n");
				}*/

				if (compare == current_scanline)
					interrupt = LCD_STATUS_COINCIDENCE_INTERRUPT;	//These may cause problems...
			} 
			break;
	}
	
	lcd_interrupt(interrupt);

	mode = (read_8_bit(LCD_STATUS_REG) & ~LCD_STATUS_MODE) | mode;
	write_8_bit(LCD_STATUS_REG, mode);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int gpu_init() {
	
	quit = 0;
	scanline_cycles = 0;

	gameboy_window = display_create_window(160, 144, window_title, key_callback);

	return 0;
}

int gpu_stop() {
	glfwDestroyWindow(gameboy_window);
	return 0;
}
