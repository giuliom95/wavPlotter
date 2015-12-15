#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"

GLFWwindow* GLFW_window;

int GLFW_init( int screen_w, int screen_h );
int initGL();
void GLFW_close();

#endif
