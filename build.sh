#!/bin/sh

gcc -o a src/wavPlotter.c src/SDL.c -lGL -lGLU -lSDL2 -lncurses -Wall -O3 -g
