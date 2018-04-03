#include "Memory.h"
#include "GPU.h"
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

void render_tiles() {
	unsigned char scroll_y = read_8_bit(SCROLL_Y);
	unsigned char scroll_x = read_8_bit(SCROLL_X);
	unsigned char window_y = read_8_bit(WINDOW_Y);
	unsigned char window_x = read_8_bit(WINDOW_X) - 7;

	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char window = lcd_control & WINDOW_DISP_ENABLE;
	unsigned short tile_set = lcd_control & BG_AND_WINDOW_TILE_DATA_SELECT;
	unsigned char scanline = read_8_bit(LCD_SCANLINE);

	int i;
	unsigned char tile_map;
	unsigned char y_position;
	unsigned char tile_row;

	if (window)
		if (window_y > scanline)
			window = 0;	//window is not on this scanline 
		else
			tile_map = lcd_control & WINDOW_TILE_MAP_SELECT ? TILE_MAP_1 : TILE_MAP_0;
	
	if(!window)
		tile_map = lcd_control & BG_TILE_MAP_SELECT ? TILE_MAP_1 : TILE_MAP_0;

	if (!window)
		y_position = scroll_y + scanline;
	else
		y_position = scanline - window_y;

	tile_row = ((unsigned char)(y_position / 8)) * 32;

	for (i = 0; i < 160; i++) {

	}
}

void draw_scanline() {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);

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
		write_8_bit(LCD_SCANLINE, current_scanline);

		// Number of cycles to complete scanline
		scanline_cycles = 456;

		// Reached the end of the screen
		if (current_scanline == 144)
			write_8_bit(INTERRUPT_FLAGS, INTERRUPT_VBLANK | read_8_bit(INTERRUPT_FLAGS));
		else if (current_scanline > 153)
			write_8_bit(LCD_SCANLINE, 0);
		else if (current_scanline < 144)
			;//drawscanline
	}
}

int gpu_init() {
	//glfwSetErrorCallback(error_callback);

	quit = 0;

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
	glfwSwapInterval(1);

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