#include <stdio.h>
#include <altivec.h>
#include <math.h>
#include <time.h>
#include <altivec.h>
#include <ppu_intrinsics.h>
//#include <vec_types.h>



float a[4] __attribute__((aligned(16))) = {1,2,3,4};
float b[4] __attribute__((aligned(16))) = {0,1,0,0};
float c[4] __attribute__((aligned(16)));


#define PARTICLES_MAXCOUNT 128 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 10.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT 0.00000000006673 // real value is 6.673 * 10^-11
#define DELTA_TIME 100.0
#define GRID_SIZE 10 // grid is a +- GRID_SIZE/2 cube
#define EPS 1.0 // EPS^2 constant to avoid singularities
#define ITERATION_COUNT 100

typedef struct 
{
	__vector float position;	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	__vector float velocity;	// || --> 4th element will be used for mass value of the particle
} 
particle_Data;

particle_Data particle_Array[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));



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

	


///main loop
	

	// temp particle Datas used for calculations, not pointers, purposefully passed by value
	particle_Data pDi;
	particle_Data pDj;


	//temp vectors used for calculations in loop
	__vector float tempAcceleration = {0,0,0,0};
	__vector float tempVelocity = {0,0,0,0};
	__vector float tempDistance = {0,0,0,0}; //--> use 4th element to store radius
	__vector float tempDistanceRL1 = {0,0,0,0};
	__vector float tempDistanceRL2 = {0,0,0,0};

	__vector float tempNumerator = {0,0,0,0};
	__vector float tempMassSplat = {0,0,0,0};
	__vector float tempGConstant = {GRAVITATIONALCONSTANT,GRAVITATIONALCONSTANT,GRAVITATIONALCONSTANT,GRAVITATIONALCONSTANT };
	__vector float tempDELATTIME = {DELTA_TIME, DELTA_TIME, DELTA_TIME, DELTA_TIME};
	__vector float tempEPS= {EPS, EPS, EPS, EPS};

	__vector float zeroVector = {0,0,0,0};

	__vector unsigned char yzxwMask = { 4,5,6,7, 8,9,10,11, 0,1,2,3,  12,13,14,15};
	__vector unsigned char zxywMask = { 8,9,10,11, 0,1,2,3, 4,5,6,7,  12,13,14,15};


	//stupid C99, need to declare indicies before for loops
	int i = 0;
	int j = 0;
	int it_counter = 0;

	printf("\n^^^^^^^   Now starting main loop\n\n\n");


	for(it_counter = 0; it_counter < ITERATION_COUNT; ++it_counter)
	{

	//	printf("\nIteration: %d\n",it_counter );


		// this first loop is to calculate the forces/accelerations
		// NOTE ---> NO FORCES ARE APPLIED IN THIS LOOP, NO POSITIONS WILL BE CHANGED.
		// The calculated accelerations will be used to increment the particles velocity vector, NOT POSITION
		for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
		{
			//cache the particle data struct to the temp declared outside the loops
			pDi = particle_Array[i];

			for(j = 0; j<PARTICLES_MAXCOUNT; ++j)
			{

				//for every particle i, calculate for all j's
				// get resultant total velocity, don't apply it in these loops,
				// apply velocities for all bodies at the same time, in seperate loop at the end.

				//cache the particle data struct to the temp declared outside the loops
				pDj = particle_Array[j];

				// Formula being used --> a = (G * m )/(r^2)

				tempDistance = vec_sub(pDj.position,pDi.position); //actual distance vector between objects i and j
				


				/* //Print distances between particles
				printf("Particle %d:   ", i );
				printf("x= %f, y=%f, z=%f", tempDistance[0], tempDistance[1], tempDistance[2]);
				printf("\n");
				*/

				//use the distance vector  right now for numerator, before we overwrite is later in the code
				tempMassSplat = vec_splats((float)pDj.velocity[3]); //mass is stored in the last element (3) of velocity vector
				tempNumerator = vec_madd(tempMassSplat, tempGConstant, zeroVector);
				

				/*
				//Print numerator
				printf("Numerator %d:   ", i );
				printf("x= %f, y=%f, z=%f", tempNumerator[0], tempNumerator[1], tempNumerator[2]);
				printf("\n");
				*/
				 
				 //Assembly for vector rotate
				//__asm__("addi    4,4,1;");

				// denominator part
				// sqaure each component, x,y,z beforehand
				tempDistance = vec_madd(tempDistance, tempDistance, zeroVector);

				//using perm instead of rotate, bleurg
				tempDistanceRL1 = vec_perm(tempDistance, zeroVector, yzxwMask); // imitates lxfloat left rotate
				tempDistanceRL2 = vec_perm(tempDistance, zeroVector, zxywMask); // imitates 2xfloat left rotate

				//add both
				tempDistanceRL1 = vec_add(tempDistanceRL1, tempDistanceRL2);
				//add to original to get total ---> x+y+z
				tempDistance = vec_add(tempDistance, tempDistanceRL1); //tempDistance is now total distance squared
				
				// add EPS to avoid singularity
				tempDistance =  vec_add(tempDistance, tempEPS); //this is now the denominator value

				// invert vector to avoid division later
				tempDistance = vec_re(tempDistance); // this is final denominator (already inverted), only need to multiply
				// tempDistance is now eqivalent to 1/r^2 


				/*
				//Print denominator
				printf("Denominator %d:   ", i );
				printf("x= %f, y=%f, z=%f", tempDistance[0], tempDistance[1], tempDistance[2]);
				printf("\n");
				*/

				//total acceleration applied to particle i, by particle j
				tempAcceleration = vec_madd(tempDistance, tempNumerator, zeroVector);
				
				//increment velocity value of particle with a*dt
				// need to explicitly call the array, since pDi is only a temp pass by value, doesn't change the particle
				particle_Array[i].velocity = vec_madd(tempAcceleration, tempDELATTIME, particle_Array[i].velocity);

				/*
				//Print velocity
				printf("Velocity %d:   ", i );
				printf("x= %f, y=%f, z=%f", pDi.velocity[0], pDi.velocity[1], pDi.velocity[2]);
				printf("\n");
				*/


				/*

				printf("Particle %d:   ", i );
				printf("x= %f, y=%f, z=%f", pDi.velocity[0], pDi.velocity[1], pDi.velocity[2]);
				printf("\n");

				*/
				
				//end of this loop
			}
			//printf("\n");
		}

		//now that all the accelerations for all particles are calculated,
		//apply them and update velocity 
		for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
		{
			//incrementing position with v*dt
			// vec_madd is awesome, it all gets done in one line! emulated the += operator, kinda, but more flexible
			particle_Array[i].position = vec_madd(particle_Array[i].velocity, tempDELATTIME, particle_Array[i].position);

		/*			
			printf("Particle %d positions:   ", i );
			printf("x= %f, y=%f, z=%f", particle_Array[i].position[0], particle_Array[i].position[1], particle_Array[i].position[2]);
			printf("\n");
		*/
		/////////// INSERT QUADRANT CODE HERE  ----> USE tempDistance before its too late!
	

		}

		//end of main loop

	}


	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

	printf("Particle %d positions:   ", i );
	printf("x= %f, y=%f, z=%f", particle_Array[i].position[0], particle_Array[i].position[1], particle_Array[i].position[2]);
	printf("\n");
	}


/*
  __vector  float *va = (__vector  float *) a;	
  __vector  float *vb = (__vector  float *) b;
  __vector  float *vc = (__vector  float *) c;

  //*vc = vec_sub(*va,*vb);


  *vc = vec_perm(*va,*vb, yzxwMask);

printf("c[0]=%f, c[1]=%f, c[2]=%f, c[3]=%f¥n", c[0], c[1], c[2], c[3]);
*/


return 0;



}