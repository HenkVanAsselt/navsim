/****************************************************************************
*                                                                           *
*        FILE: FILIO.C                                                      *
*                                                                           *
*     PURPOSE: FILE I/O ROUTINES FOR 'NAVSIM' NAVIGATION SIMULATOR          *
*                                                                           *
* DESCRIPTION: -                                                            *
*                                                                           *
*     HISTORY: 911214 V1.4                                                  *
*                                                                           *
****************************************************************************/

/*---------------------------------------------------------------------------
                        HEADER FILES
---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <dos.h>
#include <window.h>
#include <front9.h>
#include "navsim.h"

/*---------------------------------------------------------------------------
                        PROTOTYPES
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  GRID_REC_1 and GRID_REC_2 provide interface to PDS-1000 projection files
---------------------------------------------------------------------------*/
struct GRID_REC_1   /* 40 bytes records */
{
  int record_id;
  double e0;
  double n0;
  double k0;
  double lon0;
  int dummy[3];
};

struct GRID_REC_2   /* 40 bytes records */
{
  int record_id;
  double lat0;
  int    n_s;
  int    e_w;
  double skew;
  int    dummy[9];
};

union GRID_RECORDS
{
  struct GRID_REC_1 rec1;
  struct GRID_REC_2 rec2;
};

/*---------------------------------------------------------------------------
  SPH_REC provides an interface to PDS-1000 projection files
---------------------------------------------------------------------------*/
typedef struct     /* 40 bytes records */
{
  int record_id;
  double a;
  double e2;
  int dummy[11];
}
SPH_RECORD;

/*===========================================================================
                        START OF FUNCTIONS
===========================================================================*/

/*-------------------------------------------------------------------
|    FUNCTION: base_name()
|     PURPOSE: Strip extension of filename
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 910822 V0.1 - Initial version
---------------------------------------------------------------------*/
void base_name(char *s)
{
  char *p;

  trimstr(s);
  p = strchr(s,'.');
  if (*p)
    *p = '\0';
  strupr(s);
}

/*-------------------------------------------------------------------
|   FUNCTION: save_system(char *filename)
|    PURPOSE: Save system data
| DESRIPTION: Save system data on disk with name 'filename'
|    RETURNS: 1 if successfull, 0 if not
|    VERSION: 901101 V0.2
---------------------------------------------------------------------*/
save_system(char *filename)
{
  int  i,j;
  FILE *outfile;
  extern char syledis_format[];

  /*------------------
  | Open output file
  -------------------*/
  base_name(filename);
  strcat(filename,".SYS");
  outfile = fopen(filename,"w");
  if (!outfile)
  {
    wn_error("ERROR: could not save systemfile %s",filename);
    return(FALSE);
  }

  disp_status("Saving system in file '%s'",filename);

  /*-------------------------------------
    information about navigation system
  --------------------------------------*/
  for (i=0 ; i<MAX_CHANNELS; i++)
  {
    if (systeem[i] != NULL)
    {
      fprintf(outfile,"%-15s %02d %02d %02d %5d %1d %1c %1d %1d %.2lf\n",
        "XSYSTEM",i,systeem[i]->sys,
        systeem[i]->channel, systeem[i]->baudrate,
        systeem[i]->databits, systeem[i]->parity, systeem[i]->stopbits,
        systeem[i]->format, systeem[i]->dt, systeem[i]->handshake);
    }
  }

  /*-------------------------------------
  |  Write selected stations or patterns
  --------------------------------------*/
  for (i=0 ; i<MAX_CHANNELS; i++)
  {
    if (systeem[i] != NULL)
    {
      fprintf(outfile,"%-15s %02d ",
        "XPATTERN",i);
      for (j=0 ; j<MAX_PATTERNS ; j++)
        fprintf(outfile,"%2d ",systeem[i]->p[j]);
      fprintf(outfile,"\n");
    }
  }

  /*-------------------------------------
    Ouput of syledis format as a string
  ---------------------------------------*/
  fprintf(outfile,"%-15s %s\n","SYLEDIS_FORMAT",syledis_format);

  /*------------------------------
  | information about the ship
  ------------------------------*/
  fprintf(outfile,"%-15s %lf\n","SHIP_X",ship.x);
  fprintf(outfile,"%-15s %lf\n","SHIP_LON",ship.lon);
  fprintf(outfile,"%-15s %lf\n","SHIP_Y",ship.y);
  fprintf(outfile,"%-15s %lf\n","SHIP_LAT",ship.lat);
  fprintf(outfile,"%-15s %lf\n","SHIP_H",ship.h);
  fprintf(outfile,"%-15s %lf\n","SPEED",ship.speed);
  fprintf(outfile,"%-15s %lf\n","HEADING",ship.heading);
  fprintf(outfile,"%-15s %lf\n","HELM",ship.helm);
  fprintf(outfile,"%-15s %lf\n","VERTICAL",ship.vert_angle);

  /*------------------------------
  | information about spheroid
  -------------------------------*/
  fprintf(outfile,"%-15s %s\n", "SPH_NAME",spheroid.name);
  fprintf(outfile,"%-15s %lf\n","SPH_A",spheroid.a);
  fprintf(outfile,"%-15s %15.10lf\n","SPH_E2",spheroid.e2);

  /*-----------------------------
  | information about projection
  ------------------------------*/
  fprintf(outfile,"%-15s %s\n", "GRID_NAME",projection.name);
  fprintf(outfile,"%-15s %lf\n","GRID_E0",projection.e0);
  fprintf(outfile,"%-15s %lf\n","GRID_N0",projection.n0);
  fprintf(outfile,"%-15s %lf\n","GRID_K0",projection.k0);
  fprintf(outfile,"%-15s %lf\n","GRID_LAT0",projection.lat0);
  fprintf(outfile,"%-15s %lf\n","GRID_LON0",projection.lon0);

  /*----------------------------------------------------
  |  Write name of chain file and write chain data file
  ------------------------------------------------------*/
  fprintf(outfile,"%-15s %s\n","CHAIN",chain_name);
  save_chain(chain_name);

  /*-----------------------------------------------------
  | Write name of profile data
  ------------------------------------------------------*/
  fprintf(outfile,"%-15s %s\n","PROFILE_FILE",profile_name);

  fclose(outfile);
  return(TRUE);     /* station data is saved */

}

/*-------------------------------------------------------------------
|   FUNCTION: read_systemdata(char *filename)
|    PURPOSE: Read system data
| DESRIPTION: Reads station data from disk with name 'filename'
|    RETURNS: 1 if successfull, 0 if not
|    VERSION: 901101 V0.2
---------------------------------------------------------------------*/
load_system(char *filename)
{
  int   i,j,sys;
  FILE	*infile;
  char  param[132];
  char  s[132];
  extern char syledis_format[];

  /*---------------
  | Open datafile
  ----------------*/
  base_name(filename);
  strcat(filename,".SYS");
  infile = fopen(filename,"r");
  if (!infile)
  {
    wn_error("ERROR: couldn't open %s",filename);
    return(FALSE);
  }

  disp_status("Loading system from file '%s'",filename);

  /*----------------------
    Clear all systems
  ---------------------*/
  for (i=0 ; i<no_navsystems ; i++)
    systeem[i] = NULL;

  /*--------------------------
  | Read items of system file
  ---------------------------*/
  while (!feof(infile))
  {
    fscanf(infile,"%s",param);
    strupr(param);

    /*-------------------------------------------
    | Read name of chain file and read this file
    ---------------------------------------------*/
    if (!strcmp(param,"CHAIN"))
    {
      fscanf(infile,"%s\n",chain_name);
    }

    /*--------------------------------------------------
      Read the system numbers of all the systems we use
    ----------------------------------------------------*/
    else if (!strcmp(param,"XSYSTEM"))
    {
      fscanf(infile,"%d %d",&i, &sys);
      sys--;     /* Take care of offset of 1 */
      if (i>=0 && i<MAX_CHANNELS && sys>=0 && sys<no_navsystems)
      {
        systeem[i] = sys_array[sys];
        nav_system = systeem[i];
        fscanf(infile,"%d %d %d %c %d %d %lf\n",
          &systeem[i]->channel, &systeem[i]->baudrate,
          &systeem[i]->databits, &systeem[i]->parity, &systeem[i]->stopbits,
          &systeem[i]->format, &systeem[i]->dt, &systeem[i]->handshake);
        systeem[i]->output_func = systeem[i]->output_list[systeem[i]->format];
      }
    }

    /*--------------------------------------------------
      Read the selected patterns for each system in use
    ----------------------------------------------------*/
    else if (!strcmp(param,"XPATTERN"))
    {
      fscanf(infile,"%d",&i);
      if (i>=0 && i<MAX_CHANNELS)
      {
        for (j=0 ; j<systeem[i]->max_patterns ; j++)
        {
          fscanf(infile,"%d ",&systeem[i]->p[j]);
        }
      }
    }

    else if (!strcmp(param,"SYLEDIS_FORMAT"))
    {
      fgets(syledis_format,40,infile);
    }

    else if (!strcmp(param,"PROFILE_FILE"))
    {
       fgets(profile_name,40,infile);
       trimstr(profile_name);
       strupr(profile_name);
       if (profile_name)
         load_prf_file(profile_name);
    }

    /*------------------------
    | information about ship
    --------------------------*/
    else if (!strcmp(param,"SHIP_X"))
      fscanf(infile,"%lf\n",&ship.x);
    else if (!strcmp(param,"SHIP_LON"))
      fscanf(infile,"%lf\n",&ship.lon);
    else if (!strcmp(param,"SHIP_Y"))
      fscanf(infile,"%lf\n",&ship.y);
    else if (!strcmp(param,"SHIP_LAT"))
      fscanf(infile,"%lf\n",&ship.lat);
    else if (!strcmp(param,"SHIP_H"))
      fscanf(infile,"%lf\n",&ship.h);
    else if (!strcmp(param,"SPEED"))
      fscanf(infile,"%lf\n",&ship.speed);
    else if (!strcmp(param,"HEADING"))
      fscanf(infile,"%lf\n",&ship.heading);
    else if (!strcmp(param,"HELM"))
      fscanf(infile,"%lf\n",&ship.helm);
    else if (!strcmp(param,"VERTICAL"))
      fscanf(infile,"%lf\n",&ship.vert_angle);

    /*------------------------------
    | information about spheroid
    -------------------------------*/
    else if (!strcmp(param,"SPH_NAME"))
    {
      fgets(spheroid.name,40,infile);
      trimstr(spheroid.name);
    }
    else if (!strcmp(param,"SPH_A"))
      fscanf(infile,"%lf\n",&spheroid.a);
    else if (!strcmp(param,"SPH_E2"))
      fscanf(infile,"%lf\n",&spheroid.e2);

    /*-----------------------------
    | information about projection
    ------------------------------*/
    else if (!strcmp(param,"GRID_NAME"))
    {
      fgets(projection.name,40,infile);
      trimstr(projection.name);
    }
    else if (!strcmp(param,"GRID_E0"))
      fscanf(infile,"%lf\n",&projection.e0);
    else if (!strcmp(param,"GRID_N0"))
      fscanf(infile,"%lf\n",&projection.n0);
    else if (!strcmp(param,"GRID_K0"))
      fscanf(infile,"%lf\n",&projection.k0);
    else if (!strcmp(param,"GRID_LAT0"))
      fscanf(infile,"%lf\n",&projection.lat0);
    else if (!strcmp(param,"GRID_LON0"))
      fscanf(infile,"%lf\n",&projection.lon0);
  }

  fclose(infile);
  return(TRUE);
}


/*-------------------------------------------------------------------
|   FUNCTION: load_projection()
|    PURPOSE: Load projection data from disk
| DESRIPTION: Determines filename of projection data and tries to load
|             the file from disk.
|    RETURNS: Nothing
|    VERSION: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
void load_projection(void)
{
  char filename[132];
  char path[132];
  char *fullpath = NULL;

  if (data_dir[0]) {
    strcpy(path,data_dir);
    strcat(path,"\\*.prj");
  }
  else
    strcpy(path,"*.prj");

  fullpath = get_dir(path,filename);
  if (fullpath) {
    strcpy(projection.name,filename);
    load_prj_file(fullpath);
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: load_prj_file()
|     PURPOSE: load projection data in file
| DESCRIPTION: -
|     RETURNS: TRUE if successfull, FALSE if no
|     HISTORY: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
int load_prj_file(char *filename)
{
  FILE *infile;
  union GRID_RECORDS g;

  infile = fopen(filename,"r");
  if (infile == NULL)
  {
    wn_error("ERROR: Can't load %s",filename);
    return(FALSE);
  }

  /*---------------
    Read record 1
  ----------------*/
  fread(&g,40,1,infile);
  projection.e0   = g.rec1.e0;
  projection.n0   = g.rec1.n0;
  projection.k0   = g.rec1.k0;
  projection.lon0 = g.rec1.lon0;

  /*---------------
    Read record 2
  ----------------*/
  fread(&g,40,1,infile);
  projection.lat0 = g.rec2.lat0;

  fclose(infile);
  return(TRUE);

}

/*-------------------------------------------------------------------
|   FUNCTION: load_spheroid()
|    PURPOSE: Load spheroid data from disk
| DESRIPTION: Determines filename of spheroid data and tries to load
|             the file from disk.
|    RETURNS: Nothing
|    VERSION: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
void load_spheroid(void)
{
  char filename[132];
  char path[132];
  char *fullpath = NULL;

  if (data_dir[0]) {
    strcpy(path,data_dir);
    strcat(path,"\\*.sph");
  }
  else
    strcpy(path,"*.sph");

  fullpath = get_dir(path,filename);
  if (fullpath) {
    strcpy(spheroid.name,filename);
    load_sph_file(fullpath);
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: load_sph_file()
|     PURPOSE: load spheroid data in file
| DESCRIPTION: -
|     RETURNS: TRUE if successfull, FALSE if no
|     HISTORY: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
int load_sph_file(char *filename)
{
  FILE *infile;
  SPH_RECORD p;

  infile = fopen(filename,"r");
  if (infile == NULL)
  {
    wn_error("ERROR: couldn't load %s",filename);
    return(FALSE);
  }

  /*---------------
    Read record 1
  ----------------*/
  fread(&p,40,1,infile);
  spheroid.a  = p.a;
  spheroid.e2 = p.e2;

  fclose(infile);
  return(TRUE);

}
