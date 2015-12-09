//The main file for the wavPlotter program.

#include "SDL.h"

//The window initial dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BASE_SCROLL_SPEED 100

//These two define the axis ratio of the plot.
//BASE_WIDTH indicates the initial distance (in pixels) between two samples.
#define BASE_WIDTH 1

//DEPTH is the amplitude multiplier of the signal. 
//It squeezes the plot into the window. It was obtained by the proportion:
// 1 : 2^16 = DEPTH : SCREEN_HEIGHT 
#define DEPTH 0.009155273f

//The length(in bytes) of the header of a standard WAV file.
#define WAV_HEADER 44

//The two channels of a single file.
int16_t* left;
int16_t* right;

//The number of total 16-bit samples that compose a single channel.
long total_samples;

//An iterator. It will be used a lot of times, so I have declared it here
// once for all.
int i;

uint32_t position;
uint16_t width;

//TODO: Add specifications of these two methods.
void plot();
void print_info();

int main() {
	
	//The pointer to the wav file
	FILE* input_file;
	
	SDL_Event e;
	
	uint8_t* currentKeyStates;
	
	int scroll_speed;
	
	
	short exit, queue_length;
	
	//These variables are needed to make the OpenGL and SDL libraries work. 
	gWindow = NULL;
	gContext = NULL;
	screenWidth = SCREEN_WIDTH;
	screenHeight = SCREEN_HEIGHT;
	
	position = 0;
	width = BASE_WIDTH;
	
	//Creates the window and the OpenGL context.
	SDL_init();
	
	//Initializes the ncurses library.
	initscr();
	
	//Opens the input file.
	input_file = fopen( "./a.wav", "r" );
	//Moves the cursor to the EOF.
	fseek( input_file, 0, SEEK_END );
	
	//Calculates the total samples per channel in this way: it reads the actual
	// cursor position (the EOF) and then it subtracts the bytes used as header 
	// in the WAV. The resulting number is the amount of bytes of actual data
	// so, in order to get the samples, it must be divided by 4, because the channels
	// are two and every sample is made up by 2 bytes (16-bit). 
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
		left[i] =  (uint8_t)fgetc(input_file) + (uint16_t)( fgetc(input_file) << 8 );
		right[i] = (uint8_t)fgetc(input_file) + (uint16_t)( fgetc(input_file) << 8 );
	}
	
	//Closes the file.
	fclose( input_file );
		
	//And now the plotting part.
	plot();
	exit = 0;
	while( !exit ) {
		
		queue_length = 0;
		while( SDL_PollEvent( &e ) != 0 ) {
			
			if( queue_length < 5 ) {
			
				currentKeyStates = SDL_GetKeyboardState( NULL );			
					
				if( e.type == SDL_QUIT ) {
					exit = 1;
				} else if( e.type == SDL_MOUSEWHEEL ) {
				
					//e.wheel.y is "the amount scrolled vertically, positive 
					// away from the user and negative toward the user" 
					// (from http://wiki.libsdl.org/SDL_MouseWheelEvent).
					//So, if the user scrolls down, the camera will move to right
					// and vice-versa.
					//TODO: Add an upper limitation to the scrolling.
				
					//This occurs when the user uses the mouse wheel.
					if( e.wheel.y != 0 ) {
					
						if( currentKeyStates[ SDL_SCANCODE_LSHIFT ] ) {
						
							if( e.wheel.y > 0 ) {
								width++;
							} else if( width > 1 ) {
								width--;
							}
						
							plot();
						
						} else {
							scroll_speed = BASE_SCROLL_SPEED;
					
							if( currentKeyStates[ SDL_SCANCODE_LCTRL ] )
								scroll_speed /= 3;
				
							if( e.wheel.y > 0 )
								if( position > scroll_speed )
									scroll_speed *= -1;
								else
									scroll_speed = 0;
				
							position += scroll_speed;
							plot();
						}
				
					}
				
				} else if( e.type == SDL_KEYDOWN ) {
					if( e.key.keysym.sym == SDLK_PLUS ) {
						width++;
						plot();
					} else if( e.key.keysym.sym == SDLK_MINUS && width > 1 ) {
						width--;
						plot();
					}
				}
			}
			
			queue_length++;
		}
		
		SDL_Delay(40);
	}
	
	//Frees the memory allocated for the channels arrays.
	free(left);
	free(right);
	
	//Shuts down ncurses.
	endwin();
	
	//Shuts down SDL on OpenGL things.
	SDL_close();
}

void print_info() {
	move( 0, 0 );
	printw( "Position: %i/%i\n", position, total_samples );
	printw( "Pixels per sample: %i\n", width );
	refresh();
}

void plot() {
	//Prints info on terminal.
	print_info();
	
	//Cleans the screen
	glClear( GL_COLOR_BUFFER_BIT );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho( 
		position + width * 5, 
		screenWidth + position - width * 5, 
		screenHeight, 0.0f, 1.0f, -1.0f 
	);
	
	//Plots the time axis.
	glBegin( GL_LINES );
		glColor3ub( 150, 150, 150 );
		glVertex2f( position, screenHeight / 2 );
		glVertex2f( screenWidth + position, screenHeight / 2 );
	glEnd();
	
	//NOTE: The FFT algorithm which I've talked about here: 
	// http://giuliom95.tumblr.com/post/134427522354/
	// will be implemented in those two next cycles.
	
	//Plots the left channel in red. 
	glBegin( GL_LINE_STRIP );
		glColor3ub( 255, 100, 100 );
		for( i = 0; i < total_samples; i+=1 ) {
			glVertex2f( i, screenHeight / 2 + left[i] * DEPTH );
		} 	
	glEnd();

	//Plots the right channel in green.
	glBegin( GL_LINE_STRIP );
		glColor3ub( 100, 255, 100 );
		for( i = 0; i < total_samples; i+=1 ) {
			glVertex2f( i, screenHeight / 2 + right[i] * DEPTH );
		}	
	glEnd();	
	
	SDL_GL_SwapWindow( gWindow );
}
