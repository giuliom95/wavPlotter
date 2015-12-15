#include "graphics.h"

int GLFW_init( int screen_w, int screen_h ) {
	
	int success = TRUE;

	if (!glfwInit()) {
		fprintf( stderr, "Error in the glfwInit() function!\n");
		success = FALSE;
	}
	
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
	
	//Anti-aliasing
	glfwWindowHint( GLFW_AUX_BUFFERS, 1 );
	glfwWindowHint( GLFW_SAMPLES, 4 );
	
	if( !initGL() ) {
		fprintf( stderr, "Error in the OpenGL init!\n");
		success = FALSE;
	}
	
	GLFW_window = glfwCreateWindow( screen_w, screen_h, "wavPlotter", NULL, NULL);
	
	if( !GLFW_window ){
		fprintf( stderr, "Error in the window creation!\n");
		glfwTerminate();
		success = FALSE;
	}
	
	glfwMakeContextCurrent( GLFW_window );
	
	glfwSwapInterval( 1 );
	
	return success;
}

int initGL() {

	int success = TRUE;
	GLenum error = GL_NO_ERROR;

	//Initialize Projection Matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	//Check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		fprintf( stderr, "Error initializing OpenGL! %s\n", gluErrorString( error ) );
		success = FALSE;
	}

	//Initialize Modelview Matrix
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	//Check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		fprintf( stderr, "Error initializing OpenGL! %s\n", gluErrorString( error ) );
		success = FALSE;
	}
	
	//Initialize clear color
	glClearColor( 0.f, 0.f, 0.f, 1.f );
	
	//Check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		fprintf( stderr, "Error initializing OpenGL! %s\n", gluErrorString( error ) );
		success = FALSE;
	}
	
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	
	return success;
}

void GLFW_close() {

	glfwDestroyWindow( GLFW_window );
	GLFW_window = NULL;
	glfwTerminate();
}
