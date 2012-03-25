/*
Sagar Patel
9356037
COMP426 Asg1

Trivial N-Body Simulation that runs on the PPU of the Cell Processor

Pretty much entirely SIMD based and NOT A SINGLE BRANCH IN THE CODE



 Copyright (C) 2012  Sagar Patel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/




#include <stdio.h>
#include <math.h>
#include <time.h>
#include <altivec.h>
#include <ppu_intrinsics.h>
//#include <vec_types.h>
#include <sys/time.h>


#define PARTICLES_MAXCOUNT 512 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 1000.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT 0.00000000006673 // real value is 6.673 * 10^-11
#define DELTA_TIME 60.0
#define GRID_SIZE 10 // grid is a +- GRID_SIZE/2 cube
#define EPS 1.0 // EPS^2 constant to avoid singularities
#define ITERATION_COUNT 200

typedef struct 
{
	__vector float position;	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	__vector float velocity;	// || --> 4th element will be used for mass value of the particle
} 
particle_Data;

particle_Data particle_Array[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));

__vector unsigned short octantCount;

int main(int argc, char **argv)
{

	time_t startTime = time(NULL);


// setup, assign particles initla positions and masses
// this is done in scalar fashion, NOT SIMD
// insignificant to performance since it's only done once

	struct timeval start;
	gettimeofday(&start,NULL);

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
	__vector unsigned int oneVector = {1,1,1,1};

	__vector unsigned int axisBitShiftMask = {0,1,2,0};


	__vector unsigned char yzxwMask = { 4,5,6,7, 8,9,10,11, 0,1,2,3,  12,13,14,15};
	__vector unsigned char zxywMask = { 8,9,10,11, 0,1,2,3, 4,5,6,7,  12,13,14,15};

	__vector unsigned short resetOctantCount = {0,0,0,0,0,0,0};
	__vector unsigned short increment = {1,1,1,1,1,1,1,1};

	__vector float tempUnitVector = {0,0,0,0};
	__vector float distanceVector = {0,0,0,0};

	//stupid C99, need to declare indicies before for loops
	int i = 0;
	int j = 0;
	int it_counter = 0;

	printf("\n^^^^^^^   Now starting main loop\n\n\n");


	for(it_counter = 0; it_counter < ITERATION_COUNT; ++it_counter)
	{

		octantCount = resetOctantCount;
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
				
				// save value for unit vector calculation later
				distanceVector = tempDistance;

				/* //Print distances between particles
				printf("Particle %d:   ", i );
				printf("x= %f, y=%f, z=%f", tempDistance[0], tempDistance[1], tempDistance[2]);
				printf("\n");
				*/

				//use the distance vector  right now for numerator, before we overwrite is later in the code
				// use mass of subject mass
				tempMassSplat = vec_splats((float)pDi.velocity[3]); //mass is stored in the last element (3) of velocity vector
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

				//save inverse magnitude for unit vector later
				tempUnitVector = vec_rsqrte(tempDistance);

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

				// create unit vector
				tempUnitVector = vec_madd(distanceVector, tempUnitVector, zeroVector);
				
				// apply unit vector to acceleration
				tempAcceleration = vec_madd(tempUnitVector, tempAcceleration, zeroVector);


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


			///// ALL CODE BELOW THIS SHOULD ONLY BE RUN ON PPU \\\\\\\\\\\\\\\\\\


		/////////// INSERT QUADRANT CODE HERE , actually octant --> 8 equal sub cubes 
			
			// compare with zero vector to get on which side of each axis the particle is
			// 0 is negative, 1 is positive side of the axis
			__vector bool int axisDirection = vec_cmpgt(particle_Array[i].position, zeroVector);



			// need to manually set, can't cast due to size difference error
			__vector unsigned int shiftedAxis = { (unsigned int)axisDirection[0],
												  (unsigned int)axisDirection[1],
												  (unsigned int)axisDirection[2],
													0};
			// need to do this to revert 1s into NON 2s complement form --> vec_cmgt doc LIES
			shiftedAxis = vec_andc(oneVector, shiftedAxis);

			/*
			printf("Particle %d axis sign:   ", i );
			printf("x= %x, y=%x, z=%x", shiftedAxis[0], shiftedAxis[1], shiftedAxis[2]);
			printf("\n");
			*/

			// shift 3 axies simultaneously (actually only 2, 1 stays in origina positon
			//, with intent to OR them later
			shiftedAxis = vec_sl(shiftedAxis, axisBitShiftMask); // will also use as x vector

			__vector unsigned int axis_Y = vec_splats(shiftedAxis[1]);
			__vector unsigned int axis_Z = vec_splats(shiftedAxis[2]);
			// merge shhifted x y z values by OR-ing
			// this gives the octant id, range from 0-7 (000 to 111 in binary)
			shiftedAxis = vec_or(shiftedAxis, axis_Y);
			shiftedAxis = vec_or(shiftedAxis, axis_Z);
			// insert octant value into last slot of position vector of particle
			particle_Array[i].position[3] = (float)shiftedAxis[0];

			//printf("Oct ID: %d \n", shiftedAxis[0]);

			/////// Update octant vector by incrementing octant that the particle is in
			// The only possible non SIMD line in the entire program, 
			//irreleant since quadrant counting should occur on PPU anyways
			octantCount[shiftedAxis[0]] ++ ;
			
			

		}

		//end of main loop
/*
		printf("End of iteration %d --->    ",it_counter );
		printf("Particle disttribution across the octants: \n");
		printf("O0: %d    O1: %d    O2: %d    O3: %d    O4: %d    O5: %d    O6: %d    O7: %d\n",
				octantCount[0], octantCount[1], octantCount[2], octantCount[3], 
				octantCount[4],	octantCount[5], octantCount[6], octantCount[7]);
		printf("\n");
		*/
	}

/*
	printf("\n");
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{
	printf("Particle %d final position:   ", i );
	printf("x= %f, y=%f, z=%f", particle_Array[i].position[0], particle_Array[i].position[1], particle_Array[i].position[2]);
	printf("\n");

	printf("End of iteration %d --->    ",it_counter );
		printf("Particle disttribution across the octants: \n");
		printf("O0: %d    O1: %d    O2: %d    O3: %d    O4: %d    O5: %d    O6: %d    O7: %d\n",
				octantCount[0], octantCount[1], octantCount[2], octantCount[3], 
				octantCount[4],	octantCount[5], octantCount[6], octantCount[7]);
		printf("\n");
	}
*/
	printf("Particle disttribution across the octants: \n");
		printf("O0: %d    O1: %d    O2: %d    O3: %d    O4: %d    O5: %d    O6: %d    O7: %d\n",
				octantCount[0], octantCount[1], octantCount[2], octantCount[3], 
				octantCount[4],	octantCount[5], octantCount[6], octantCount[7]);
		printf("\n");


		struct timeval end;
	gettimeofday(&end,NULL);
	float deltaTime = ((end.tv_sec - start.tv_sec)*1000.0f + (end.tv_usec -start.tv_usec)/1000.0f);


	printf("Execution time:    %f\n",deltaTime);
	


return 0;



}