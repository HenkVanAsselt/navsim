#include <stdio.h>
#include <stdlib.h>
#include <dos.h>


unsigned char data[40];

struct GRID_REC_1
{
  int record_id;
  double e0;
  double n0;
  double k0;
  double lon0;
  int dummy[3];
};

struct GRID_REC_2
{
  int record_id;
  double lat0;
  int    n_s;
  int    e_w;
  double skew;
  int    dummy[9];
};

union GRID
{
  struct GRID_REC_1 rec1;
  struct GRID_REC_2 rec2;
};

union GRID g;

main()
{
  FILE *infile;

  infile = fopen("uk_grid_.prj","r");
  if (!infile)
  {
    puts("Error: file not opened");
    exit(-1);
  }

  fread(&g,40,1,infile);
  printf("id: %d\n",g.rec1.record_id);
  printf("e0: %lf\n",g.rec1.e0);
  printf("n0: %lf\n",g.rec1.n0);
  printf("k0: %.10lf\n",g.rec1.k0);
  printf("cm0: %lf\n",g.rec1.lon0);
  fread(&g,40,1,infile);
  printf("id: %d\n",g.rec2.record_id);
  printf("l0: %lf\n",g.rec2.lat0);
  printf("n: %d\n",g.rec2.n_s);
  printf("s: %d\n",g.rec2.e_w);
  printf("skew: %lf\n",g.rec2.skew);

  exit(0);

}
