/******************************************************
*
*  Module with routines for DESO 25 Echo Sounder
*
*  Includes:
*
*  HISTORY: 910425 V0.1
*
*******************************************************/

#include "navsim.h"

#if DESO25

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

/*------------------
  Local Prototypes
-------------------*/
static void deso25_msg(void);
static void deso25_key_action(int key);
static void init_deso25(void);

NAV_SYSTEM sys_deso25 =
{
  "Deso_25",                  /* name of navigation system          */
  DESO25,                     /* system number for case statements  */
  0,                          /* number of frequencies              */
  {0.0,},                     /* operation frequencies              */
  {1.0,},                     /* propagation velocity               */
  NONE_STATION,               /* mode                               */
  0,                          /* maximum number of patterns         */
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns      */
  0,                          /* Number of formats                  */
  { "", },                    /* list of format names               */
  { deso25_msg,},             /* List of output functions           */
  0,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0},                      /* Standard deviations                */
  init_deso25,                /* Initialization function            */
  deso25_msg,                 /* Output function                    */
  NULL,                       /* Input function                     */
  deso25_key_action,          /* Special keyboard functions         */
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

static char *dimension_str = " m";
static int strength = 30;

/*-------------------------------------------------------------------
|   FUNCTION: init_deso25()
|    PURPOSE: Initialize and check ATLAS Deso 25 parameters
| DESRIPTION: 1) Checks if update rate is more than 100ms
|    RETURNS: Nothing
|    HISTORY: 910426 V0.1 - Initial version
---------------------------------------------------------------------*/
static void init_deso25()
{
  /*---------------------------
  | Check minimum update rate
  ----------------------------*/
  if (sys_deso25.dt < 0.1)
    sys_deso25.dt = 0.1;

}


/*-------------------------------------------------------------------
|   FUNCTION: deso25_msg()
|    PURPOSE: Generate ATLAS Deso 25 message
| DESRIPTION: Format:  DAxxxxx.xx m<CR><LF>
|                      DBxxxxx.xx m<CR><LF>
|                      BCxx<CR><LF>
|    RETURNS: nothing
|    HISTORY: 910425 V0.1 - First version
---------------------------------------------------------------------*/
static void deso25_msg()
{
  char tmp[20];
  static double diff = 0.15;
  char msg[80];

  diff = -diff;

  /*-------------------------
    Depth of channel 1 (DA)
  --------------------------*/
  sprintf(msg,"DA%08.2f%s%c%c",depth1,dimension_str,CR,LF);

  /*---------------------------------------------
    Depth of channel 2 (DB) (channel 1 +/- diff)
  ----------------------------------------------*/
  sprintf(tmp,"DB%08.2f%s%c%c",depth2,dimension_str,CR,LF);
  strcat(msg,tmp);

  /*-----------------------------
    Bottom target strength(BC)
  ------------------------------*/
  sprintf(tmp,"BC%02d%c%c",strength,CR,LF);
  strcat(msg,tmp);

  strcpy(sys_deso25.out_msg,msg);

}

/*-------------------------------------------------------------------
|   FUNCTION: deso25_key_action()
|    PURPOSE: React on key pressed
| DESRIPTION: 1) Change the output dimension ( meters/feet ).
|    RETURNS: nothing
|    HISTORY: 910425 V0.1 - First version
---------------------------------------------------------------------*/
static void deso25_key_action(int key)
{
  switch (key)
  {
    case 'm':
    case 'M':
      dimension_str = " m";
      break;

    case 'f':
    case 'F':
      dimension_str = "ft";
      break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      strength = (key - '0') * 10;
      break;

    default:
      break;
  }

}

#endif
