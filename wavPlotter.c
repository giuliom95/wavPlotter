#include "SDL.h"
#define WIDTH 10
#define DEPTH 0.01f

int main() {
	
	int16_t* left;
	int16_t* right;
	int total_samples;
	int i;
	FILE* input_file;
	
	gWindow = NULL;
	gContext = NULL;
	
	screenWidth = 800;
	screenHeight = 600;
	
	SDL_init();
	
	input_file = fopen( "./a.wav", "r" );
	fseek( input_file, 0, SEEK_END );
	total_samples = ftell( input_file ) / 4;
	fseek( input_file, 0, 0 );
	
	left = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	right = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	
	for( i == 0; i < 44; i++ ) {
		fgetc( input_file );
	}
	
	for( i = 0; i < total_samples; i++ ) {
		left[i] = (uint8_t)fgetc( input_file ) + (uint16_t)( fgetc( input_file ) << 8 );
		
		right[i] = (uint8_t)fgetc( input_file ) + (uint16_t)( fgetc( input_file ) << 8 );		
	}
	
	glBegin( GL_LINE_STRIP );
		glColor3ub( 255, 100, 100 );
		for( i = 0; i < total_samples; i+=WIDTH ) {
			//printf( "L:(%x,%i) R:(%x,%i) \n", left[i], left[i], right[i], right[i] );
			glVertex2f( i, screenHeight / 2 + left[i/WIDTH] * DEPTH );
		} 	
	glEnd();
	
	glBegin( GL_LINE_STRIP );
		glColor3ub( 100, 255, 100 );
		for( i = 0; i < total_samples; i+=WIDTH ) {
			glVertex2f( i, screenHeight / 2 + right[i/WIDTH] * DEPTH );
		}	
	glEnd();
	
	SDL_GL_SwapWindow( gWindow );
	SDL_Delay(15000);
	
	free(left);
	free(right);
	
	fclose( input_file );
	
	SDL_close();
}
