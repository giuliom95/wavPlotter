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

void error_callback( int error, const char* description );

static void GLFW_key_callback( 
	GLFWwindow* window, 
	int key, 
	int scancode, 
	int action, 
	int mods 
);

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

long position;
int width;

int main() {
	
	//The two channels of a single file.
	int16_t* left;
	int16_t* right;
	
	//The number of total 16-bit samples that compose a single channel.
	long total_samples;
	
	const int screen_width = SCREEN_WIDTH;
	const int screen_height = SCREEN_HEIGHT;
	
	//The pointer to the wav file
	FILE* input_file;
	
	position = 0;
	width = BASE_WIDTH;
	
	
	//Creates the window and the OpenGL context.
	glfwSetErrorCallback( error_callback );
	if( !GLFW_init( screen_width, screen_height ) ) {
		fprintf( stderr, "Error while initializing GLFW!\n" );
		return -1;	
	}
	glfwSetKeyCallback( GLFW_window, GLFW_key_callback );
	
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
	
	while( !glfwWindowShouldClose( GLFW_window ) ) {
		
		print_info( position, total_samples, width );
		plot( left, right, position, total_samples, width, screen_width, screen_height );
		
		 glfwWaitEvents();
	}
	
	//Frees the memory allocated for the channels arrays.
	free(left);
	free(right);
	
	//Shuts down ncurses.
	endwin();
	
	//Shuts down GLFW and OpenGL.
	GLFW_close();
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
		pixel_per_sample * 5, 
		screen_w - pixel_per_sample * 5, 
		screen_h, 0.0f, 1.0f, -1.0f 
	);
	
	//Plots the time axis.
	glBegin( GL_LINES );
		glColor3ub( 150, 150, 150 );
		glVertex2f( 0, screen_h / 2 );
		glVertex2f( screen_w, screen_h / 2 );
	glEnd();
	
	//NOTE: The FFT algorithm which I've talked about here: 
	// http://giuliom95.tumblr.com/post/134427522354/
	// will be implemented in those two next cycles.
	
	//Plots the left channel in red. 
	glBegin( GL_LINE_STRIP );
		glColor3ub( 255, 100, 100 );
		for( i = pos; i < pos + screen_w; i+=1 ) {
			glVertex2f( i - pos, screen_h / 2 + left_ch[i] * DEPTH );
		} 	
	glEnd();

	//Plots the right channel in green.
	glBegin( GL_LINE_STRIP );
		glColor3ub( 100, 255, 100 );
		for( i = pos; i < pos + screen_w; i+=1 ) {
			glVertex2f( i - pos, screen_h / 2 + right_ch[i] * DEPTH );
		}	
	glEnd();	
	
	glfwSwapBuffers( GLFW_window );
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

void error_callback( int error, const char* description ){
    fputs( description, stderr );
}

static void GLFW_key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ){
	
	if( action == GLFW_REPEAT || action == GLFW_PRESS ){
		switch( key ) {
			case GLFW_KEY_RIGHT:
				position += BASE_SCROLL_SPEED;
				break;
			case GLFW_KEY_LEFT:
				position -= BASE_SCROLL_SPEED;
				
				if( position < 0 ) {
					position = 0;
				}
				break;
			case GLFW_KEY_S:
				width++;
				break;
			case GLFW_KEY_A:
				width--;
				break;
		}
	}
}
