#include "Memory.h"
#include "GPU.h"
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define SCROLL_Y 0xFF42
#define SCROLL_X 0xFF43
#define WINDOW_Y 0xFF4A
#define WINDOW_X 0xFF4B
#define TILE_SIZE 16

// It takes the GPU 456 cycles to draw one scanline
int scanline_cycles;
int quit;
GLFWwindow* window;

unsigned char screen_buffer[160][144][3];

void render_tiles() {

	//used to find starting point in Background map
	unsigned char scroll_y = read_8_bit(SCROLL_Y);
	unsigned char scroll_x = read_8_bit(SCROLL_X);

	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char window = lcd_control & WINDOW_DISP_ENABLE;
	unsigned short tile_set = lcd_control & BG_AND_WINDOW_TILE_DATA_SELECT;
	unsigned char scanline = read_8_bit(LCD_SCANLINE);

	int i, j, x, y;
	unsigned short tile_map = read_8_bit(LCD_CONTROL) & BG_TILE_MAP_SELECT ? TILE_MAP_1 : TILE_MAP_0;

	// skip tiles to get to correct x,y coordinate in map (starting tile)
	tile_map += scroll_y * 32;
	tile_map += (scanline / 8) * 32;

	tile_map += scroll_x;

	y = (scanline + scroll_y) & 7;
	x = scroll_x & 7;

	//get 18 tiles and extract pixels
	for (i = 0; i < 20; i++) {
		unsigned char byte1;
		unsigned char byte2;
		unsigned short current_tile;

		if (!tile_set) {
			current_tile = (signed char)read_8_bit(tile_map++) + 128;
			tile_set = TILE_SET_0;
		}
		else {
			current_tile = read_8_bit(tile_map++);
			tile_set = TILE_SET_1;
		}
		//starting tile_set address + current_tile index from map * previous tiles
		byte1 = read_8_bit(tile_set + current_tile * 16);
		byte2 = read_8_bit(tile_set + current_tile * 16 + 1);

		// extract pixels
		for (j = 7; j >= 0; j--) {
			unsigned char pixel = ((byte2 >> (j - 1)) & 2) | ((byte1 >> j) & 1);
			unsigned char color = 0;

			if (pixel == 3)
				color = 0;
			else if (pixel == 2)
				color = 96;
			else if (pixel == 1)
				color = 192;
			else
				color = 255;

			screen_buffer[i * 8 + (7 - j)][scanline - 1][0] = color;
			screen_buffer[i * 8 + (7 - j)][scanline - 1][1] = color;
			screen_buffer[i * 8 + (7 - j)][scanline - 1][2] = color;
		}
	}
}

void draw_scanline() {
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2f(-1, 1);
	glPixelZoom(1, -1);

	glDrawPixels(160, 144, GL_RGB, GL_UNSIGNED_BYTE, screen_buffer);
	glfwSwapBuffers(window);
	//glfwPollEvents();
}

void render_scanline() {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char current_scanline = read_8_bit(LCD_SCANLINE);

	if (lcd_control & BG_DISPLAY)
		render_tiles();

	if (lcd_control & SPRITE_DISPLAY)
		;//render_sprites();
}

void lcd_status() {
	unsigned char status = read_8_bit(LCD_STATUS_REG);
	unsigned char current_line = read_8_bit(LCD_SCANLINE);
	unsigned char current_mode = read_8_bit(LCD_STATUS_REG);
	unsigned char mode, interrupt = 0;

	if (!(read_8_bit(LCD_CONTROL) & LCD_ENABLED)) {
		scanline_cycles = 456;
		write_8_bit(LCD_SCANLINE, 0);
		write_8_bit(LCD_STATUS_REG, LCD_STATUS_MODE_1);
		return;
	}

	current_line = read_8_bit(LCD_SCANLINE);
	current_mode = read_8_bit(LCD_STATUS_REG);

	// All scanlines written to
	//go back to top of screen
	if (current_line >= 144) {
		mode = LCD_STATUS_MODE_1;
		status &= ~LCD_STATUS_MODE;
		status |= LCD_STATUS_MODE_1;
		interrupt = status & LCD_STATUS_MODE_1_INTERRUPT;

		goto EXIT_LCD_STATUS;
	}

	if (scanline_cycles >= 376) {
		mode = LCD_STATUS_MODE_2;
		status &= ~LCD_STATUS_MODE;
		status |= LCD_STATUS_MODE_2;
		interrupt = status & LCD_STATUS_MODE_2_INTERRUPT;
	}
	else if (scanline_cycles >= 204) {
		mode = LCD_STATUS_MODE_3;
		status &= ~LCD_STATUS_MODE;
		status |= LCD_STATUS_MODE_3;
	}
	else {
		mode = LCD_STATUS_MODE_0;
		status &= ~LCD_STATUS_MODE;
		interrupt = status & LCD_STATUS_MODE_0_INTERRUPT;
	}

EXIT_LCD_STATUS:
	if (interrupt && (mode != current_mode))
		write_8_bit(INTERRUPT_FLAGS, INTERRUPT_LCD | read_8_bit(INTERRUPT_FLAGS));

	if (current_line == read_8_bit(LCD_SCANLINE_COMPARE)) {
		status |= LCD_STATUS_COINCIDENCE_FLAG;

		if (status & LCD_STATUS_COINCIDENCE_INTERRUPT)
			write_8_bit(INTERRUPT_FLAGS, INTERRUPT_LCD | read_8_bit(INTERRUPT_FLAGS));

	}
	else {
		status &= ~LCD_STATUS_COINCIDENCE_FLAG;
	}

	write_8_bit(LCD_STATUS_REG, status);
}

void gpu_update(int cycles) {
	unsigned char current_scanline;
	lcd_status();

	if (read_8_bit(LCD_CONTROL) & LCD_ENABLED)
		scanline_cycles -= cycles;
	else
		return;

	if (scanline_cycles <= 0) {
		current_scanline = read_8_bit(LCD_SCANLINE) + 1;
		io[LCD_SCANLINE - 0xFF00] = current_scanline;

		// Number of cycles to complete scanline
		scanline_cycles = 456;

		// Reached the end of the screen
		if (current_scanline == 144) {
			write_8_bit(INTERRUPT_FLAGS, INTERRUPT_VBLANK | read_8_bit(INTERRUPT_FLAGS));
			draw_scanline();
		} else if (current_scanline > 153) {
			write_8_bit(LCD_SCANLINE, 0);
		} else if(current_scanline < 144) {
			render_scanline();
		}
	}
}

int gpu_init() {
	//glfwSetErrorCallback(error_callback);

	quit = 0;
	scanline_cycles = 0;
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(160, 144, "GameBoy", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -2;
	}

	//glfwSetKeyCallback(window, key_callback); for setting up inputs later
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//glfwSwapInterval(1);

	/*while (!glfwWindowShouldClose(window) && !quit){
		glfwSwapBuffers(window);
		glfwPollEvents();
	}*/

	
	return 0;
}

int gpu_stop() {
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}