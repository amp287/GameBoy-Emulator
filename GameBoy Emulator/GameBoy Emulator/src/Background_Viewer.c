#include <stdio.h>
#include <string.h>
#include "Memory.h"
#include "GPU.h"
#include "Display.h"
#include "Background_Viewer.h"


// In pixels
#define TILE_PIXEL_SIZE 8

#define MAP_TILE_SIZE 32

#define TILE_BYTES 16
#define TILE_ROW_BYTES 2

int quit;
static GLFWwindow* background_window;
static const char *window_title = "Map Background Viewer";
static unsigned char buffer[256][256][3];

void *lock;
void *thread; 

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

unsigned short get_tile_address(unsigned short map_index) {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned short tile_map = lcd_control & BG_TILE_MAP_SELECT ? TILE_MAP_1 : TILE_MAP_0;
	unsigned short tile_set = lcd_control & BG_AND_WINDOW_TILE_DATA_SELECT ? TILE_SET_1 : TILE_SET_0;


	char tile_id = read_8_bit(tile_map + map_index);

	if (tile_map + map_index == 0x9910)
		tile_id += 0;

	// get unsigned value (Check if this works)
	if (tile_set == TILE_SET_0)
		tile_id += 128;

	return tile_set + tile_id * TILE_BYTES;
}

static void get_tile(unsigned short *tile_out, unsigned short map_index) {
	int i;
	unsigned short tile_addr = get_tile_address(map_index);

	for (i = 0; i < 8; i++)
		tile_out[i] = read_16_bit(tile_addr + (i * TILE_ROW_BYTES));
}

// Returns color 
static unsigned char get_pixel(unsigned short tile_row) {
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

void background_viewer_update_screen() {
	unsigned short i, j, pi, pj;
	unsigned short tile[8];

	for (i = 0; i < MAP_TILE_SIZE; i++) {
		for (j = 0; j < MAP_TILE_SIZE; j++) {
			int x, y;
			x = i * TILE_PIXEL_SIZE;
			y = j * TILE_PIXEL_SIZE;

			get_tile(tile, i * MAP_TILE_SIZE + j);

			for (pi = 0; pi < TILE_PIXEL_SIZE; pi++) {
				for (pj = 0; pj < TILE_PIXEL_SIZE; pj++) {
					unsigned char color = get_pixel(tile[pi] << pj);

					if (x + pi >= 256 && x + pi < 0)
						printf("WHAT EVEN!");

					if (y + pj >= 256 && y + pj < 0)
						printf("WHAT EVEN! 2 Electric Bugaloo");

					buffer[x + pi][y + pj][0] = color;
					buffer[x + pi][y + pj][1] = color;
					buffer[x + pi][y + pj][2] = color;
				}
			}
		}
	}
}

void background_viewer_draw_screen() {
	background_viewer_update_screen();
	display_poll_events(background_window);
	display_update_buffer(background_window, buffer, 256, 256);
}

void *background_viewer_get_thread(){
	return thread;
}

void *background_viewer_run(void *arg){
	int *quit = arg;
	int quit_local = 0;
	
	background_window = display_create_window(256, 256, window_title, key_callback);

	while(!quit_local){
		background_viewer_draw_screen();

		if(mutex_lock(lock) == 0){
			quit_local = *quit;	
			mutex_unlock(lock);
		}
	}

	display_destroy(background_window);

	return NULL;
}

int background_viewer_init() {
	int ret = 0;
	quit = 0;

	ret = mutex_create(&lock);

	if(ret != 0) {
		return ret;
	}

	ret = thread_create(&thread, &background_viewer_run, &quit);//pthread_create(&thread, NULL, &background_viewer_run, &quit);

	if(ret != 0){
		printf("background_viewer_init() Thread creation failed (error:%d)\n",ret);
		return ret;
	}

	return 0;
}

int background_viewer_quit(){
	int ret = 0;

	if((ret = mutex_lock(lock)) != 0)
		//use debug_error
		printf("background_viewer_send_quit() mutex lock failed: %d", ret);
		return ret;

	quit = 1;

	mutex_unlock(lock);

	if((ret = thread_join(thread)) != 0) {
		//use debug_error
		printf("background_viewer_send_quit() thread join failed: %d", ret);
		return ret;
	}

	if((ret = mutex_destroy(lock)) != 0) {
		//use debug_error
		printf("background_viewer_send_quit() mutex_destroy failed: %d", ret);
		return ret;
	}

	return ret;
}