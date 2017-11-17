/******************************************************
*
*  Module with routines for TRIMBLE GPS ontvanger
*
*  Includes:
*
*  HISTORY: 910813 V0.1
*
*******************************************************/

#include "navsim.h"

#if TRIMBLE

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <window.h>
#include <front9.h>

/*------------------
  Local Prototypes
-------------------*/
static void trimble_msg(void);

static char tmp[250];

NAV_SYSTEM sys_trimble =
{
  "TRIMBLE",                  /* name of navigation system                      */
  TRIMBLE,                    /* system number for case statements      */
  1,                          /* number of frequencies              */
  {9.0e9,},                   /* operation frequencies		  		*/
  {299.650e6,},               /* propagation velocity		  		*/
  NONE_STATION,               /* mode                                               */
  0,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  0,                          /* Number of formats                  */
  {                           /* list of format names               */
    "",
  },
  {                           /* List of output functions           */
    trimble_msg,
  },
  0,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0},                      /* Standard deviations                */
  NULL,                       /* Initialization function            */
  trimble_msg,                /* Output function                    */
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


/*-----------------
| Local variables
------------------*/
static float pdop = 2.0;
static int counter = 0;     /* counter to output satellite summary */
static int fix_type = 4;

/*-------------------------------------------------------------------
|   FUNCTION: trimble_msg()
|    PURPOSE: Generate TRIMBLE message
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910813 V0.1 - First version: Fixed satellite summary
---------------------------------------------------------------------*/
static void trimble_msg()
{
  int i;
  char msg[132];

  /*-------------------
    Initialise message
  ---------------------*/
  msg[0] = '\0';

  /*------------------------------------------------
    Adjust counter for output of satellite summary
  -------------------------------------------------*/
  counter++;
  counter = counter % 10;

  /*-------------------------------
    Print fixed satellite summary
  --------------------------------*/
  if (!counter)
  {
    sprintf(msg,"SV EL AZM SN IODC CONT  GPS.TIME%c%c",CR,LF);
    sprintf(tmp,"11 45 132 25 0610 0518 +499044.000%c%c",CR,LF);
    strcat(msg,tmp);
    sprintf(tmp,"06 37 314 17 0166 0477 +499044.000%c%c",CR,LF);
    strcat(msg,tmp);

    /*--- PERFORM INTERMEDIATE OUTPUT */
    display(DISP_OUTPUT);
    msg_output(msg);
    for (i=0 ; i<10000 ; i++);      /* wait */
    msg[0] = '\0';

    sprintf(msg,"02 54 040 17 0203 0465 +499044.000%c%c",CR,LF);
    sprintf(tmp,"16 14 293 07 0379 0456 +499044.000%c%c",CR,LF);
    strcat(msg,tmp);

    /*--- PERFORM INTERMEDIATE OUTPUT */
    strcpy(sys_trimble.out_msg,msg);
    display(DISP_OUTPUT);
    msg_output(sys_trimble.out_msg);
    for (i=0 ; i<10000 ; i++);      /* wait */
    msg[0] = '\0';
  }

  /*----------------------------------------
    Generate position calculations message
  -----------------------------------------*/
  if (!counter)      /* Generate header */
  {
    sprintf(msg," ID DAY DOY   DATE      TIME    LATITUDE    LONGITUDE   HGT  PDOP CLOCK   V.VEL  H.VEL   HDG  FREQ.OFFSET CONT S  SVS%c%c",CR,LF);
    /*--- PERFORM INTERMEDIATE OUTPUT */
    strcpy(sys_trimble.out_msg,msg);
    display(DISP_OUTPUT);
    msg_output(sys_trimble.out_msg);
    for (i=0 ; i<10000 ; i++);      /* wait */
    msg[0] = '\0';
  }

  sprintf(msg,"[00 FRI 173 %02d-AUG-%02d %02d:%02d:%02dd",
               tmnow->tm_mday,tmnow->tm_year,
               tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);

  sprintf(tmp,"%02d:%07.4lfN ",ship.lat_deg,ship.lat_mm+ship.lat_ss/60.0);
  strcat(msg,tmp);

  sprintf(tmp,"%03d:%07.4lfE ",ship.lon_deg,ship.lon_mm+ship.lon_ss/60.0);
  strcat(msg,tmp);

  strcat(msg,"54:00.0000N 001:30.0000E ");

  sprintf(tmp,"%+04.0f ",ship.h);        /* HGT  */
  strcat(msg,tmp);

  sprintf(tmp,"%04.1f ",pdop);            /* PDOP */
  strcat(msg,tmp);

  strcat(msg,"981657 ");                 /* CLOCK */

  strcat(msg,"+000.00 ");                /* V.VEL */

  sprintf(tmp,"%06.2lf ",ship.speed);    /* H.VEL */
  strcat(msg,tmp);

  sprintf(tmp,"%05.1lf ",ship.heading);  /* HDG   */
  strcat(msg,tmp);

  strcat(msg,"-1.2740E-06 ");            /* FREQ.OFFSET */

  strcat(msg,"0457 ");                   /* CONT */

  sprintf(tmp,"%01d ",fix_type);         /* S */
  strcat(msg,tmp);

  strcat(msg,"11,6,2,16");               /* SVS  */

  sprintf(tmp,"]%c%c",CR,LF);            /* Close message */
  strcat(msg,tmp);

  strcpy(sys_trimble.out_msg,msg);

}

#endif
