#include <stdio.h>
#include "Display.h"

int loaded = 0;

static void error_callback(int error, const char* description)
{
	printf("%s\n", description);
}

int display_init(){

	if (!glfwInit()) {
		printf("ERROR\n");
		getchar(); //TODO Remove this after Testing
		return -1;
	}

	loaded = 1;

	glfwSetErrorCallback(error_callback);

	return 0;
}

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

	glfwMakeContextCurrent(display);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSwapInterval(1);

	glfwSetKeyCallback(display, key_callback);
	
	return display;
}

void display_update_buffer(GLFWwindow *display, const GLvoid *buffer, int width, int height) {
	if (glfwGetCurrentContext() != display)
		glfwMakeContextCurrent(display);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glRasterPos2f(-1, 1);
	glPixelZoom(1, -1);
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

	glfwSwapBuffers(display);
}

void display_poll_events(GLFWwindow *display) {
	if(glfwGetCurrentContext() != display)
		glfwMakeContextCurrent(display);
	
	glfwPollEvents();
}