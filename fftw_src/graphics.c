#include "graphics.h"

int SDL_init( int screen_w, int screen_h ) {
	//Initialization flag
	int success = TRUE;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = FALSE;
	} else {
		
		//Anti-aliasing
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		

		if( SDL_SetVideoMode( screen_w, screen_h, 32, SDL_OPENGL ) == NULL ){
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = FALSE;
		} else {
			//Initialize OpenGL
			if( !SDL_initGL() ) {
				printf( "Unable to initialize OpenGL!\n" );
				success = FALSE;
			}
				
		}
	}

	return success;

}

int SDL_initGL() {

	int success = TRUE;
	GLenum error = GL_NO_ERROR;

	//Initialize Projection Matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	//Check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
		success = FALSE;
	}

	//Initialize Modelview Matrix
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	//Check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
		success = FALSE;
	}
	
	//Initialize clear color
	glClearColor( 0.f, 0.f, 0.f, 1.f );
	
	//Check for error
	error = glGetError();
	if( error != GL_NO_ERROR )
	{
		printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
		success = FALSE;
	}
	
	return success;
}

void SDL_close() {
	
	//Quit SDL subsystems
	SDL_Quit();
}
