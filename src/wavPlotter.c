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
	plot_spectrum = FALSE;
	
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
				*l,
				position,
				width, 
				screen_width, 
				screen_height 
			);
			
			l = l->next;
		}
		glfwSwapBuffers( GLFW_window );

		glfwWaitEvents();
	}
	
	//Frees the memory allocated for every WAVE file loaded
	l = list;
	while( l != NULL ) {
		free( l->right );
		free( l->left );
		fftw_free( l->left_spectrum );
		fftw_free( l->right_spectrum );
		l = l->next;
	}
	free(list);
	
	//Shuts down ncurses.
	endwin();
	
	//Shuts down GLFW and OpenGL.
	GLFW_close();
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
		List* l;
		double* left_double;
		double* right_double;
		int i;
		fftw_plan left_plan;
		fftw_plan right_plan;
		
		input_file = fopen( arg, "r" );
		
		l = list;
		
		if( list != NULL ) {

			while( l->next != NULL ) {
				l = l->next;
			}
			
			l->next = (List*) malloc( sizeof( List ) );
			l = l->next;
			
			l->left_color = DEF_LEFT_COLOR_ALT;
			l->right_color = DEF_RIGHT_COLOR_ALT; 
		} else {
		
			list = (List*) malloc( sizeof( List ) );
			l = list;
			
			l->left_color = DEF_LEFT_COLOR;
			l->right_color = DEF_RIGHT_COLOR;
		}
		
		l->next = NULL;
		
		l->total_samples = get_samples_number( input_file );
		
		//Allocates the needed memory for the channels arrays. 
		l->left = (int16_t*) calloc( l->total_samples, sizeof(int16_t) );
		l->right = (int16_t*) calloc( l->total_samples, sizeof(int16_t) );
		
		l->left_spectrum = (fftw_complex*) fftw_malloc( sizeof( fftw_complex ) * ( l->total_samples / 2 + 1 ) );
		l->right_spectrum = (fftw_complex*) fftw_malloc( sizeof( fftw_complex ) * ( l->total_samples / 2 + 1 ) );
		
		left_double = (double*) calloc( l->total_samples, sizeof( double ) );
		right_double = (double*) calloc( l->total_samples, sizeof( double ) );
		
		read_samples( input_file, l->left, l->right, l->total_samples );
		
		for( i = 0; i < l->total_samples; i++ ){
			left_double[i] = l->left[i];
			right_double[i] = l->right[i];
		}
		
		for( i = 0; i < ( l->total_samples / 2 + 1 ); i++ ) {
			l->left_spectrum[i][0] = 0;
			l->left_spectrum[i][1] = 0;
			l->right_spectrum[i][0] = 0;
			l->right_spectrum[i][1] = 0;
		}
		
		left_plan = fftw_plan_dft_r2c_1d( l->total_samples, left_double, l->left_spectrum, FFTW_ESTIMATE );		
		right_plan = fftw_plan_dft_r2c_1d( l->total_samples, right_double, l->right_spectrum, FFTW_ESTIMATE );
		
		fftw_execute( left_plan );
		fftw_execute( right_plan );
		
		fftw_destroy_plan( left_plan );
		fftw_destroy_plan( right_plan );
		
		free( left_double );
		free( right_double );
		
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
		printf( " -pSAMPLE	Puts the given SAMPLE at the center of the window at start-up\n" );
		printf( " -pSECOND	Moves the sample at given SECOND in the middle of the window at\n" );
		printf( "		 start-up\n" );
		printf( "\n" );
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
	if( plot_spectrum )
		printw( "[SPECTRUM]" );
	else
		clrtoeol();
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
	
	if( !plot_spectrum ) {
	
		//Plots the time axis.
		glBegin( GL_LINES );
			glColor3ub( 150, 150, 150 );
			glVertex2f( 0, screen_h / 2 );
			glVertex2f( screen_w, screen_h /2 );
		glEnd();
		
	} else {
		//Plots two time axes.
		glBegin( GL_LINES );
			glColor3ub( 150, 150, 150 );
			glVertex2f( 0, screen_h / 3 );
			glVertex2f( screen_w, screen_h / 3 );
			glVertex2f( 0, screen_h * 2 / 3 );
			glVertex2f( screen_w, screen_h * 2 / 3 );
		glEnd();
	}
}

void plot( 
	List l, 
	long pos,
	int zoom,
	int screen_w,
	int screen_h
) {

	int i;
	int half_screen_w = screen_w / 2;
	int half_screen_h = screen_h / 2;
	
	if( !plot_spectrum ) {
		//Plots the left channel. 
		glBegin( GL_LINE_STRIP );
			glColor3ub(
				(l.left_color & 0xff0000) >> 16,
				(l.left_color & 0x00ff00) >> 8,
				l.left_color & 0x0000ff
			);
			for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
				if( i >= 0 && i < l.total_samples )
					glVertex2f( i - pos + half_screen_w, half_screen_h + l.left[i] * DEPTH );
			} 	
		glEnd();

		//Plots the left channel. 
		glBegin( GL_LINE_STRIP );
			glColor3ub(
				(l.right_color & 0xff0000) >> 16,
				(l.right_color & 0x00ff00) >> 8,
				l.right_color & 0x0000ff
			);
			for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
				if( i >= 0 && i < l.total_samples )
					glVertex2f( i - pos + half_screen_w, half_screen_h + l.right[i] * DEPTH );
			} 	
		glEnd();
	} else {
		
		long spectrum_samples = l.total_samples / 2 + 1;
		
		//Plots the real part of the left spectrum. 
		glBegin( GL_LINE_STRIP );
			glColor3ub(
				(l.left_color & 0xff0000) >> 16,
				(l.left_color & 0x00ff00) >> 8,
				l.left_color & 0x0000ff
			);
			for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
				if( i >= 0 && i < spectrum_samples )
					glVertex2f( i - pos + half_screen_w, (screen_h / 3) + l.left_spectrum[i][0] * SPECTRUM_DEPTH / 2);
			} 	
		glEnd();

		//Plots the immaginary part of the left spectrum. 
		glBegin( GL_LINE_STRIP );
			glColor3ub(
				(l.right_color & 0xff0000) >> 16,
				(l.right_color & 0x00ff00) >> 8,
				l.right_color & 0x0000ff
			);
			for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
				if( i >= 0 && i < spectrum_samples )
					glVertex2f( i - pos + half_screen_w, (screen_h / 3) + l.right_spectrum[i][0] * SPECTRUM_DEPTH );
			} 	
		glEnd();
		
		//Plots the real part of the left spectrum. 
		glBegin( GL_LINE_STRIP );
			glColor3ub(
				(l.left_color & 0xff0000) >> 16,
				(l.left_color & 0x00ff00) >> 8,
				l.left_color & 0x0000ff
			);
			for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
				if( i >= 0 && i < spectrum_samples )
					glVertex2f( i - pos + half_screen_w, (screen_h * 2 / 3) + l.left_spectrum[i][1] * SPECTRUM_DEPTH );
			} 	
		glEnd();

		//Plots the immaginary part of the left spectrum. 
		glBegin( GL_LINE_STRIP );
			glColor3ub(
				(l.right_color & 0xff0000) >> 16,
				(l.right_color & 0x00ff00) >> 8,
				l.right_color & 0x0000ff
			);
			for( i = pos - half_screen_w + zoom; i < pos + half_screen_w - zoom; i++ ) {
				if( i >= 0 && i < spectrum_samples )
					glVertex2f( i - pos + half_screen_w, (screen_h * 2 / 3) + l.right_spectrum[i][1] * SPECTRUM_DEPTH );
			} 	
		glEnd();
		
	}
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
			case GLFW_KEY_D:
				width++;
				break;
			case GLFW_KEY_A:
				width--;
				break;
			case GLFW_KEY_S:
				plot_spectrum = !plot_spectrum;
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
		else if( glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS )
			position -= (long) yoffset * BASE_SCROLL_SPEED / 5;
		else
			position -= (long) yoffset * BASE_SCROLL_SPEED;
	}
}

