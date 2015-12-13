//The main file for the wavPlotter program.

#include "includes.h"
#include "graphics.h"

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

//TODO: Add specifications of these two methods.
void plot( 
	int16_t* left_ch, 
	int16_t* right_ch, 
	long pos, 
	long samples, 
	int pixel_per_sample,
	int screen_w,
	int screen_h 
);
void print_info( long pos, long samples, int pixel_per_sample );

/* Returns the samples of a given WAVE file.
 * Pre: "fd" is a pointer to a 16-bit stereo WAVE file opened by the fopen function.
 * Post: The return is the samples number of the given WAVE. 
 *  Returns -1 in case of error.
 */
int get_samples_number( FILE* fd );

/* Reads data from a 16-bit stereo WAVE file and stores it in two arrays.
 * Pre: "fd" is a pointer to a 16-bit stereo WAVE file opened by the fopen function.
 *  "left_ch" and "right_ch" are two arrays with as many elements as samples.
 *  "samples" is the number returned by the "int get_samples_number( FILE* fd )"
 *   function.
 * Post: "left_ch" and "right_ch" are filled with the samples of the given WAVE file.
 */  
void read_samples( FILE* fd, int16_t* left_ch, int16_t* right_ch, long samples );

int main() {
	
	//The two channels of a single file.
	int16_t* left;
	int16_t* right;
	
	//The number of total 16-bit samples that compose a single channel.
	long total_samples;

	long position;
	int width;
	
	const int screen_width = SCREEN_WIDTH;
	const int screen_height = SCREEN_HEIGHT;
	
	//The pointer to the wav file
	FILE* input_file;
	
	SDL_Event e;
	
	int scroll_speed;
	
	short exit, queue_length;
	
	position = 0;
	width = BASE_WIDTH;
	
	//Creates the window and the OpenGL context.
	if( !SDL_init( screen_width, screen_height ) ) {
		return -1;	
	}
	
	//Initializes the ncurses library.
	initscr();
	
	//Opens the input file.
	input_file = fopen( "./a.wav", "r" );
	
	total_samples = get_samples_number( input_file );
	
	//Allocates the needed memory for the channels arrays. 
	left = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	right = (int16_t*) calloc( total_samples, sizeof(int16_t) );
	
	read_samples( input_file, left, right, total_samples );
	
	//Closes the file.
	fclose( input_file );
		
	//And now the plotting part.
	plot( left, right, position, total_samples, width, screen_width, screen_height );
	print_info( position, total_samples, width );
	
	exit = 0;
	while( !exit ) {
		
		queue_length = 0;
		while( SDL_PollEvent( &e ) != 0 ) {
			
			if( queue_length < 5 ) {
				
				char refresh = FALSE;
				const uint8_t* currentKeyStates = SDL_GetKeyboardState( NULL );			
					
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
							
							if( e.wheel.y != 0 ) {
							
								if( e.wheel.y > 0 && width < 79 ) {
									width++;
								} else if( width > 1 ) {
									width--;
								}
						
								refresh = TRUE;
							
							}
						
						} else {
							scroll_speed = BASE_SCROLL_SPEED;
					
							if( currentKeyStates[ SDL_SCANCODE_LCTRL ] ) {
								scroll_speed /= 3;
							}
							
							if( e.wheel.y > 0 ) {
								if( position > scroll_speed )
									scroll_speed *= -1;
								else {
									scroll_speed = 0;
									position = 0;
								}
							}
							
							position += scroll_speed;
							
							refresh = TRUE;
						}
				
					}
				
				} else if( e.type == SDL_KEYDOWN ) {
					if( e.key.keysym.sym == SDLK_PLUS && width < 79 ) {
						width++;
						refresh = TRUE;
					} else if( e.key.keysym.sym == SDLK_MINUS && width > 1 ) {
						width--;
						refresh = TRUE;
					}
				}
				
				if( refresh ){
					print_info( position, total_samples, width );
					plot( left, right, position, total_samples, width, screen_width, screen_height );
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

void print_info( long pos, long samples, int pixel_per_sample ){
	move( 0, 0 );
	printw( "Position: %i/%i\n", pos, samples );
	printw( "Pixels per sample: %i\n", pixel_per_sample );
	refresh();
}

void plot( 
	int16_t* left_ch, 
	int16_t* right_ch, 
	long pos, 
	long samples, 
	int pixel_per_sample,
	int screen_w,
	int screen_h 
) {
	int i;
	
	//Cleans the screen
	glClear( GL_COLOR_BUFFER_BIT );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho( 
		pos + pixel_per_sample * 5, 
		screen_w + pos - pixel_per_sample * 5, 
		screen_h, 0.0f, 1.0f, -1.0f 
	);
	
	//Plots the time axis.
	glBegin( GL_LINES );
		glColor3ub( 150, 150, 150 );
		glVertex2f( pos, screen_h / 2 );
		glVertex2f( screen_w + pos, screen_h / 2 );
	glEnd();
	
	//NOTE: The FFT algorithm which I've talked about here: 
	// http://giuliom95.tumblr.com/post/134427522354/
	// will be implemented in those two next cycles.
	
	//Plots the left channel in red. 
	glBegin( GL_LINE_STRIP );
		glColor3ub( 255, 100, 100 );
		for( i = 0; i < samples; i+=1 ) {
			glVertex2f( i, screen_h / 2 + left_ch[i] * DEPTH );
		} 	
	glEnd();

	//Plots the right channel in green.
	glBegin( GL_LINE_STRIP );
		glColor3ub( 100, 255, 100 );
		for( i = 0; i < samples; i+=1 ) {
			glVertex2f( i, screen_h / 2 + right_ch[i] * DEPTH );
		}	
	glEnd();	
	
	SDL_GL_SwapWindow( gWindow );
}

int get_samples_number( FILE* fd ){
	int samples;
	
	//Moves the cursor to the EOF.
	fseek( fd, 0, SEEK_END );
 
	//Calculates the total samples per channel in this way: it reads the actual
	// cursor position (the EOF) and then it subtracts the bytes used as header 
	// in the WAV. The resulting number is the amount of bytes of actual data
	// so, in order to get the samples, it must be divided by 4, because the channels
	// are two and every sample is made up by 2 bytes (16-bit). 
	samples = ( ftell( fd ) - WAV_HEADER ) / 4;

	//Brings back the cursor to the head of the file.
	fseek( fd, 0, 0 );
	
	return samples;
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
