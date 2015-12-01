#ifndef SDL_H
#define SDL_H

#include "includes.h"

SDL_Window* gWindow;
SDL_GLContext gContext;

int screenWidth, screenHeight;

int SDL_initGL();
int SDL_init();
void SDL_close();

#endif
