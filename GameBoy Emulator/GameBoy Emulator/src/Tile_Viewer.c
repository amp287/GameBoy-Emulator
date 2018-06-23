#include <stdio.h>
#include <string.h>
#include "Memory.h"
#include "PPU_Utils.h"
#include "Display.h"
#include "Tile_Viewer.h"
#include "Utils.h"

#define WIDTH 128
#define HEIGHT 192

int quit;
static GLFWwindow* tile_window;
static const char *window_title = "Vram Tile Viewer";
static unsigned char buffer[HEIGHT][WIDTH][3];

void *lock;
void *thread;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
		tile_viewer_quit();
	}
}

void get_set_tile(unsigned short *tile_out, int tile) {
	unsigned short tile_set_start = 0x8000;
	int i;

	tile_set_start += tile * 16;

	for (i = 0; i < 8; i++)
		tile_out[i] = read_16_bit(tile_set_start + (i * 2));
}

void tile_viewer_update_screen() {
	unsigned char i, j;
	unsigned short pi, pj;
	unsigned short tile[8];
	int tile_count = 0;

	for (i = 0; i < 24; i++) {
		for (j = 0; j < 16; j++) {
			int x, y;
			x = i * 8;
			y = j * 8;

			get_set_tile(tile, tile_count++);

			for (pi = 0; pi < 8; pi++) {
				for (pj = 0; pj < 8; pj++) {
					unsigned char color = get_pixel(tile[pi] << pj);
		
					buffer[x + pi][y + pj][0] = color;
					buffer[x + pi][y + pj][1] = color;
					buffer[x + pi][y + pj][2] = color;
				}
			}
		}
	}
}

void tile_viewer_draw_screen() {
	tile_viewer_update_screen();
	display_poll_events(tile_window);
	display_update_buffer(tile_window, buffer, WIDTH, HEIGHT);
}

void *tile_viewer_run(void *arg) {
	int *quit = arg;
	int quit_local = 0;

	tile_window = display_create_window(WIDTH, HEIGHT, window_title, key_callback);

	while (!quit_local) {
		tile_viewer_draw_screen();

		if (mutex_lock(lock) == 0) {
			quit_local = *quit;
			mutex_unlock(lock);
		}
	}

	display_destroy(tile_window);

	return NULL;
}

int tile_viewer_init() {
	int ret = 0;
	quit = 0;

	ret = mutex_create(&lock);

	if (ret != 0) {
		return ret;
	}

	ret = thread_create(&thread, &tile_viewer_run, &quit);

	if (ret != 0) {
		printf("tile_viewer_init() Thread creation failed (error:%d)\n", ret);
		return ret;
	}

	return 0;
}

int tile_viewer_quit() {
	int ret = 0;

	if ((ret = mutex_lock(lock)) != 0)
		//use debug_error
		printf("tile_viewer_send_quit() mutex lock failed: %d", ret);
	return ret;

	quit = 1;

	mutex_unlock(lock);

	if ((ret = thread_join(thread)) != 0) {
		//use debug_error
		printf("tile_viewer_send_quit() thread join failed: %d", ret);
		return ret;
	}

	if ((ret = mutex_destroy(lock)) != 0) {
		//use debug_error
		printf("tile_viewer_send_quit() mutex_destroy failed: %d", ret);
		return ret;
	}

	return ret;
}