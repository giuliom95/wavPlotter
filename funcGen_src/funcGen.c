#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265

#define DEF_HEADER 44
#define DEF_SAMPLES 44100
#define DEF_PULSATION 0.065449847f
#define DEF_PHASE 0
#define DEF_AMPLITUDE 100

int main(){
	long i;
	long total_samples;
	float pulsation, phase, amplitude;
	int8_t one, two;
	int16_t sample;
	char* env;
	int header;
	
	env = getenv("samples");
	total_samples = env ? atoll(env) : DEF_SAMPLES;
	
	env = getenv("pulsation");
	pulsation = env ? atof(env) : DEF_PULSATION;
	
	env = getenv("phase");
	phase = env ? atof(env) : DEF_PHASE;
	
	env = getenv("amplitude");
	amplitude = env ? atof(env) : DEF_AMPLITUDE;

	env = getenv("header");
	header = env ? atoi(env) : DEF_HEADER;
	
	for( i = 0; i < header; i++ ) {
		printf( "%c", 0 );
	}

	for( i = 0; i < total_samples; i++ ) {
		sample = (int16_t)( amplitude * sin( i * pulsation + phase ) );
		one = (int8_t)( sample & 0x00ff );
		two = (int8_t)( sample & 0xff00 >> 8 );
	
		printf( "%c%c%c%c", one, two, 0, 0 );
	
	}
	
	return 0;

}
