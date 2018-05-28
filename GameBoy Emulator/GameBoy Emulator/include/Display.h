#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Use to initialize the window display library
// Must be called before display_create_window
int display_init();

// Create a OpenGL window
GLFWwindow *display_create_window(int width, int height, const char* name, GLFWkeyfun key_callback);

void display_update_buffer(GLFWwindow *display,const GLvoid *buffer, int width, int height);

void display_poll_events(GLFWwindow *display);

void display_destroy(GLFWwindow *display);