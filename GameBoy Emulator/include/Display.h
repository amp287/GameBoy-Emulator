#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Use to initialize the window display library
// Must be called before display_create_window
int display_init();

typedef struct WINDOW_HANDLE {
    int window_height;
    int window_width;
    int scale;
    GLFWwindow *window_handle;
} WINDOW_HANDLE;
// Create a OpenGL window
WINDOW_HANDLE *display_create_window(int width, int height, const char* name, GLFWkeyfun key_callback, int scale);

void display_update_buffer(WINDOW_HANDLE *display,const GLvoid *buffer, int width, int height);

void display_poll_events(WINDOW_HANDLE *display);

void display_destroy(WINDOW_HANDLE *display);