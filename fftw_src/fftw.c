//The main file for the wavPlotter program.

#include "includes.h"
#include "graphics.h"

//The window initial dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BASE_SCROLL_SPEED 100

//These two define the axis ratio of the plot.
//BASE_WIDTH indicates the initial distance (in pixels) between two samples.
#define BASE_WIDTH 10

//DEPTH is the amplitude multiplier of the signal. 
//It squeezes the plot into the window. It was obtained by the proportion:
// 1 : 2^16 = DEPTH : SCREEN_HEIGHT 
#define DEPTH 0.009155273f

//The length(in bytes) of the header of a standard WAV file.
#define WAV_HEADER 44

void read_samples( FILE* fd, int16_t* left_ch, int16_t* right_ch, long samples );

int main() {
	
	int i;
	
	//The two channels of a single file.
	int16_t* left;
	int16_t* right;
	
	double* fftw_left;
	double* left_original;
	
	fftw_complex *spectrum;
	fftw_plan r2c, c2r;
	
	//The number of total 16-bit samples that compose a single channel.
	long total_samples;
	
	int width;
	
	const int screen_width = SCREEN_WIDTH;
	const int screen_height = SCREEN_HEIGHT;
	
	//The pointer to the wav file
	FILE* input_file;
	
	short exit;
	
	SDL_Event e;
	
	width = BASE_WIDTH;
	
	//Creates the window and the OpenGL context.
	if( !SDL_init( screen_width, screen_height ) ) {
		return -1;
	}
	
	//Opens the input file.
	input_file = fopen( "./fftw.wav", "r" );
	
	total_samples = screen_width / width;
	
	left_original = (double*) calloc( total_samples, sizeof(double) ); 
	fftw_left = (double*) calloc( screen_width + 2, sizeof(double) );
	spectrum = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * ( screen_width + 1 ) / 2 );
	
	//Allocates the needed memory for the channels arrays. 
	left = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	right = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	
	read_samples( input_file, left, right, total_samples );
	
	//Closes the file.
	fclose( input_file );
	
	for( i = 0; i < ( screen_width + 1 ) / 2; i++ ) {
		spectrum[i][0] = 0;
		spectrum[i][1] = 0;
	}
	
	for( i = 0; i < total_samples; i++ ) {
		left_original[i] = left[i];
	}
	
	r2c = fftw_plan_dft_r2c_1d( total_samples, left_original, spectrum, FFTW_ESTIMATE );
	
	c2r = fftw_plan_dft_c2r_1d( screen_width, spectrum, fftw_left, FFTW_ESTIMATE );
	
	fftw_execute(r2c);
	
	printf( "%f + %fj\n", spectrum[ total_samples / 2 ][0], spectrum[ total_samples / 2 ][1] ); 
	
	fftw_execute(c2r);
	
	//Cleans the screen
	glClear( GL_COLOR_BUFFER_BIT );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho( 0.0f, screen_width, screen_height, 0.0f, 1.0f, -1.0f );
	
	//Plots the time axis.
	glBegin( GL_LINES );
		glColor3ub( 150, 150, 150 );
		glVertex2f( 0, screen_height / 3 );
		glVertex2f( screen_width, screen_height / 3 );
		
		glVertex2f( 0, screen_height * 2 / 3 );
		glVertex2f( screen_width, screen_height * 2 / 3 );
	glEnd();
	
	//Plots the left channel in red. 
	glBegin( GL_LINE_STRIP );
		glColor3ub( 255, 100, 100 );
		for( i = 0; i < total_samples; i+=1 ) {
			glVertex2f( i*width, screen_height / 3 + left_original[i] * DEPTH );
		} 	
	glEnd();
	
	glBegin( GL_LINE_STRIP );
		glColor3ub( 100, 255, 100 );
		for( i = 0; i < screen_width; i+=1 ) {
			glVertex2f( i, screen_height * 2 / 3 + fftw_left[i] * DEPTH * 0.01 );
		} 	
	glEnd();
	
	SDL_GL_SwapBuffers();
	
	exit = 0;
	while( !exit ) {
		
		while( SDL_PollEvent( &e ) != 0 ) {
				if( e.type == SDL_QUIT ) {
					exit = 1;
				} 
		}
		
		SDL_Delay(40);
	}
	
	//Frees the memory allocated for the channels arrays.
	free( left );
	free( right );
	
	free( left_original );
	
	fftw_free( fftw_left );
	
	fftw_destroy_plan( c2r );
	fftw_destroy_plan( r2c );
	
	//Shuts down SDL on OpenGL things.
	SDL_close();
}

void read_samples( FILE* fd, int16_t* left_ch, int16_t* right_ch, long samples ) {
	int i;
	
	//Skips the header of the file.
	for( i = 0; i < WAV_HEADER; i++ ) {
		fgetc( fd );
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
	for( i = 0; i < samples; i++ ) {
		left_ch[i] =  (uint8_t)fgetc( fd ) + (uint16_t)( fgetc( fd ) << 8 );
		right_ch[i] = (uint8_t)fgetc( fd ) + (uint16_t)( fgetc( fd ) << 8 );
	}
	
} 
