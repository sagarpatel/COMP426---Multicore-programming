#include <stdio.h>
#include <altivec.h>
#include <math.h>
#include <time.h>
#include <altivec.h>
#include <ppu_intrinsics.h>



/*
int a[4] __attribute__((aligned(16))) = {1,3,5,7};
int b[4] __attribute__((aligned(16))) = {2,3,6,8};
int c[4] __attribute__((aligned(16)));
*/

#define PARTICLES_MAXCOUNT 128 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 1.0
#define GRAVITATIONALCONSTANT 1.0
#define DELTA_TIME 1.0;

typedef struct 
{
	__vector float position;	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	__vector float velocity;	// || --> 4th element will be used for mass value of the particle
} 
particle_Data;

particle_Data particle_Array[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));


int main(int argc, char **argv)
{


///main loop

	

	// temp particle Datas used for calculations, not pointers, purposefully passed by value
	particle_Data pDi;
	particle_Data pDj;


	//temp vectors used for calculations in loop
	__vector float tempAcceleration = {0,0,0,0};
	__vector float tempVelocity = {0,0,0,0};
	__vector float tempRadius = {0,0,0,0};

	//stupid C99, need to declare indicies before for loops
	int i = 0;
	int j = 0;

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
			





		}




	}






/*
  __vector signed int *va = (__vector signed int *) a;
  __vector signed int *vb = (__vector signed int *) b;
  __vector signed int *vc = (__vector signed int *) c;

  *vc = vec_add(*va,*vb);

printf("c[0]=%d, c[1]=%d, c[2]=%d, c[3]=%d¥n", c[0], c[1], c[2], c[3]);
*/
return 0;



}