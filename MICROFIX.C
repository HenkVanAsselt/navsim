#include "navsim.h"

#if MICROFIX

#include <stdio.h>
#include <string.h>
#include <time.h>

/*--------------------------------
| Prototype's of local functions
---------------------------------*/
static void mfix_600(void);
static void mfix_540(void);
static void mfix_compressed(void);
static void mfix_540_guidance(void);

NAV_SYSTEM sys_microfix =
{
  "Micro-Fix",                /* name of navigation system                  */
  MICROFIX,                   /* system number for case statements  */
  1,                          /* number of frequencies              */
  {4.5e9},                    /* operation frequencies		  		*/
  {299.65e6},                 /* propagation velocity		  		*/
  RANGE_RANGE, 		          /* mode            		 			*/
  8,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  4,                          /* Number of formats                  */
  { "Type 540",               /* list of format names               */
    "Type 600",
    "Compressed",
    "540 Guidance",
  },
  { mfix_540,                 /* List of output functions           */
    mfix_600,
    mfix_compressed,
    mfix_540_guidance,
  },
  1,                          /* Actual format                      */
  1,                          /* Number of standard deviations      */
  {1.0,5.0},                  /* Standard deviations                */
  NULL,                       /* Initialization function            */
  mfix_540,                   /* Output function                    */
  NULL,                       /* Input function                     */
  NULL,                       /* Special keyboard functions         */
  -1,                         /* Channel                            */
  -1,                         /* Slot1                              */
  9600,                       /* Baudrate                           */
  8,                          /* Databits                           */
  1,                          /* Stopbits                           */
  'N',                        /* Parity                             */
  'N',                        /* Handshake                          */
  LF,                         /* Terminator                         */
  120,                        /* Terminal count                     */
};


static char crlf[3] = {CR,LF,'\0'};

/*------------------------------------------------------------------
  Format microfix 540 guidance message
-------------------------------------------------------------------*/
static void mfix_540_guidance()
{
  double rl,dtog;
  int   linenr = 1;
  char  msg[132];

  /*-------------------------
    event number       1-3
    1 blank	       4
    X coordinate       5-14
    2 blanks	      15-16
    Y coordinate      17-26
    2 blanks	      27-28
    line number+sign  29-32
    Off track	      33-38
    2 blanks	      39-40
    Dist to go	      41-50
    2 blanks	      51-52
    CR/LF	          53-54
  ---------------------------*/
  rl   = left_right.rtlft * 10;
  dtog = left_right.togo * 10;
  sprintf(sys_microfix.out_msg,"%3d %10.1lf  %10.1lf  %+4d%+6.0f  %+10.0f  %c%c",
               sys_microfix.eventnr,ship.x,ship.y,linenr,rl,dtog,CR,LF);
}

/*------------------------------------------------------------------
  Format microfix compressed message
-------------------------------------------------------------------*/
static void mfix_compressed()
{
  char tmp[132];
  int	i,j;
  double rl,dtog;
  long  range;
  char  msg[256];
  int	filter = 1, line_nr = 1;

  /*--- Prelimanary calculations ---*/
  rl   = left_right.rtlft * 10;
  dtog = left_right.togo  * 10;

  /*--- First part of message string ---*/
  sprintf(msg,"%01d%04d%02d%02d%02d%+011ld%+011ld%+04d%+05ld%+06ld",
          filter,sys_microfix.eventnr,
      tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec,
      (long)ship.x*10,(long)ship.y*10,line_nr,(long)rl,(long)dtog);

  /*--- Add codes and ranges ---*/
  for (i=0 ; i<8 ; i++)
  {
    j = find_station(nav_system->p[i]);
    if (j>=0)
    {
      range = (long) station[j].range*10;
      if (range > 999999)
        range=999999;
      sprintf(tmp,"%02d%06ld",station[j].code,(long)range);
      strcat(msg,tmp);
    }
    else
      strcat(msg,"??000000");
  }
  strcat(msg,crlf);

  strcpy(sys_microfix.out_msg,msg);

}

/*------------------------------------------------------------------
  Format microfix 540 message
-------------------------------------------------------------------*/
static void mfix_540()
{
  int i,j,c,n;
  char tmp[132];
  char msg[132];

  /*--- Start with event number ---*/
  sprintf(msg,"%3d ",sys_microfix.eventnr);

  /*--- Add at most 4 codes and ranges ---*/
  n = 0;
  for (i=0 ; i<nav_system->max_patterns ; i++)
  {
    j = find_station(nav_system->p[i]);
    if (j>=0)
    {
      c = station[j].code + 50;    /* inc. code with 50        */
      sprintf(tmp,"%2d %7.1f  ",c,station[j].range);
      strcat(msg,tmp);
      n++;
    }
  }
  strcat(msg,crlf);
  strcpy(sys_microfix.out_msg,msg);

}

/*------------------------------------------------------------------
  Format microfix 600 message
-------------------------------------------------------------------*/
static void mfix_600()
{
  int  i,j,n;
  char ns,ew;
  char tmp[132];
  char msg[132];

  /*--- Start of message ---*/
  if (latlong)
  {
    if (ship.lon > 0.0) ew = 'E'; else ew = 'W';
    if (ship.lat > 0.0) ns = 'N'; else ns = 'S';
    sprintf(msg,"EVENT:%4d  TIME:%02d-%02d-%02d  %3d %2d'%5.2lf\"%c  %3d %2d'%5.2lf\"%c%c%c",
     sys_microfix.eventnr,tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec,
     ship.lon_deg,ship.lon_mm,ship.lon_ss,ew,
     ship.lat_deg,ship.lat_mm,ship.lat_ss,ns,
     CR,LF);
  }
  else
    sprintf(msg,"EVENT:%4d  TIME:%02d-%02d-%02d   X=%10.1f   Y=%11.1f%c%c",
     sys_microfix.eventnr,tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec,ship.x,ship.y,CR,LF);

  /*--- Add codes and ranges ---*/
  n = 0;
  for (i=0 ; i<nav_system->max_patterns ; i++)
  {
    j = find_station(nav_system->p[i]);
    if (j>=0)
    {
      n++;
      sprintf(tmp,"%02d %8.1f  ",station[j].code,station[j].range);
      strcat(msg,tmp);
      /*--- Add CR/LF after 4th and 8th code/range block ---*/
      if ((n == 4) || (n == 8))
    	strcat(msg,crlf);
    }
  }

  /*--- Add CR/LF if not 4th or 8th code/range block ---*/
  if ((n != 4) && (n != 8))
    strcat(msg,crlf);

  strcat(sys_microfix.out_msg,msg);

}

#endif
