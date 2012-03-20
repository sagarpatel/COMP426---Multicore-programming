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
#include <stdlib.h>

#include "common.h"


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


int main(int argc, char **argv)
{
	

// setup, assign particles initla positions and masses
// this is done in scalar fashion, NOT SIMD
// insignificant to performance since it's only done once

	//seed random generator
	srand( time(NULL) );

//	printf("\n\n\n~~~~~~~~Printing out particles and their randomly assigned positions: \n\n");

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




	spe_context_ptr_t spe1_ID;
	spe1_ID = spe_context_create(0,NULL);
	spe_program_load(spe1_ID, &spe_code );
	spe_context_run(spe1_ID, &entry, 0, &spe1_Data, 6, &stop_info);

	spe_context_destroy(spe1_ID);

	printf("print out values from post spe calculations\n");
	int i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", spe1_Data[i].position[0], spe1_Data[i].position[1], spe1_Data[i].position[2]);
		printf("\n");
	
	}



	spe_context_ptr_t spe2_ID;
	spe2_ID = spe_context_create(0,NULL);

	printf("spe1_ID: %d\n",spe1_ID );
	printf("spe2_ID: %d\n",spe2_ID );

	printf("SPEID diff: %d\n", spe2_ID - spe1_ID );

	return 0;
}


