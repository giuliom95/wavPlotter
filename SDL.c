#include "SDL.h"

int SDL_init() {
	//Initialization flag
	int success = TRUE;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = FALSE;
	} else {
		
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		
		//Anti-aliasing
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		
		//Create window
		gWindow = SDL_CreateWindow( "wavPlotter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		if( gWindow == NULL ) {
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = FALSE;
		} else {
		
			//Create context
			gContext = SDL_GL_CreateContext( gWindow );
			if( gContext == NULL ) {
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				success = FALSE;
			} else {
				//Use Vsync
				if( SDL_GL_SetSwapInterval( 1 ) < 0 ) {
					printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
				}

				//Initialize OpenGL
				if( !SDL_initGL() ) {
					printf( "Unable to initialize OpenGL!\n" );
					success = FALSE;
				}
				
				//Make the cursor invisible
				if( SDL_ShowCursor(SDL_DISABLE) == -1 ) {
					printf( "Warning: Unable to hide the cursor! SDL Error: %s\n", SDL_GetError() );
				}
				
				//Anti-aliasing
				glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
				glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );

				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_POLYGON_SMOOTH);
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
	glOrtho( 0.0, screenWidth, screenHeight, 0.0, 1.0, -1.0 );
	
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

	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gContext = NULL;
	
	//Quit SDL subsystems
	SDL_Quit();
}

int main() {
	gWindow = NULL;
	gContext = NULL;
	
	screenWidth = 800;
	screenHeight = 600;
	
	SDL_init();
	
	glBegin( GL_TRIANGLES );
		glColor3ub( 255, 0, 0 );
		
		glVertex2f( screenWidth / 2, screenHeight / 2 - 100 );
		glVertex2f( screenWidth / 2 + 86.6f, screenHeight / 2 + 50 );
		glVertex2f( screenWidth / 2 - 86.6f, screenHeight / 2 + 50 );
		
	glEnd();
	
	SDL_GL_SwapWindow( gWindow );
	
	SDL_Delay(4000);
	SDL_close();
	
	return 0;
}
