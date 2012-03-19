//PPU code 

#include <sched.h>
#include <libspe2.h>
#include <pthread.h>
#include <dirent.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#include <math.h>
#include <time.h>
#include <altivec.h>
#include <ppu_intrinsics.h>
#include <libspe2.h>



#define PARTICLES_MAXCOUNT 64 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 1000.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT 0.00000000006673 // real value is 6.673 * 10^-11
#define DELTA_TIME 60.0
#define GRID_SIZE 10 // grid is a +- GRID_SIZE/2 cube
#define EPS 1.0 // EPS^2 constant to avoid singularities
#define ITERATION_COUNT 100

#define SPU_COUNT 6 // numbers of spus that will be used



//pointer to spe code
extern spe_program_handle_t spe_code;

// handle which will be returned bu spe_context_create
spe_context_ptr_t speid;

// SPU entry point address which is initially set to default
unsigned int entry = SPE_DEFAULT_ENTRY;

//used to return data regaring abnormal return from SPE
spe_stop_info_t stop_info;


typedef struct 
{
	__vector float position;	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	__vector float velocity;	// || --> 4th element will be used for mass value of the particle
} 
particle_Data;

// full array
particle_Data particle_Array[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));

/// subdivisions of arrays for each spe
particle_Data spe1_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe2_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe3_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe4_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe5_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe6_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));

// based off http://www.ibm.com/developerworks/library/pa-libspe2/
void *spe_function(void *data)
{

	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY;
	spe_context_ptr_t my_context;

	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);

	spe_program_load(my_context, &spe_code);

	do 
	{
		retval = spe_context_run(my_context, &entry_point, 0, NULL, &spe1_Data, NULL);
	} 
	while (retval > 0); /* Run until exit or error */

	pthread_exit(NULL);

}


int main(int argc, char **argv)
{
	

// setup, assign particles initla positions and masses
// this is done in scalar fashion, NOT SIMD
// insignificant to performance since it's only done once

	//seed random generator
	srand( time(NULL) );

	printf("\n\n\n~~~~~~~~Printing out particles and their randomly assigned positions: \n\n");

	int pC = 0;
	for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
	{
		int grideSize = GRID_SIZE;

	//	printf("\n grideSize/2: %d", grideSize/2);

		float xPos = (float)( rand() % grideSize  - grideSize/2);
		float yPos = (float)( rand() % grideSize  - grideSize/2);
		float zPos = (float)( rand() % grideSize  - grideSize/2);

		particle_Array[pC].position[0] = xPos;
		particle_Array[pC].position[1] = yPos;
		particle_Array[pC].position[2] = zPos;

		particle_Array[pC].velocity[3] = PARTICLES_DEFAULTMASS;

		//particle_Array[pC].position = vec_splat(particle_Array[pC].position, 1);
		//particle_Array[pC].position = vec_splats((float)GRAVITATIONALCONSTANT); --> use splats, seems faster
		/*
		printf("Particle %d:   ", pC );
		printf("x= %f, y=%f, z=%f", particle_Array[pC].position[0], particle_Array[pC].position[1], particle_Array[pC].position[2]);
		printf("\n");
		*/
	}


	// copy arrays into spe ones
	pC = 0;
	for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
	{

		spe1_Data[pC] = particle_Array[pC];	
		spe2_Data[pC] = particle_Array[pC];	
		spe3_Data[pC] = particle_Array[pC];	
		spe4_Data[pC] = particle_Array[pC];	
		spe5_Data[pC] = particle_Array[pC];	
		spe6_Data[pC] = particle_Array[pC];		
	}



	int speCount = spe_cpu_info_get(SPE_COUNT_PHYSICAL_SPES,-1);
	printf("\n");
	printf("%d", speCount);



	// start threading code
	pthread_t spe1Thread;
	int spe1Retval;

	//create thread
	spe1Retval = pthread_create(
								&spe1Thread,
								NULL, // thread attributes
								spe_function,
								NULL // thread argument
								);

	/* Check for thread creation errors */
	if(spe1Retval) 
	{
		fprintf(stderr, "Error creating thread! Exit code is: %d\n", spe1Retval);
		exit(1);
	}


	//Wait for thread completion
	spe1Retval = pthread_join(spe1Thread, NULL);
	// Check for errors
	if(spe1Retval)
	{
		fprintf(stderr, "Error joining thread! Exit code is: %d\n", spe1Retval);
		exit(1);
	}




	pthread_t spe2Thread;
	int spe2Retval;

	//create thread
	spe2Retval = pthread_create(
								&spe2Thread,
								NULL,
								spe_function,
								NULL
								);

	/* Check for thread creation errors */
	if(spe2Retval) 
	{
		fprintf(stderr, "Error creating thread! Exit code is: %d\n", spe2Retval);
		exit(1);
	}


	//Wait for thread completion
	spe2Retval = pthread_join(spe2Thread, NULL);
	// Check for errors
	if(spe2Retval)
	{
		fprintf(stderr, "Error joining thread! Exit code is: %d\n", spe2Retval);
		exit(1);
	}



















	return 0;
}