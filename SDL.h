#ifndef SDL_H
#define SDL_H

#include "includes.h"

SDL_Window* gWindow;
SDL_GLContext gContext;

int SDL_initGL();
int SDL_init( int screen_w, int screen_h );
void SDL_close();

#endif
