#include "SDL.h"
#define WIDTH 3
#define DEPTH 2

int main() {
	
	signed char* a;
	int length;
	int i;
	FILE* input_file;
	
	gWindow = NULL;
	gContext = NULL;
	
	screenWidth = 800;
	screenHeight = 600;
	
	SDL_init();
	
	input_file = fopen( "./a.wav", "r" );
	fseek( input_file, 0, SEEK_END );
	length = ftell( input_file );
	fseek( input_file, 0, 0 );
	
	a = calloc( length, sizeof(signed char) );
	
	for( i = 0; i < length; i++ ) {
		a[i] = fgetc( input_file );
	}
	
	glBegin( GL_LINE_STRIP );
		glColor3ub( 255, 255, 255 );
		for( i = 0; i < length; i+=WIDTH ) {
			glVertex2f( i, screenHeight / 2 + a[i/WIDTH] * DEPTH );
		}	
	glEnd();
	
	SDL_GL_SwapWindow( gWindow );
	SDL_Delay(5000);
	
	fclose( input_file );
	
	SDL_close();
}
