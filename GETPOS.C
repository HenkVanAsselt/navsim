#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <window.h>
#include "navsim.h"

#define FALSE 0
#define TRUE !FALSE

/*-------------------------------------------------------------------
|   FUNCTION: double get_latlon(char *s)
|    PURPOSE: Get angle in degrees from a lat- or lon-string
| DESRIPTION: Get angle from string 's'
|             The format of the string may be:
|             "DD.DDDD" or "DD MM.MMM" or "DD MM SS.SSS".
|    RETURNS: Angle in degrees
|    VERSION: 901127 V0.1
---------------------------------------------------------------------*/
double get_latlon(char *s)
{
  int    deg = 0;
  int    mm = 0;
  double ss = 0.0;
  double r;
  char   *token;
  char   *delim = " ,-";

  /*--------
  | Degrees
  ---------*/
  token = strtok(s,delim);
  if (token)
  {
    if (strchr(s,'.'))       /* Format was DDD.DDDD */
    {
      posmode = LATLON_DD;
      r = atof(token);
      return(r);
    }
    else
      deg = atoi(token);
  }

  /*---------
  | Minutes
  ----------*/
  token = strtok(NULL,delim);
  if (token)
  {
    if (strchr(s,'.'))     /* Format was DDD MM.MMM */
    {
      posmode = LATLON_DDMM;
      r = (double)deg + atof(token)/60.0;
      return(r);
    }
    else
      mm = atoi(token);
  }

  /*---------
  | Seconds
  ----------*/
  token = strtok(NULL,delim);
  if (token)
  {
    posmode = LATLON_DDMMSS;
    ss = atof(token);
  }

  r = (double)deg + (double)mm/60.0 + ss/3600.0;
  return(r);
}

/*-------------------------------------------------------------------
|   FUNCTION: getpos()
|    PURPOSE: Get position out of a string
| DESRIPTION: This function will determine string 'pos_str' and extract
|             a position out of it. The string can hold an UTM x/y
|             coordinate or a lat/lon coordinate or a part of it.
|
|             This will be determined as follows:
|             lat: (sub)string contains 'N','S' or 'Z'
|             lon: (sub)string contains 'E','W' or 'O'
|
|             The postition will be returned in
|             lat: lat as DDD.DDDD (decimal degrees), negative if south.
|             lon: lon as DDD.DDDD (decimal degrees), negative if west.
|               X: XXXXX.XX
|               Y: YYYYY.YY
|
|    RETURNS: position mode detected, -1 if error occured.
|    VERSION: 901127 V0.1
|             910819 V0.2 - Returns the mode, extracted from pos_str
|             911112 V0.3 - Removed handling of X and Y on one line.
---------------------------------------------------------------------*/
int get_pos(int *mode, char *pos_str, double *lat, double *lon, double *x, double *y)
{
  char sub[40];          /* Substring pointer         */
  char s[40];            /* Copy of input string      */
  char *kar;             /* Pointer to NSEW character */
  int  i,j;
  int  in_mode;

  /*----------------------
    Initialise variables
  -----------------------*/
  in_mode = *mode;
  strcpy(s,pos_str);     /* Make a copy of the string */

  /*-----------
    Get mode
  ------------*/
  if (strpbrk(s,"NnSsZzEeWwOo"))
    posmode = LATLON;
  else
    posmode = UTM;

  if (posmode == LATLON)
  {
    /*---------------------
    | Handle latlon input
    ----------------------*/
    i = 0;        /* Reset string index */
    kar = s;      /* initialize pointer */
    do
    {
      kar = strpbrk(kar,"NnSsZzEeWwOo");     /* Search for a latlong char */
      if (kar == NULL) break;                /* No one found, break loop  */
      j = 0;
      while (s[i] && !isdigit(s[i])) i++;    /* Find first digit          */
      while (isdigit(s[i]) || isspace(s[i]) || s[i] == '.')
        sub[j++] = s[i++];                   /* Generate substring        */
      sub[j] = '\0';
      if (strchr("WwEeOo",*kar))             /* longitude                 */
      {
        *lon = get_latlon(sub);
        if (strchr("Ww",*kar))               /* West ? then negative      */
          *lon = -*lon;
      }
      else if (strchr("NnSsZz",*kar))        /* lattitude                 */
      {
        *lat = get_latlon(sub);
        if (strchr("SsZz",*kar))             /* South ? then negative     */
          *lat = -*lat;
      }
      kar++;                                 /* Point past found karakter */
    }
    while (TRUE);
    geoutm(*lat,*lon,x,y);
    return(posmode);
  }

  if (posmode == UTM)
  {
    /*-----------------
    | Handle UTM input
    -------------------*/
    trimstr(s);
    if (in_mode == UTM_X)
    {
      *x = atof(s);
    }
    else if (in_mode == UTM_Y)
    {
      *y = atof(s);
    }

    /*------------------------------
      Also convert it to lat/lon
    -------------------------------*/
    utmgeo(*x,*y,lat,lon);
    return(posmode);
  }
}

#ifdef TESTING
/*-------------------------------------------------------------------
|    FUNCTION: main()
|     PURPOSE:
| DESCRIPTION:
|     RETURNS:
|     HISTORY:
---------------------------------------------------------------------*/
main()
{
  double lat,lon;
  double x,y;
  char s[80];

  do
  {
    printf("position: ");
    gets(s);
    if (s[0] == '\n') break;

    get_pos(s,&lat,&lon,&x,&y);

    if (mode == LATLON)
      printf("lat: %lf \nlon: %lf\n\n",lat,lon);
    else
      printf("UTM: %lf %lf \n",x,y);
  }
  while(TRUE);

  return(0);
}
#endif   /* TESTING */
