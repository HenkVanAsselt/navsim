/**********************************************************************
*                                                                     *
*        FILE: POLAR.C                                                *
*                                                                     *
*     PURPOSE: Generate polartrack messages                           *
*              React on commands from navigation computer             *
*              React on operator inputs from keyboard                 *
*                                                                     *
* DESCRIPTION: -                                                      *
*                                                                     *
*     HISTORY: 900606 V1.2                                            *
*              910811 V1.3 - Added preliminary check on commands      *
*                            from navigation computer                 *
*                                                                     *
**********************************************************************/

#include "navsim.h"

#if POLARTRACK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <front9.h>

/*------------
  Defines
----------*/
#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

#define LOC '0'             /* commands from navigation computer */
#define STA '1'
#define MOV '2'
#define VER '3'
#define TED '4'
#define RED '5'
#define REF '6'
#define TST '7'
#define SLP '8'
#define LAM '9'

static char *command[] =
{
  "LOC",
  "STA",
  "MOV",
  "VER",
  "TED",
  "RED",
  "REF",
  "TST",
  "SLP",
  "LAM"
};

#define  P_OFF      1        /* polartrack mode constants */
#define  P_MEASURE  2
#define  P_TRACK    3
#define  P_SEARCH   4

/*-------------------------------
  Prototypes of local functions
---------------------------------*/
static void init_polar(void);
static void polar_long(void);
static void polar_short(void);
static void polar_cmd(char *cmd);
static void polar_key_action(int key);

/*------------------
  Local variables
-------------------*/
static short sta_rcvd = 0;

/*--------------------
  System parameters
----------------------*/
NAV_SYSTEM sys_polartrack =
{
  "Polartrack",               /* name of navigation system           */
  POLARTRACK,                 /* system number for case statements   */
  1,                          /* number of frequencies               */
  {999.0e9},   /* Laser */    /* operation frequencies               */
  {299.65e6,},                /* propagation velocity                */
  RANGE_BEARING_VERTICAL,     /* mode                                */
  1,                          /* maximum number of patterns          */
  1,                          /* Number of patterns system is using  */
  {1},                        /* Selected stations or patterns       */
  2,                          /* Number of formats                   */
  { "Short",                  /* list of format names                */
    "Long",
  },
  {
    polar_short,              /* list of output functions            */
    polar_long,
  },
  1,                          /* Actual format                       */
  1,                          /* Number of standard deviations       */
  {0.75},                     /* Standard deviations                 */
  init_polar,                 /* Initialization function             */
  polar_long,                 /* Output function                     */
  polar_cmd,                  /* Input function                      */
  polar_key_action,           /* Special keyboard functions          */
  -1,                         /* Channel                             */
  -1,                         /* Slot1                               */
  9600,                       /* Baudrate                            */
  8,                          /* Databits                            */
  1,                          /* Stopbits                            */
  'N',                        /* Parity                              */
  'N',                        /* Handshake                           */
  CR,                         /* Terminator                          */
  120,                        /* Terminal count                      */
};

/*------------------------
  Polartrack enviroment
------------------------*/
typedef struct
{
  int	mode;
  int	remote;
  int	tracking;
  int	battery;
  int	temp;
  int	no_contact;
}
POLARTRACK_ENV;
POLARTRACK_ENV polartrack;

/*-------------------------------------------------------------------
|   FUNCTION: init_polar()
|    PURPOSE: Initialize polartrack variables
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901110 V0.1
---------------------------------------------------------------------*/
static void init_polar()
{
  polartrack.mode        = P_OFF;    /* OFF                 */
  polartrack.remote      = 0;        /* not in remote mode  */
  polartrack.tracking    = 0;        /* not tracking now    */
  polartrack.battery     = 4;        /* FULL                */
  polartrack.temp        = 3;        /* OK                  */
  polartrack.no_contact  = TRUE;
}

/*-------------------------------------------------------------------
|   FUNCTION: polar_short()
|    PURPOSE: Generate Polartrack short message
| DESRIPTION: -
|    RETURNS: nothing
|    VERSION: 910425 V0.2
---------------------------------------------------------------------*/
static void polar_short()
{
  int i;
  double range,bearing;

  if ((polartrack.mode == P_OFF) || polartrack.no_contact)
  {
    /*--------------------------------------------------------
      mode == OFF or no contact with target --> give status
    --------------------------------------------------------*/
    bearing = ship.bearing * 1000;
    sprintf(sys_polartrack.out_msg,"0000%1d%c%06.0f%c",
      polartrack.mode,(polartrack.no_contact)?'1':'8',bearing, CR);
  }
  else
  {
    /*-----------------------------------------------------------
      polartrack is not "OFF" or there is contact with target
    -----------------------------------------------------------*/
    i = nav_system->p[1];                 /* only 1 station possible */
    range   = station[i].range * 100.0;   /* centimeters*/ /* %06.0f */
    bearing = ship.bearing * 1000;        /* 0.001 degr.*/ /* %06.0f */
    sprintf(sys_polartrack.out_msg,"%06.0f%06.0f%c",range,bearing,CR);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: polar_long()
|    PURPOSE: Generate Polartrack long message
| DESRIPTION: -
|    RETURNS: nothing
|    VERSION: 910425 V0.2
---------------------------------------------------------------------*/
static void polar_long()
{
  int	 i;
  double  range,bearing,vert;

  if ((polartrack.mode == P_OFF) || polartrack.no_contact)
  {
    /*--------------------------------------------------------
      mode == OFF or no contact with target --> give status
    --------------------------------------------------------*/
    bearing = ship.bearing * 1000;
    vert = ship.vert_angle * 1000;
    sprintf(sys_polartrack.out_msg,"0000%01d%c%06.0f%06.0fTTT%01d%01d%01d%c",
                 polartrack.mode,
                 (polartrack.no_contact)?'1':'8',
                 bearing,
                 vert,
                 polartrack.mode,
                 polartrack.battery,
                 polartrack.temp,
                 CR);
  }
  else
  {
    /*-----------------------------------------------------------
      polartrack is not "OFF" or there is contact with target
    -----------------------------------------------------------*/
    i = nav_system->p[0];                 /* only 1 station possible */
    range   = station[i].range * 100.0;   /* centimeters             */
    bearing = ship.bearing * 1000;        /* 0.001 degr.             */
    vert = ship.vert_angle * 1000;        /* 0.001 degr.             */
    sprintf(sys_polartrack.out_msg,"%06.0lf%06.0lf%06.0lfTTT%01d%01d%01d%c",
                 range,
                 bearing,
                 vert,
                 polartrack.mode,
                 polartrack.battery,
                 polartrack.temp,
                 CR);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: polartrack_cmd()
|    PURPOSE: Execute a command polartrack has received from
|             remote controller
| DESRIPTION: -
|    RETURNS: nothing
|    VERSION: 910425 V0.2
---------------------------------------------------------------------*/
static void polar_cmd(char *cmd)
{
  char      msg[80];
  char      tmpstr[20],vertstr[10];
  int       len;
  char      k;
  double    r;
  int       rx_error = FALSE;

  len = strlen(cmd);            /* Get length of command string */
  k = *cmd;                     /* Extract the command          */

  sprintf(msg,"001%c%01dE",k,polartrack.mode);

  /*--------------------------
    Check on valid commands
  ---------------------------*/
  switch (k)
  {
    case LOC:
    case STA:
    case MOV:
    case VER:
    case TED:
    case RED:
    case REF:
    case TST:
    case SLP:
    case LAM:
      disp_status("Polartrack received command %s",command[k-'0']);
      rx_error = FALSE;    /* Valid commands */
      break;
    default:
      rx_error = TRUE;
      disp_status("Polartrack command error: %s",cmd);
      break;
  }

  /*-----------------------------------------
    Process received command if it was valid
  -------------------------------------------*/
  if (!rx_error)
  {
    switch (k)
    {
      case LOC:
        if (len != 2) rx_error = TRUE;
        polartrack.mode = P_TRACK;          /* enter track mode  */
        polartrack.no_contact = FALSE;
        sprintf(tmpstr,"000000%c",CR);
        break;

      case STA :    /* NT has to send a status message */
        if (len != 2) rx_error = TRUE;
        if (polartrack.remote == 0)      /* enter remote mode */
          sta_rcvd++;
        if (sta_rcvd > 3)      /* enter remote mode after 3 times STA */
          polartrack.remote = 1;         /* remote enabled from here on      */
        sprintf(tmpstr,"%01d%01d%01d%01d00%c",
                       polartrack.remote,
                       polartrack.tracking,
                       polartrack.battery,
                       polartrack.temp,
                       CR);
        break;

      case MOV:
        if (len != 14) rx_error = TRUE;
        polartrack.mode = P_MEASURE;    /* enter measure mode */
        sprintf(tmpstr,"000000%c",CR);
        break;

      case VER :  /* NT has to send vertical angle */
        if (len != 2) rx_error = TRUE;
        r = ship.vert_angle * 1000;   /* vertical angle in 0.001 degrees */
        sprintf(tmpstr,"%06.0f%c",r,CR);
        break;

      case TED :    /* NT has to send extra data (e.g. info about format) */
        if (len != 2) rx_error = TRUE;
        sprintf(tmpstr,"000000%c",CR);         /* "TTTXXX" */
        if (nav_system->output_func == polar_long)
          tmpstr[3] = '1';
        if (nav_system->output_func == polar_short)
          tmpstr[3] = '0';
        break;

      case RED:      /*  -receive extra data- NT has to switch to specified */
                     /*       data format                                        */
        if (len != 8) rx_error = TRUE;
        strncpy(tmpstr,cmd+1,6);
        if (len == 8)
        {
          if (cmd[5] == '0')
          {
            nav_system->output_func = polar_short;
            nav_system->dt = 0.1;
          }
          if (cmd[5] == '1')
          {
            nav_system->output_func = polar_long;
            nav_system->dt = 0.2;
          }
        }
        break;

      case REF:
        if (len != 14) rx_error = TRUE;
        strncpy(tmpstr,cmd+1,6);
        strncpy(vertstr,cmd+7,6);
        ship.vert_angle = atof(vertstr) / 1000.0;
        polartrack.mode = P_MEASURE;
        strcat(tmpstr,"000000");
        break;

      case TST:
        if (len != 8) rx_error = TRUE;
        if (len > 7)
          strncpy(tmpstr,cmd+1,6);
        else
          strcpy(tmpstr,"YYYYY");
        break;

      case SLP:
        if (len != 2) rx_error = TRUE;
        polartrack.mode = P_OFF;        /* Enter "OFF" mode */
        polartrack.remote = 0;
        strcpy(tmpstr,"000000");
        nav_system->dt = 10000.0;
        break;

      case LAM:
        if (len != 14) rx_error = TRUE;
        polartrack.mode = P_TRACK;       /* enter "TRACK" mode   */
        polartrack.tracking = TRUE;      /* idem                 */
        polartrack.no_contact = FALSE;   /* enter "CONTACT" mode */
        strncpy(vertstr,cmd+7,6);        /* Get vert. angle      */
        ship.vert_angle = atof(vertstr) / 1000.0;
        strcpy(tmpstr,"000000");
        break;

      default:
        break;

    } /* switch */
  }

  strcat(msg,tmpstr);

  /*---------------------------------------
    Special messages if remote is blocked
  ----------------------------------------*/
  if (polartrack.remote == 0)
  {
    if (k == STA)
      sprintf(msg,"001%c%01d4%01d%01d%01d%01d00%c",
                   k,
                   polartrack.mode,
                   polartrack.tracking,
                   polartrack.battery,
                   polartrack.temp,
                   CR);
    else
      sprintf(msg,"001%c%01d4000000%c",
                   k,
                   polartrack.mode,
                   CR);
  }

  /*----------------------------------------------------
    Special message if an invalid command has been
    received, or if the message length was not correct.
    Set B5 to 8 and command info to '000000'.
  -----------------------------------------------------*/
  if (rx_error)
  {
    sprintf(msg,"rx_error: /%x/ 001%c%01d8000000%c",k,k,polartrack.mode,CR);
  }

  strcpy(sys_polartrack.out_msg,msg);
}

/*-------------------------------------------------------------------
|   FUNCTION: polar_key_action()
|    PURPOSE: Take special actions on key input
| DESRIPTION: -
|    RETURNS: nothing
|    VERSION: 910425 V0.2
---------------------------------------------------------------------*/
static void polar_key_action(int key)
{
  switch(key)
  {
    case '-':      /* Decrease vertical angle */
      if (ship.vert_angle > 0.1)
	  ship.vert_angle -= 0.1;
      display(DISP_VERTICAL);
	  break;

    case '+':   /* Increase vertical angle */
      if (ship.vert_angle < 179.9)
	  ship.vert_angle += 0.1;
      display(DISP_VERTICAL);
	  break;

    case 'B':  /* Toggle polartrack battery status */
      polartrack.battery++;
      if (polartrack.battery > 4)
        polartrack.battery = 1;
  	  break;

    case 'C':  /* Toggle contact status for POLARTRACK */
	  polartrack.no_contact = !polartrack.no_contact;
      break;

    case 'M':  /* Toggle polartrack mode */
      polartrack.mode = polartrack.mode<<1;
      if (polartrack.mode > 8)
        polartrack.mode = 1;
 	  break;

    case 'T':  /* Toggle polartrack temperature status */
      polartrack.temp++;
      if (polartrack.temp > 5)
        polartrack.temp = 1;
      break;

      default:
        break;
  }

}

#endif
