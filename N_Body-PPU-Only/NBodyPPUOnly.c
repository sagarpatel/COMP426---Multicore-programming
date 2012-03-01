#include <stdio.h>
#include <altivec.h>
#include <math.h>
#include <time.h>
#include <altivec.h>
#include <ppu_intrinsics.h>






int a[4] __attribute__((aligned(16))) = {1,3,5,7};
int b[4] __attribute__((aligned(16))) = {2,3,6,8};
int c[4] __attribute__((aligned(16)));


int main(int argc, char **argv)
{

  __vector signed int *va = (__vector signed int *) a;
  __vector signed int *vb = (__vector signed int *) b;
  __vector signed int *vc = (__vector signed int *) c;

  *vc = vec_add(*va,*vb);

printf("c[0]=%d, c[1]=%d, c[2]=%d, c[3]=%d¥n", c[0], c[1], c[2], c[3]);

return 0;



}