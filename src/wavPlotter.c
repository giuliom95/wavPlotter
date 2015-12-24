//The main file for the wavPlotter program.
#include "wavPlotter.h"

int main( int argc, char **argv ){

	const int screen_width = SCREEN_WIDTH;
	const int screen_height = SCREEN_HEIGHT;
	int arg;
	
	List* l;
	long maximum_samples;
	
	position = 0;
	width = BASE_WIDTH;
	
	srand( time( NULL ) );
	
	list = NULL;
	
	for( arg = 1; arg < argc; arg++ ) {
		if( parse_arg( argv[ arg ] ) != 0 ) {
			return -1;
		}
	}
	
	if( list == NULL ) {
		fprintf( stderr, "Error: No input file provided!\n" );
		print_help();
		return -1;
	}
	
	l = list;
	maximum_samples = l->total_samples;
	while( l != NULL ) {
		if( maximum_samples < l->total_samples )
			maximum_samples = l->total_samples;
		l = l->next;
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
		else if( position > maximum_samples )
			position = maximum_samples - 1;
		
		if( width < 1 )
			width = 1;
		else if( width > MAX_WIDTH )
			width = MAX_WIDTH;
		
		print_info( position, maximum_samples, width );
		
		l = list;
		
		pre_plot( width, screen_width, screen_height );
		while( l != NULL ) {
			plot( 
				l->left, 
				l->right, 
				position, 
				l->total_samples, 
				width, 
				screen_width, 
				screen_height, 
				l->left_color,
				l->right_color 
			);
			
			l = l->next;
		}
		glfwSwapBuffers( GLFW_window );
		
		glfwWaitEvents();
	}
	
	l = list;
	while( l != NULL ) {
		free( l->right );
		free( l->left );
		l = l->next;
	}
	
	//Frees the memory allocated for the channels arrays.
	free(list);
	
	close_all();
}

int parse_arg( char* arg ){
	
	if( arg[0] == '-' ) {
		if( arg[1] == 'h' ) {
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
		
		if( list == NULL ) {
			
			list = (List*) malloc( sizeof( List ) );
			
			list->total_samples = get_samples_number( input_file );
			
			list->left_color = DEF_LEFT_COLOR;
			list->right_color = DEF_RIGHT_COLOR;
			
			//Allocates the needed memory for the channels arrays. 
			list->left = (int16_t*) calloc( list->total_samples, sizeof(int16_t) );
			list->right = (int16_t*) calloc( list->total_samples, sizeof(int16_t) );

			read_samples( input_file, list->left, list->right, list->total_samples );
			
		} else {
			List* l = list;
					
			while( l->next != NULL ) {
				l = l->next;
			} 
		
			l->next = (List*) malloc( sizeof( List ) );
			
			l->next->total_samples = get_samples_number( input_file );
			
			l->next->left_color = DEF_LEFT_COLOR_ALT;
			l->next->right_color = DEF_RIGHT_COLOR_ALT;
			
			//Allocates the needed memory for the channels arrays. 
			l->next->left = (int16_t*) calloc( l->next->total_samples, sizeof(int16_t) );
			l->next->right = (int16_t*) calloc( l->next->total_samples, sizeof(int16_t) );

			read_samples( input_file, l->next->left, l->next->right, l->next->total_samples );
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

void pre_plot( int zoom, int screen_w, int screen_h ) {
	
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
		glVertex2f( 0, screen_h / 2 );
		glVertex2f( screen_w, screen_h /2 );
	glEnd();
}

void plot( 
	int16_t* left_ch, 
	int16_t* right_ch, 
	long pos, 
	long samples, 
	int zoom,
	int screen_w,
	int screen_h,
	Color l_col,
	Color r_col
) {

	int i;
	int half_screen_w = screen_w / 2;
	int half_screen_h = screen_h / 2;
	
	//Plots the left channel. 
	glBegin( GL_LINE_STRIP );
		glColor3ub(
			(l_col & 0xff0000) >> 16,
			(l_col & 0x00ff00) >> 8,
			l_col & 0x0000ff
		);
		for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
			if( i >= 0 && i < samples )
				glVertex2f( i - pos + half_screen_w, half_screen_h + left_ch[i] * DEPTH );
		} 	
	glEnd();

	//Plots the right channel.
	glBegin( GL_LINE_STRIP );
		glColor3ub(
			(r_col & 0xff0000) >> 16,
			(r_col & 0x00ff00) >> 8,
			r_col & 0x0000ff
		);
		for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
			if( i >= 0 && i < samples )
				glVertex2f( i - pos + half_screen_w, half_screen_h + right_ch[i] * DEPTH );
		}	
	glEnd();	
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

