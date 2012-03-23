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


particle_Data tempParticleArray[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));



int speNumber = 0;
//particle_Data* speData;
int i;
// based off http://www.ibm.com/developerworks/library/pa-libspe2/
/* NOTE -- the prototype is based on the standard pthread thread signature */
void *spe_code_launch_1(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe1_Data, 1, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_2(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe2_Data, 2, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_3(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe3_Data, 3, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_4(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe4_Data, 4, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_5(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe5_Data, 5, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_6(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe6_Data, 6, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
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
		
		printf("Particle %d:   ", pC );
		printf("x= %f, y=%f, z=%f", particle_Array[pC].position[0], particle_Array[pC].position[1], particle_Array[pC].position[2]);
		printf("\n");
		
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
/*
	printf("\n");
	printf("%d", speCount);

	printf("\n");
	printf("\n");
	printf("--------------\n");
	printf("Starting spe1 part\n");
*/
	int retval;
	pthread_t spe1_Thread;
	pthread_t spe2_Thread;
	pthread_t spe3_Thread;
	pthread_t spe4_Thread;
	pthread_t spe5_Thread;
	pthread_t spe6_Thread;


	//speData = spe1_Data;
	speNumber = 0;
	/* Create Thread */
//	printf("spe1_Data value: %d\n", (int)spe1_Data );
	retval = pthread_create(&spe1_Thread, // Thread object
							NULL, // Thread attributes
							spe_code_launch_1, // Thread function
							NULL // Thread argument
							);

//	printf("spe2_Data value: %d\n", (int)spe2_Data );
	
	retval = pthread_create(&spe2_Thread, // Thread object
							NULL, // Thread attributes
							spe_code_launch_2, // Thread function
							NULL // Thread argument
							);
	
	retval = pthread_create(&spe3_Thread, // Thread object
							NULL, // Thread attributes
							spe_code_launch_3, // Thread function
							NULL // Thread argument
							);

	
	retval = pthread_create(&spe4_Thread, // Thread object
							NULL, // Thread attributes
							spe_code_launch_4, // Thread function
							NULL // Thread argument
							);

	retval = pthread_create(&spe5_Thread, // Thread object
							NULL, // Thread attributes
							spe_code_launch_5, // Thread function
							NULL // Thread argument
							);

	retval = pthread_create(&spe6_Thread, // Thread object
							NULL, // Thread attributes
							spe_code_launch_6, // Thread function
							NULL // Thread argument
							);
	


	//Wait for Thread Completion
	retval = pthread_join(spe1_Thread, NULL);

	retval = pthread_join(spe2_Thread, NULL);

	
	retval = pthread_join(spe3_Thread, NULL);

	retval = pthread_join(spe4_Thread, NULL);
	retval = pthread_join(spe5_Thread, NULL);
	retval = pthread_join(spe6_Thread, NULL);
	/*	
	printf("print out values from post spe1 calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", spe1_Data[i].position[0], spe1_Data[i].position[1], spe1_Data[i].position[2]);
		printf("\n");
	
	}
	printf("print out values from post spe2 calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", spe2_Data[i].position[0], spe2_Data[i].position[1], spe2_Data[i].position[2]);
		printf("\n");
	
	}
	printf("print out values from post spe3 calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", spe3_Data[i].position[0], spe3_Data[i].position[1], spe3_Data[i].position[2]);
		printf("\n");
	
	}
	printf("print out values from post spe4 calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", spe4_Data[i].position[0], spe4_Data[i].position[1], spe4_Data[i].position[2]);
		printf("\n");
	
	}
	printf("print out values from post spe5 calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", spe5_Data[i].position[0], spe5_Data[i].position[1], spe5_Data[i].position[2]);
		printf("\n");
	
	}
	printf("print out values from post spe6 calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", spe6_Data[i].position[0], spe6_Data[i].position[1], spe6_Data[i].position[2]);
		printf("\n");
	
	}
*/
	

	speNumber = 1;
	
	for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
	{
		particle_Array[i] = spe1_Data[i];
	}

	speNumber = 2;
	for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
	{
		particle_Array[i] = spe2_Data[i];
	}

	speNumber = 3;
	for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
	{
		particle_Array[i] = spe3_Data[i];
	}

	speNumber = 4;
	for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
	{
		particle_Array[i] = spe4_Data[i];
	}

	speNumber = 5;
	for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
	{
		particle_Array[i] = spe5_Data[i];
	}

	speNumber = 6;
	for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
	{
		particle_Array[i] = spe6_Data[i];
	}

	// reset spe counter
	speNumber = 0;



	


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



	printf("print out values from post spe calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", particle_Array[i].position[0], particle_Array[i].position[1], particle_Array[i].position[2]);
		printf("\n");
	
	}





	return 0;
}


