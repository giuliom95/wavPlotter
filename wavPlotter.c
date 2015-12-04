//The main file for the wavPlotter program.

#include "SDL.h"

//The window initial dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

//These two define the axis ratio of the plot.
#define WIDTH 10
#define DEPTH 0.01f

//The lenght(in bytes) of the header of a standard WAV file.
#define WAV_HEADER 44

int main() {
	
	//The two channels of a single file.
	int16_t* left;
	int16_t* right;
	
	//The number of total 16-bit samples that compose a single channel.
	int total_samples;
	
	//An iterator. It will be used a lot of times, so I've declared it here
	// once for all.
	int i;
	
	//The pointer to the wav file
	FILE* input_file;
	
	//Those variables are needed to make the OpenGL and SDL libraries work. 
	gWindow = NULL;
	gContext = NULL;
	screenWidth = SCREEN_WIDTH;
	screenHeight = SCREEN_HEIGHT;
	
	//Creates the window and the OpenGL contex.
	SDL_init();
	
	//Opens the input file.
	input_file = fopen( "./a.wav", "r" );
	//Moves the cursor to the EOF.
	fseek( input_file, 0, SEEK_END );
	
	//Calculates the total samples per channel in this way: it reads the actual
	// cursor position (the EOF) and then it subtracts the bytes used as header 
	// in the WAV. The resulting number is the amount of bytes of actual data
	// so, in order to get the samples, it must be divided by 4, because the channels
	// are two and every samble is made up by 2 bytes (16-bit). 
	total_samples = ( ftell( input_file ) - WAV_HEADER ) / 4;
	
	//Brings back the cursor to the head of the file.
	fseek( input_file, 0, 0 );
	
	//Allocates the needed memory for the channels arrays. 
	left = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	right = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	
	//Skips the header of the file.
	for( i == 0; i < WAV_HEADER; i++ ) {
		fgetc( input_file );
	}
	
	//Reads the whole input file and stores its values in the channels arrays.
	//Here a simple reference on how data is stored in a dual channel
	// 16-bit WAV file:
	// 
	// Decimal:       1094      3302       96       31296      270
	//                  |         |         |         |         |
	//                  V         V         V         V         V
	// Exadecimal:  0x04 0x46 0x0C 0xE6 0x00 0x60 0x7A 0x40 0x01 0x0E ... And so on
	//              |_______| |_______| |_______| |_______| |_______|
	// Channel:        Left     Right      Left     Right      Left
	//
	//I got this info from: http://soundfile.sapp.org/doc/WaveFormat/ 
	// Check it out further information.
	for( i = 0; i < total_samples; i++ ) {
		left[i] = (uint8_t)fgetc( input_file ) + (uint16_t)( fgetc( input_file ) << 8 );	
		right[i] = (uint8_t)fgetc( input_file ) + (uint16_t)( fgetc( input_file ) << 8 );		
	}
	
	//And now the plotting part.
	
	//First of all the time axis.
	glBegin( GL_LINES );
		glColor3ub( 150, 150, 150 );
		glVertex2f( 0, screenHeight / 2 );
		glVertex2f( screenWidth, screenHeight / 2 );
	glEnd();
	
	//NOTE: The FFT algorithm which I've talked about here: 
	// http://giuliom95.tumblr.com/post/134427522354/
	// will be implemented in those two next for cycles.
	
	//Then the left channel in red. 
	glBegin( GL_LINE_STRIP );
		glColor3ub( 255, 100, 100 );
		for( i = 0; i < total_samples; i+=WIDTH ) {
			glVertex2f( i, screenHeight / 2 + left[i/WIDTH] * DEPTH );
		} 	
	glEnd();
	
	//And finally the right channel in green.
	glBegin( GL_LINE_STRIP );
		glColor3ub( 100, 255, 100 );
		for( i = 0; i < total_samples; i+=WIDTH ) {
			glVertex2f( i, screenHeight / 2 + right[i/WIDTH] * DEPTH );
		}	
	glEnd();
	
	//Swaps buffers and actually prints the waveforms to the screen.
	SDL_GL_SwapWindow( gWindow );
	
	//Keeps open the window for 3sec.
	//NOTE: The progam exit is planned (obviously) to be user-driven.
	SDL_Delay(3000);
	
	//Deallocates the memory allocated for the channels arrays.
	free(left);
	free(right);
	
	//Closes the file.
	fclose( input_file );
	
	//Shuts down SDL on OpenGL things.
	SDL_close();
}
