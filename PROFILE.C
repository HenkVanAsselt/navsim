/**********************************************************************
*                                                                     *
*        FILE: PROFILE.C                                              *
*                                                                     *
*     PURPOSE: Performs calculations for output of a profile          *
*                                                                     *
* DESCRIPTION: -                                                      *
*                                                                     *
*     HISTORY: 911112 V0.1 - Initial version                          *
*                                                                     *
**********************************************************************/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <window.h>
#include "navsim.h"

#ifdef __MSC__
#include <stdarg.h>
#endif

/*-----------
  Constants
------------*/
#define UP    1
#define DOWN -1

/*--------------
  Prototypes
--------------*/

/*-----------------
  Local variables
------------------*/
static LEFT_RIGHT line_lr;
static int linenr = 0;
static int line_direction = UP;
static int block_direction = UP;
static int point = 0;

/*-------------------------------------------------------------------
|   FUNCTION: calc_length()
|    PURPOSE: Calculate length between (x1,y1) and (x2,y2)
| DESRIPTION: -
|    RETURNS: Calculated length
|    VERSION: 911113 V0.1 - Initial version
---------------------------------------------------------------------*/
double calc_length(double x1, double y1, double x2, double y2)
{
  return( sqrt( SQR(x1-x2) + SQR(y1-y2) ) );
}

/*-------------------------------------------------------------------
|   FUNCTION: calc_direction()
|    PURPOSE: Calculates direction from a (x1,y1) to (x2,y2)
| DESRIPTION: -
|    RETURNS: Calculated direction in degrees.
|    VERSION: 911112 V0.1 - Initial version
---------------------------------------------------------------------*/
double calc_direction(double x1, double y1, double x2, double y2)
{
  double de,dn;  /* differences in northing and easting */
  double b;

  de = x2 - x1;
  dn = y2 - y1;

  if (de == 0.0 && dn == 0.0)
    b = 0.0;
  else
    b = atan2(de,dn);            /* between -pi and pi */

  b *= RAD_TO_DEG;               /* Convert to degrees */

  return(b);
}

/*-------------------------------------------------------------------
|    FUNCTION: default_profile()
|     PURPOSE: Initialize profile to a default state
| DESCRIPTION: Setup of a profile of 20 lines with a distance of
|              25 meters and a length of 500 meters.
|              The start of the profile is the current ships coordinate
|     RETURNS: 0 if OK, -1 if not
|     HISTORY: 911112 V0.1 - Initial version
---------------------------------------------------------------------*/
void default_profile(PROFILE *p)
{
  int i;

  profile_mode = ON;

  p->startx   = ship.x;
  p->starty   = ship.y;
  p->endx     = p->startx + 100;
  p->endy     = p->starty + 0;
  p->dx       = 0.0;
  p->dy       = 25.0;
  p->no_lines = 4;
  p->linenr   = 0;

  /*-----------------------------------------
    Calculate lenght and direction of line 0
  -------------------------------------------*/
  p->direction = calc_direction(p->startx,p->starty,p->endx,p->endy);
  p->length = calc_length(p->startx,p->starty,p->endx,p->endy);

  for (i=0 ; i<MAX_PROFILE_POINTS ; i++)
  {
    p->dist[i] = 25.0 * i;
    p->depth[i] = 1.0 * (i%5);
  }

  calc_profile_line(p,p->linenr);

  profile_mode = OFF;

}

/*-------------------------------------------------------------------
|    FUNCTION: calc_profile_line()
|     PURPOSE: Perform initial profile calculations for a linenumber
| DESCRIPTION: Calculates LR constants from start of line to end of
|              line,
|              Sets ship x, y, heading and helm
|     RETURNS: nothing
|     HISTORY: 911112 V0.1 - Initial version
---------------------------------------------------------------------*/
void calc_profile_line(PROFILE *p, int linenr)
{
  /*--------------------------
    Check if in profile mode
  ---------------------------*/
  if (profile_mode != ON)
    return;

  /*-------------------------------------------------
    Initialize start and end coordinates of the line
  ---------------------------------------------------*/
  p->linenr = linenr;
  if (line_direction == UP)       /* UP */
  {
    p->x1 = p->startx + linenr * p->dx;
    p->y1 = p->starty + linenr * p->dy;
    p->x2 = p->endx   + linenr * p->dx;
    p->y2 = p->endy   + linenr * p->dy;
  }
  else                     /* DOWN */
  {
    p->x1 = p->endx   + linenr * p->dx;
    p->y1 = p->endy   + linenr * p->dy;
    p->x2 = p->startx + linenr * p->dx;
    p->y2 = p->starty + linenr * p->dy;
  }

  /*------------------------------------------------
    Calculate LR coefficients of the profile line
    and initialize ship variables
  ---------------------------------------------------*/
  calc_lr_coeff(&line_lr, p->x1, p->y1, p->x2, p->y2);
  ship.x = p->x1;
  ship.y = p->y1;
  ship.heading = calc_direction(p->x1, p->y1, p->x2, p->y2);
  ship.helm = 0.0;
}

/*-------------------------------------------------------------------
|    FUNCTION: check_profile()
|     PURPOSE: Check ship on profile line.
| DESCRIPTION: Check if ship has reached end of profile line.
|              If so, recalculate ships coordinates and heading.
|     RETURNS: nothing
|     HISTORY: 911112 V0.1 - Initial version
---------------------------------------------------------------------*/
void check_profile(double x, double y, PROFILE *p)
{
  /*--------------------------
    Check if in profile mode
  ---------------------------*/
  if (profile_mode != ON)
    return;

  /*----------------------------
    Update line  L/R variables
  -----------------------------*/
  calc_leftright(&line_lr,x,y);

  /*--------------------------------------------
    If past end of line, increment line number,
    reset ship's coordinates to next line and
    reset ship's heading to opposite direction
  ---------------------------------------------*/
  if (line_lr.togo < 0.0)
  {
    disp_status("PROFILE: end of line %d",p->linenr+1);
    putch(BELL);
    /*--------------------------------------------------------
      Increment linenumber and check if end of survey block
      is reached. If so, inverse the direction of the block
      and the direction of the line, so the ship will travel
      back
    ---------------------------------------------------------*/
    if (block_direction == UP)
      p->linenr++;                     /* Increment line number      */
    else
      p->linenr--;
    line_direction *= -1;              /* Opposite direction         */
    if (p->linenr > p->no_lines-1 || p->linenr < 0)
    {
      disp_status("PROFILE: end of survey block");
      putch(BELL);
      block_direction = block_direction * -1;
      if (p->linenr > p->no_lines-1)
        p->linenr = p->no_lines-1;
      if (p->linenr < 0)
        p->linenr = 0;
    }
    calc_profile_line(p,p->linenr);    /* Initial calcs of next line */
  }

}

/*-------------------------------------------------------------------
|    FUNCTION: profile_menu()
|     PURPOSE: edit global profile profile
| DESCRIPTION: With this menu global profile data can be editted.
|     RETURNS: nothing
|     HISTORY: 911112 V0.1 - Initial version
---------------------------------------------------------------------*/
void profile_menu()
{
  #define NO_ITEMS 10

  #define START_X     0
  #define START_Y     1
  #define END_X       2
  #define END_Y       3
  #define LENGTH      4
  #define DIRECTION   5
  #define DX          6
  #define DY          7
  #define NO_LINES    8
  #define DEPTHS      9

  #define P profile

  int  i;
  int  choise = ESC;
  int  ret = ESC;
  MENU menu[NO_ITEMS];

  setup_menu(menu, 0, "Start X  ", -1, NULL, "profile", &P.startx   , "%.2lf", 0);
  setup_menu(menu, 1, "Start Y  ", -1, NULL, "profile", &P.starty   , "%.2lf", 0);
  setup_menu(menu, 2, "End   X  ", -1, NULL, "profile", &P.endx     , "%.2lf", 0);
  setup_menu(menu, 3, "End   Y  ", -1, NULL, "profile", &P.endy     , "%.2lf", 0);
  setup_menu(menu, 4, "Length   ", -1, NULL, "profile", &P.length   , "%.2lf", 0);
  setup_menu(menu, 5, "Direction", -1, NULL, "profile", &P.direction, "%.2lf", 0);
  setup_menu(menu, 6, "Dx       ", -1, NULL, "profile", &P.dx       , "%.2lf", 0);
  setup_menu(menu, 7, "Dy       ", -1, NULL, "profile", &P.dy       , "%.2lf", 0);
  setup_menu(menu, 8, "No. lines", -1, NULL, "profile", &P.no_lines , "%2d",   0);
  setup_menu(menu, 9, "Depths   ", -1, edit_profile, "profile", NULL, NULL,    0);

  /*----------------------------
    Show menu, and select items
  -----------------------------*/
  do
  {
    choise = popup(NO_ITEMS,menu,5,5);

    switch (choise)
    {
      case DIRECTION:
      case LENGTH:
        /*---------------------------------------
          Calculate end_x and end_y
          (start coordinates remain the same)
        ---------------------------------------*/
          P.endx = P.startx + P.length * sin(P.direction * DEG_TO_RAD);
          P.endy = P.starty + P.length * cos(P.direction * DEG_TO_RAD);
          break;

      case START_X:
      case START_Y:
      case END_X:
      case END_Y:
        /*--------------------------------
          Calculate lenght and direction
        ---------------------------------*/
        P.direction = calc_direction(P.startx,P.starty,P.endx,P.endy);
        P.length = calc_length(P.startx,P.starty,P.endx,P.endy);
        break;

      default:
        break;
    }
  }
  while (choise != ESC);

  /*-----------------------------------
    Perform initializations for line 0
  ------------------------------------*/
  if (profile_mode == ON)
    calc_profile_line(&profile,0);
}

/*-------------------------------------------------------------------
|    FUNCTION: edit_profile()
|     PURPOSE: edit profile
| DESCRIPTION: With this menu MAX_PROFILE_POINTS can be entered to
|              define a profile. The profile is defined from line 0
|              of the survey block
|     RETURNS: nothing
|     HISTORY: 911112 V0.1 - Initial version
---------------------------------------------------------------------*/
void edit_profile()
{
  int  i;
  int  choise = ESC;
  int  ret = ESC;
  char s[MAX_PROFILE_POINTS][80];
  char s1[40];                      /* Edit buffer */
  char header[40];
  MENU menu[MAX_PROFILE_POINTS];

  /*------------------------
  | Fill menu with profile
  -------------------------*/
  do
  {
    for (i=0 ; i<MAX_PROFILE_POINTS ; i++)
    {
      if (TRUE)
      {
        sprintf(s[i],"point %2d - %-8.2lf - %8.2lf",
                i+1,
                profile.dist[i],
                profile.depth[i]);

      }
      else
      {
        sprintf(s[i],"point %2d - %-8.2lf - %8.2lf",i+1,0.0,0.0);
      }
      setup_menu(menu, i, s[i], -1, NULL, "profile", NULL, NULL, 0);
    }

    choise = popup(MAX_PROFILE_POINTS,menu,5,5);

    if (choise == ESC)
      break;                /* Break from loop */

    /*---------------------
      Edit a profile point
    -----------------------*/
    if (choise >= 0 && choise < MAX_PROFILE_POINTS)
    {
      /*---------------
        Edit distance
      ----------------*/
      sprintf(s1,"%8.2lf",profile.dist[choise]);
      sprintf(header,"distance %2d",choise+1);
      ret = editstr(-1,-1,20,header,s1,_DRAW);
      if (ret != ESC)
        profile.dist[choise] = atof(s1);

      /*------------
        Edit depth
      -------------*/
      sprintf(s1,"%8.2lf",profile.depth[choise]);
      sprintf(header,"depth %2d",choise+1);
      ret = editstr(-1,-1,20,header,s1,_DRAW);
      if (ret != ESC)
        profile.depth[choise] = atof(s1);
    }

  }
  while (TRUE);

  nav_system = systeem[0];
}

/*-------------------------------------------------------------------
|    FUNCTION: calc_depths()
|     PURPOSE: Calculate depths when in profile mode
| DESCRIPTION: -
|     RETURNS: 0 if in profile mode, -1 if not
|     HISTORY: 911113 V0.1 - Initial version
---------------------------------------------------------------------*/
int calc_depths(double x, double y, double *depth1, double *depth2, double dt)
{
  double dx1, dx2, dd1, dd2;
  static double old_depth1;
  static double old_depth2;
  static int first_time = TRUE;
  static int depth_dir = UP;

  #define P profile

  /*----------------------------
    Check if profile mode is on
  ------------------------------*/
  if (profile_mode == ON)
  {
    /*----------------------------------------------
      Find points on the line between which we are
    ------------------------------------------------*/
    point = 0;
    if (line_direction == UP)
    {
      while (line_lr.travel > P.dist[point+1] && point < MAX_PROFILE_POINTS-1)
        point++;
    }
    else
    {
      while (line_lr.togo < P.dist[point+1] && point < MAX_PROFILE_POINTS-1)
        point++;
    }

    /*----------------------------------------------------
      Calculate new depth (depending on travel direction)
    -----------------------------------------------------*/
    if (line_direction == UP)
    {
      dx1 = profile.dist[point+1]  - profile.dist[point];
      dd1 = profile.depth[point+1] - profile.depth[point];
      dx2 = line_lr.travel - profile.dist[point];
      if ( fabs(dx1) > 1E-8)
        dd2 = (dd1 / dx1) * dx2 + profile.depth[point];
      else
        dd2 = 0.0;
    }
    else
    {
      dx1 = profile.dist[point+1]  - profile.dist[point];
      dd1 = profile.depth[point+1] - profile.depth[point];
      dx2 = line_lr.togo - (profile.dist[point] - profile.dist[0]);
      if ( fabs(dx1) > 1E-8)
        dd2 = (dd1 / dx1) * dx2 + profile.depth[point];
      else
        dd2 = 0.0;
    }
    *depth1 = dd2;
    *depth2 = dd2 + 1.0;

    return(0);
  }
  else
  {
    /*-----------------------------
      Initialize *hd depth values
    ------------------------------*/
    if (first_time)
    {
      old_depth1 = *depth1;
      old_depth2 = *depth2;
      first_time = FALSE;
    }
    /*----------------------
      Calculate new depths
    -----------------------*/
    *depth1 = *depth1 + (double)(depth_dir * dt);
    *depth2 = *depth2 + (double)(depth_dir * dt);

    /*---------------------------
      Reverse direction of depth
    -----------------------------*/
    if (*depth1 > 15.0)
      depth_dir = depth_dir * -1;
    if (*depth1 < 5.0)
      depth_dir = depth_dir * -1;
    return(0);
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: load_prf_file()
|     PURPOSE: load profile data from file
| DESCRIPTION: -
|     RETURNS: TRUE if successfull, FALSE if no
|     HISTORY: 911113 V0.1 - Initial version
---------------------------------------------------------------------*/
int load_prf_file(char *filename)
{
  int i;
  FILE *infile;
  char s[40];
  int  point;
  char  param[132];

  /*---------------
  | Open datafile
  ----------------*/
  strcpy(s,filename);
  base_name(s);
  strcat(s,".PRF");

  if (!filename[0])
    return(FALSE);

  infile = fopen(s,"r");
  if (!infile)
  {
    wn_error("ERROR: could't open profile file %s",s);
    return(FALSE);
  }

  /*---------------------------
    Load global profile data
  ---------------------------*/
  disp_status("Loading profile from file '%s'",filename);
  fscanf(infile, "%s %lf\n", param, &profile.startx);
  fscanf(infile, "%s %lf\n", param, &profile.endx);
  fscanf(infile, "%s %lf\n", param, &profile.starty);
  fscanf(infile, "%s %lf\n", param, &profile.endy);
  fscanf(infile, "%s %lf\n", param, &profile.dx);
  fscanf(infile, "%s %lf\n", param, &profile.dy);
  fscanf(infile, "%s %d\n" , param, &profile.no_lines);
  fscanf(infile, "%s %d\n" , param, &profile.linenr);

  /*--------------------
    Load profile data
  --------------------*/
  i = 0;
  while (!feof(infile) && (i < MAX_PROFILE_POINTS))
  {
    fscanf(infile,"%d",&point);
    fscanf(infile,"%lf %lf\n",&profile.dist[point],&profile.depth[point]);
    i++;
  }
  fclose(infile);
  return(TRUE);

}

/*-------------------------------------------------------------------
|    FUNCTION: save_prf_file()
|     PURPOSE: save profile data in file
| DESCRIPTION: -
|     RETURNS: TRUE if successfull, FALSE if no
|     HISTORY: 911113 V0.1 - Initial version
---------------------------------------------------------------------*/
int save_prf_file(char *filename)
{
  int  i;
  char s[40];
  FILE *outfile;

  /*------------------
  | Open output file
  -------------------*/
  strcpy(s,filename);
  base_name(s);
  strcat(s,".PRF");
  outfile = fopen(s,"w");
  if (outfile == NULL)
  {
    wn_error("ERROR: couldn't save profile file %s",s);
    return(FALSE);
  }

  disp_status("Saving profile in file '%s'",s);

  /*---------------------------
    Write global profile data
  ---------------------------*/
  fprintf(outfile, "%-15s %lf\n", "START_X", profile.startx);
  fprintf(outfile, "%-15s %lf\n", "END_X  ", profile.endx);
  fprintf(outfile, "%-15s %lf\n", "START_Y", profile.starty);
  fprintf(outfile, "%-15s %lf\n", "END_Y  ", profile.endy);
  fprintf(outfile, "%-15s %lf\n", "DX     ", profile.dx);
  fprintf(outfile, "%-15s %lf\n", "DY     ", profile.dy);
  fprintf(outfile, "%-15s %d\n" , "LINES  ", profile.no_lines);
  fprintf(outfile, "%-15s %d\n" , "LINENR ", profile.linenr);

  /*---------------------
    Write profile data
  ----------------------*/
  for (i=0 ; i<MAX_PROFILE_POINTS ; i++)
  {
    fprintf(outfile, "%2d %lf %lf\n",i,profile.dist[i],profile.depth[i]);
  }
  fclose(outfile);
  return(TRUE);
}

/*-------------------------------------------------------------------
|   FUNCTION: load_profile()
|    PURPOSE: Load profile data from disk
| DESRIPTION: Determines filename of profile data and tries to load
|             the file from disk.
|    RETURNS: Nothing
|    VERSION: 911113 V0.1 - Initial version
---------------------------------------------------------------------*/
void load_profile(void)
{
  char filename[15];

  if (get_dir("*.prf",filename) != NULL)
  {
    strcpy(profile_name,filename);
    load_prf_file(profile_name);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: save_profile()
|    PURPOSE: Save profile data on disk
| DESRIPTION: Will give change to change descriptive items, asks
|             for a file number (get_paramfile_name) and saves
|             station data to disk by saving all system data.
|    RETURNS: Nothing
|    VERSION: 901104 V0.2
|             900501 V0.3 - Will save all system data
---------------------------------------------------------------------*/
void save_profile(void)
{
  char filename[80];
  char path[132];

  strcpy(path,data_dir);
  strcat(path,"*.prf");
  if (get_dir(path,filename) != NULL)
  {
    strcpy(profile_name,filename);
    save_prf_file(profile_name);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: save_prof_as()
|    PURPOSE: Save profile data under specified name
| DESRIPTION:
|    RETURNS: nothing
|    HISTORY: 920221 V0.1 - Initial version
---------------------------------------------------------------------*/
void save_prof_as()
{
  int ret;
  char filename[80];

  ret = editstr(-1,-1,20,"profiele file name",filename,_DRAW);
  if (ret == ENTER) {
    strcpy(profile_name,filename);
    save_prf_file(profile_name);
  }
}
