#include "includes.h"
#include "graphics.h"

//The window initial dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BASE_SCROLL_SPEED 50

#define SAMPLE_FREQUENCY 44100

#define DEF_LEFT_COLOR 0xff6464
#define DEF_RIGHT_COLOR 0x64ff64

#define DEF_LEFT_COLOR_ALT 0x6464ff
#define DEF_RIGHT_COLOR_ALT 0x646400

//These two define the axis ratio of the plot.
//BASE_WIDTH indicates the initial distance (in pixels) between two samples.
#define BASE_WIDTH 1

#define MAX_WIDTH 79

//DEPTH is the amplitude multiplier of the signal. 
//It squeezes the plot into the window. It was obtained by the proportion:
// 1 : 2^16 = DEPTH : SCREEN_HEIGHT 
#define DEPTH 0.009155273f

//Same as above, but this time for the spectrum of the signal.
//It's simply DEPTH / 100
//TODO: Discover why it works this way.
#define SPECTRUM_DEPTH 0.00009155273f

//The length(in bytes) of the header of a standard WAV file.
#define WAV_HEADER 44

typedef uint32_t Color;

typedef struct _LIST {
	int16_t* left;
	int16_t* right;
	fftw_complex *left_spectrum;
	fftw_complex *right_spectrum;
	long total_samples;
	Color right_color;
	Color left_color;
	struct _LIST* next;
} List;

//TODO: Add specifications of these methods.
int parse_arg( char* arg );
void print_help();
void close_all();
void print_info( long pos, long samples, int pixel_per_sample );
void pre_plot( int zoom, int screen_w, int screen_h );
void plot( 
	List l, 
	long pos,
	int zoom,
	int screen_w,
	int screen_h
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

//TODO: Add specifications of these methods.
void GLFW_error_callback( int error, const char* description );

static void GLFW_key_callback( 
	GLFWwindow* window, 
	int key, 
	int scancode, 
	int action, 
	int mods 
);

void GLFW_scroll_callback( GLFWwindow* window, double xoffset, double yoffset );

long position;
int width;

//The two channels of a single file.
List* list;

short plot_spectrum;
