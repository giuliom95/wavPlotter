//The main file for the wavPlotter program.
#include "wavPlotter.h"

int main( int argc, char **argv ){

	const int screen_width = SCREEN_WIDTH;
	const int screen_height = SCREEN_HEIGHT;
	
	LIST* l;
	LIST* r;
	
	position = 0;
	width = BASE_WIDTH;
	
	right_color = DEF_RIGHT_COLOR;
	left_color = DEF_LEFT_COLOR;
	
	left = NULL;
	right = NULL;
	
	for( argc--; argc > 0; argc-- ) {
		if( parse_arg( argv[ argc ] ) != 0 ) {
			return -1;
		}
	}
	
	if( right == NULL && left == NULL ) {
		fprintf( stderr, "Error: No input file provided!\n" );
		print_help();
		return -1;
	}
	
	//Creates the window and the OpenGL context.
	glfwSetErrorCallback( GLFW_error_callback );
	if( !GLFW_init( screen_width, screen_height ) ) {
		fprintf( stderr, "Error while initializing GLFW!\n" );
		return -1;
	}
	glfwSetKeyCallback( GLFW_window, GLFW_key_callback );
	glfwSetScrollCallback( GLFW_window, GLFW_scroll_callback );
	
	//Initializes the ncurses library.
	initscr();
	
	while( !glfwWindowShouldClose( GLFW_window ) ) {
		
		if( position < 0 )
			position = 0;
		else if( position > total_samples )
			position = total_samples - 1;
		
		if( width < 1 )
			width = 1;
		else if( width > MAX_WIDTH )
			width = MAX_WIDTH;
		
		print_info( position, total_samples, width );
		
		l = left;
		r = right;
		
		while( l != NULL ) {
			plot( l->data, r->data, position, total_samples, width, screen_width, screen_height );
			l = l->next;
			r = r->next;
		}
		printf( "\n" );
		
		glfwWaitEvents();
	}
	
	//Frees the memory allocated for the channels arrays.
	free(left->data);
	free(right->data);
	free(left);
	free(right);
	
	close_all();
}

int parse_arg( char* arg ){
	
	if( arg[0] == '-' ) {
		if( arg[1] == 'c' ) {
			if( arg[2] == 'r' ) {
				sscanf( arg, "-cr%x", &right_color );
			} else if ( arg[2] == 'l' ) {
				sscanf( arg, "-cl%x", &left_color );
			}
		} else if( arg[1] == 'h' ) {
			print_help();
			return -1;
		} else if( arg[1] == 'p' ) {
			sscanf( arg, "-p%li", &position );
		} else if( arg[1] == 's' ) {
			sscanf( arg, "-s%li", &position );
			position *= SAMPLE_FREQUENCY;
		} else {
			fprintf( stderr, "Error: Argument not recognised!\n" );
			print_help(); 
			return -1;
		}
	} else {
		FILE* input_file;
		
		input_file = fopen( arg, "r" );
		
		total_samples = get_samples_number( input_file );
		
		
		
		if( left == NULL ) {
			printf("NULL\n");
			
			left = (LIST*) malloc( sizeof( LIST ) );
			right = (LIST*) malloc( sizeof( LIST ) );
		
			//Allocates the needed memory for the channels arrays. 
			left->data = (int16_t*) calloc( total_samples, sizeof(int16_t) );
			right->data = (int16_t*) calloc( total_samples, sizeof(int16_t) );

			read_samples( input_file, left->data, right->data, total_samples );
			
		} else {
			LIST* l = left;
			LIST* r = right;
					
			while( l->next != NULL ) {
				l = l->next;
				r = r->next;
			} 
		
			l->next = (LIST*) malloc( sizeof( LIST ) );
			r->next = (LIST*) malloc( sizeof( LIST ) );
		
			//Allocates the needed memory for the channels arrays. 
			l->next->data = (int16_t*) calloc( total_samples, sizeof(int16_t) );
			r->next->data = (int16_t*) calloc( total_samples, sizeof(int16_t) );

			read_samples( input_file, l->next->data, r->next->data, total_samples );
		}
		
		//Closes the file.
		fclose( input_file );
	}
	
	return 0;
}

void print_help() {
		printf( "\nwavPlotter: plots the waveform of a 16-bit signed dual channel WAVE file.\n\n" );
		printf( "Usage: wavPlotter <wav file> [OPTIONS]\n" );
		printf( "Available options:\n" );
		printf( " -h		Prints this info and exits.\n" );
		printf( " -crCOLOR	Changes the color for the right channel. COLOR is an HTML color\n");
		printf( "		 code.\n" );
		printf( " -clCOLOR	Changes the color for the left channel. COLOR is an HTML color\n" );
		printf( "		 code.\n" );
		printf( " -pSAMPLE	Puts the given SAMPLE at the center of the window at start-up\n" );
		printf( " -pSECOND	Moves the sample at given SECOND in the middle of the window at\n" );
		printf( "		 start-up\n" );
		printf( "\n" );
}

void close_all() {
	//Shuts down ncurses.
	endwin();
	
	//Shuts down GLFW and OpenGL.
	GLFW_close();
}

void print_info( long pos, long samples, int zoom ){
	move( 0, 0 );
	printw(
		"Position: %i/%i (%fs/%fs)\n", 
		pos, samples, 
		(double) pos / SAMPLE_FREQUENCY, 
		(double) samples / SAMPLE_FREQUENCY 
	);
	printw( "Zoom level: %i\n", zoom );
	refresh();
}

void plot( 
	int16_t* left_ch, 
	int16_t* right_ch, 
	long pos, 
	long samples, 
	int zoom,
	int screen_w,
	int screen_h 
) {
	printf( "PLOT" );
	int i;
	int half_screen_w = screen_w / 2;
	int half_screen_h = screen_h / 2;
	
	//Cleans the screen
	glClear( GL_COLOR_BUFFER_BIT );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho( 
		zoom * 5, 
		screen_w - zoom * 5, 
		screen_h, 0.0f, 1.0f, -1.0f 
	);
	
	//Plots the time axis.
	glBegin( GL_LINES );
		glColor3ub( 150, 150, 150 );
		glVertex2f( 0, half_screen_h );
		glVertex2f( screen_w, half_screen_h );
	glEnd();
	
	//Plots the left channel. 
	glBegin( GL_LINE_STRIP );
		glColor3ub(
			(left_color & 0xff0000) >> 16,
			(left_color & 0x00ff00) >> 8,
			left_color & 0x0000ff
		);
		for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
			if( i >= 0 && i < samples )
				glVertex2f( i - pos + half_screen_w, half_screen_h + left_ch[i] * DEPTH );
		} 	
	glEnd();

	//Plots the right channel.
	glBegin( GL_LINE_STRIP );
		glColor3ub(
			(right_color & 0xff0000) >> 16,
			(right_color & 0x00ff00) >> 8,
			right_color & 0x0000ff
		);
		for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
			if( i >= 0 && i < samples )
				glVertex2f( i - pos + half_screen_w, half_screen_h + right_ch[i] * DEPTH );
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

void GLFW_error_callback( int error, const char* description ){
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

void GLFW_scroll_callback( GLFWwindow* window, double xoffset, double yoffset ){

	if( glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS ){
		width += (int) yoffset;
	} else {
		if( glfwGetKey( window, GLFW_KEY_LEFT_ALT ) == GLFW_PRESS )
			position -= (long) yoffset * BASE_SCROLL_SPEED * 10;
		else 
			position -= (long) yoffset * BASE_SCROLL_SPEED;
	}
}

