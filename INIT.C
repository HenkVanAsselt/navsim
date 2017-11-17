/*-----------------------------------------------------------------------

    SIMINIT.C

    Initlialization routines for NAVSIM navigation simulator

    900503 V1.1

------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <front9.h>
#include <window.h>
#include <time.h>
#include "navsim.h"

static char *error_str[] =
{
  "No new data",
  "OK",
  "Channel range error",
  "No empty slot available",
  "Slot range error",
  "Channel direction error",
  "Channel handshake error",
  "Channel baudrate error",
  "Channel databit error",
  "Channel stopbit error",
  "Channel parity error",
  "Framing error",
  "Parity error",
  "Overrun error",
  "Buffer overflow",
  "Slot empty (not in use)",
  "No UART present",
  "Buffer smaller then terminal count",
  "Channel not initialized"
};

/*-------------------------------------------------------------------
|    FUNCTION: Intro_screen()
|     PURPOSE: Intro screen with copyright message
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 910903 V0.2
---------------------------------------------------------------------*/
static void intro_screen(void);  /* prototype */

static void intro_screen()
{
  MENU menu[1];

  setup_menu(menu, 0, "OK", 0, NULL,  NULL, NULL, NULL , EXIT);

  wn_dialog(13,40,1,menu,5,
    "NAVIGATION SIMULATOR",
    "",
    "(c) H.B.J. van Asselt ",
    "    21 FEB 1992 V3.2  ",
    "");
}

/*-------------------------------------------------------------------
|    FUNCTION: exit_prog()
|     PURPOSE: exit from program
| DESCRIPTION: Perform this function only once:
|              Flush all channels, exit from frontend processor
|              Update system files, close window enviroment and
|              call exit().
|     RETURNS: nothing
|     HISTORY: 911124 V0.3
---------------------------------------------------------------------*/
void exit_prog()
{
  static int done = FALSE;
  int choise = ESC;
  int i;

  if (!done)               /* Do this only once */
  {
    /*-------------------------------------
      Ask if programm has to be terminated
    ---------------------------------------*/
    choise = wn_yesno("Terminate program ?");
    if (choise == 'N' || choise == ESC)
      return;

    /*----------------------------------------------------
      Flush all channels and exit from frontend processor
    ------------------------------------------------------*/
    for (i=0 ; i<MAX_CHANNELS ; i++)
    {
      if (systeem[i] != NULL)
      {
        channel_flush(&systeem[i]->channel);
      }
    }
    frontend_exit();

    /*---------------------
      Update systeem files
    -----------------------*/
    save_system("WORK.SYS");
    save_chain("WORK.CHN");

    wn_exit();          /* Exit from HvA window enviroment          */
    done = TRUE;        /* Set flag (this routine has been executed */
    exit(0);
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: initialize()
|     PURPOSE: Initialize navsim enviroment
| DESCRIPTION: initialize variables,
|              prints introduction screen,
|              reads "WORK.SYS" file,
|              read DOS system time,
|              opens serial port as a file;
|     RETURNS:
|     HISTORY: 911124 V0.3
---------------------------------------------------------------------*/
void initialize()
{
  int i;
  int succes;
  char *s;

  /*----------------------------
    Initilize (graphic) screen
  ----------------------------*/
  wn_init();

  /*-----------------------
    Set default chain name
  -------------------------*/
  strcpy(chain_name,"WORK.CHN");

  /*------------------------------------
    Initialize array of systems in use
  -------------------------------------*/
  for (i=0 ; i<MAX_CHANNELS ; i++)
  {
    systeem[i] = NULL;
  }

  /*----------------------------------------------------
    Initialize array of pointers to navigation systems
  ------------------------------------------------------*/
  for (i=0 ; i<MAX_SYSTEMS ; i++)
  {
    sys_array[i] = NULL;
  }

  /*------------------------------------
    Define number of navigation systems
  --------------------------------------*/
  #if TRISPONDER
    sys_array[no_navsystems] = &sys_trisponder;
    no_navsystems++;
  #endif
  #if ARTEMIS
    sys_array[no_navsystems] = &sys_artemis;
    no_navsystems++;
  #endif
  #if MICROFIX
    sys_array[no_navsystems] = &sys_microfix;
    no_navsystems++;
  #endif
  #if HYPERFIX
    sys_array[no_navsystems] = &sys_hyperfix;
    no_navsystems++;
  #endif
  #if DECCA
    sys_array[no_navsystems] = &sys_decca;
    no_navsystems++;
  #endif
  #if SYLEDIS
    sys_array[no_navsystems] = &sys_syledis;
    no_navsystems++;
  #endif
  #if POLARTRACK
    sys_array[no_navsystems] = &sys_polartrack;
    no_navsystems++;
  #endif
  #if FALCON
    sys_array[no_navsystems] = &sys_falcon;
    no_navsystems++;
  #endif
  #if UCM40
    sys_array[no_navsystems] = &sys_ucm40;
    no_navsystems++;
  #endif
  #if ECHOTRAC
    sys_array[no_navsystems] = &sys_echotrac;
    no_navsystems++;
  #endif
  #if NMEA_0183
    sys_array[no_navsystems] = &sys_nmea_0183;
    no_navsystems++;
  #endif
  #if NR53
    sys_array[no_navsystems] = &sys_nr53;
    no_navsystems++;
  #endif
  #if DESO25
    sys_array[no_navsystems] = &sys_deso25;
    no_navsystems++;
  #endif
  #if RADARFIX
    sys_array[no_navsystems] = &sys_radarfix;
    no_navsystems++;
  #endif
  #if PULSE8
    sys_array[no_navsystems] = &sys_pulse8;
    no_navsystems++;
  #endif
  #if TRIMBLE
    sys_array[no_navsystems] = &sys_trimble;
    no_navsystems++;
  #endif

  /*----------------------
    Initialize variables
  -----------------------*/
  latlong       = FALSE;
  s = getenv("DSC_DATA");
  if (s)
    strcpy(data_dir,s);       /* Directory for .PRJ and .SPH datafiles */
  else
    data_dir[0] = '\0';
  printer_out = FALSE;

  /*-------------------------------
    Initialize help file functions
  ---------------------------------*/
  init_help("navsim");
  intro_screen();

  /*--------------------------
    Initialize serial port
  ---------------------------*/
  install();

  /*--------------------------------------------
    Read last used data from 'work.sys' file
    and chaindata from 'work.chn'
  ---------------------------------------------*/
  strcpy(systemfile_name,"WORK.SYS");
  succes = load_system(systemfile_name);
  strcpy(chain_name,"WORK.CHN");
  succes = load_chain(chain_name);

  if (!succes)
    exit(-1);

  #define SYS systeem[i]
  for (i=0 ; i<MAX_CHANNELS ; i++)
  {
    if (SYS != NULL && SYS->channel > 0 && SYS->channel < MAX_CHANNELS)
    {
      set_io(SYS);
    }
  }
  #undef SYS

  if (nav_system->mode == HYPERBOLIC)
  {
    calc_baselines();
  }

  calc_geog_const();
  utmgeo(ship.x,ship.y,&ship.lat,&ship.lon);

  default_profile(&profile);

  atexit(exit_prog);

} /*initialize*/

/*-------------------------------------------------------------------
|    FUNCTION: set_io()
|     PURPOSE: Set the I/O of a navigation system
| DESCRIPTION: * Sets the channel parameters for a certain system,
|                and set the slot parameters (for receiving data),
|                making the slot number the same as the channel number.
|              * If an error occurs, an error message will be displayed
|                in a window with an 'OK' button for conformation.
|     RETURNS: nothing
|     HISTORY: 910822 V0.1 - Initial version
|              911106 V0.2 - Fixed error in displaying error information:
|                            the channel number was not in sprintf variable
|                            list.
---------------------------------------------------------------------*/
void set_io(NAV_SYSTEM *sys)
{
  char s[80];
  int  error = 0;

  /*----------------------------
    Check on duplicate channels
  ------------------------------*/
  /*
  for (i=0 ; i<MAX_CHANNELS ; i++)
  {
    if (systeem[i] != NULL)
    {
      for (j=i+1 ; j<MAX_CHANNELS ; j++)
      {
        if (systeem[j]->channel == systeem[i]->channel)
        {
          sprintf(s,"Duplicate assignment for channel %1d",systeem[i]->channel);
          wn_error(s);
        }
      }
    }
  }
  */

  error = 0;
  set_serial_io(sys->channel,
                'B',
                sys->handshake,
                sys->baudrate,
                sys->databits,
                sys->stopbits,
                sys->parity,
                sys->terminator,
                120);
  if (error)
  {
    sprintf(s,"Channel %2d: %s",
      sys->channel,error_str[++error]);
    wn_error(s);
  }
  sys->slot = sys->channel;
  /*
  set_serial_slot(sys->channel,"",0,0,sys->slot,&error);
  if (error)
  {
    sprintf(s,"Channel %2d / Slot %2d : %s",
      sys->channel,sys->slot,error_str[++error]);
    wn_error(s);
  }
  */
}
