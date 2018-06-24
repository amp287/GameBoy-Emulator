#include <stdio.h>
#include <string.h>
#include "Memory.h"
#include "PPU.h"
#include "PPU_Utils.h"
#include "Display.h"
#include "Background_Viewer.h"
#include "Utils.h"

// In pixels
#define TILE_PIXEL_SIZE 8

#define MAP_TILE_SIZE 32

#define TILE_BYTES 16
#define TILE_ROW_BYTES 2

int quit;
static GLFWwindow* background_window;
static const char *window_title = "Map Background Viewer";
static unsigned char buffer[256][256][3];

static void *lock;
static void *thread; 

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
		// Set quit logic here
	}
}

void background_viewer_update_screen() {
	unsigned char i, j;
	unsigned short pi, pj;
	unsigned short tile[8];

	for (i = 0; i < MAP_TILE_SIZE; i++) {
		for (j = 0; j < MAP_TILE_SIZE; j++) {
			int x, y;
			x = i * TILE_PIXEL_SIZE;
			y = j * TILE_PIXEL_SIZE;

			get_tile(tile, j, i);

			for (pi = 0; pi < TILE_PIXEL_SIZE; pi++) {
				for (pj = 0; pj < TILE_PIXEL_SIZE; pj++) {
					unsigned char color = get_pixel(tile[pi] << pj);
					if(mutex_lock(lock) == 0) {
						buffer[x + pi][y + pj][0] = color;
						buffer[x + pi][y + pj][1] = color;
						buffer[x + pi][y + pj][2] = color;
						mutex_unlock(lock);
					}
				}
			}
		}
	}
}


void *background_viewer_get_thread(){
	return thread;
}

void *background_viewer_run(void *arg){
	int *quit = arg;
	int quit_local = 0;

	while(!quit_local){
		background_viewer_update_screen();

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

	background_window = display_create_window(256, 256, window_title, key_callback);

	ret = thread_create(&thread, &background_viewer_run, &quit);//pthread_create(&thread, NULL, &background_viewer_run, &quit);

	if(ret != 0){
		printf("background_viewer_init() Thread creation failed (error:%d)\n",ret);
		return ret;
	}

	return 0;
}

void background_viewer_update() {
	if(mutex_lock(lock) == 0) {
		display_update_buffer(background_window, buffer, 256, 256);
		mutex_unlock(lock);
	}
	
	display_poll_events(background_window);
}

// Should be called on a different thread
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
