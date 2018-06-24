#include <stdio.h>
#include "Display.h"
#include "Utils.h"

int loaded = 0;

static void *lock;

static void error_callback(int error, const char* description)
{
	printf("%s\n", description);
}

// Only call this from the main thread!
int display_init(){
	int ret;

	if (!glfwInit()) {
		printf("ERROR\n");
		return -1;
	}

	loaded = 1;

	glfwSetErrorCallback(error_callback);

	ret = mutex_create(&lock);

	if(ret != 0) {
		printf("display_init(): mutex creation failed! (%d)\n", ret);
		return ret;
	}

	return 0;
}

// Only call this from the main thread!
GLFWwindow *display_create_window(int width, int height, const char* name, GLFWkeyfun key_callback) {
	GLFWwindow *display;

	if (!loaded) {
		printf("dispay error: GLFW not loaded...");
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	display = glfwCreateWindow(width, height, name, NULL, NULL);

	if (!display) {
		glfwTerminate();
		printf("Error!\n"); // Very descriptive 
		getchar();
		return NULL;
	}
	
	if(mutex_lock(lock) == 0) {

		glfwMakeContextCurrent(display);

		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		glfwSwapInterval(1);

		glfwSetKeyCallback(display, key_callback);

		mutex_unlock(lock);
	}
	
	return display;
}

void display_update_buffer(GLFWwindow *display, const GLvoid *buffer, int width, int height) {
	if(display == NULL)
		return;
	
	if(mutex_lock(lock) == 0) {
			
		if (glfwGetCurrentContext() != display)
			glfwMakeContextCurrent(display);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glRasterPos2f(-1, 1);
		glPixelZoom(1, -1);
		glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

		glfwSwapBuffers(display);

		mutex_unlock(lock);
	}
}

// Must be called from main thread
void display_poll_events(GLFWwindow *display) {
	if(display == NULL)
		return;
	
	if(mutex_lock(lock) == 0) {
			
		if(glfwGetCurrentContext() != display)
			glfwMakeContextCurrent(display);
		
		glfwPollEvents();

		mutex_unlock(lock);
	}
}

// only call from main thread!
void display_destroy(GLFWwindow *display) {
	glfwDestroyWindow(display);
}

// only call from main thread!
void display_cleanup(){
	glfwTerminate();
}