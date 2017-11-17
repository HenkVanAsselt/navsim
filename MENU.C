/*=================================================================

	 SIMMENU.C   890507  V0.2

     UNIT with menu's and information windows
	 for NAVSIM navigation simulator

=================================================================*/

#include "navsim.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <window.h>
#include <time.h>
#include <front9.h>

/*-------------------------------------------------------------------
|   FUNCTION: check_pattern()
|    PURPOSE: Check patterns
| DESRIPTION: Checks if pattern configuration is possible
|    RETURNS: 1 if pattern configuration ok, 0 if not
|    VERSION: 901120 V0.4
---------------------------------------------------------------------*/
int check_pattern()
{
  int  ok = TRUE;      /* Default return value */
  int  i,j;            /* loop counters        */
  char s[80];          /* Gen. purpose string  */
  int  master,slave;
  char *token;

  if (nav_system->mode == NONE_STATION)
    return(TRUE);

  /*------------------------------------------------
  |  Check TRISPONDER, MICROFIX
  |  The station codes in the pattern have to be in
  |  the array of stations
  -------------------------------------------------*/
  if (nav_system->sys == TRISPONDER ||
      nav_system->sys == MICROFIX)
  {
    for (i=0 ; i<nav_system->max_patterns ; i++)
    {
      if (nav_system->p[i] != 0)
      {
        j = find_station(nav_system->p[i]);
        if ( j < 0)
        {
          sprintf(s,"no station available with code %2d",nav_system->p[i]);
          wn_error(s);
          return(FALSE);
        }
      }
    }
    return(TRUE);
  }

  /*--------------------------------------------------
  | Check FALCON.
  | The pattern represents the sides selected
  | Check if selected site ( = station) is available
  | by checking the coordinates.
  ---------------------------------------------------*/
  else if (nav_system->sys == FALCON)
  {
    for (i=0 ; i<nav_system->max_patterns ; i++)
    {
      if (nav_system->p[i] != 0)  /* Check if this station is available */
	  {
        ok = FALSE;     /* Assume error */
        if (station[i].name[0] != '\0')
        {
         ok = TRUE;  /* Reset error */
        }
        if (!ok)
        {
          sprintf(s,"no site #%1d available",nav_system->p[i]);
          wn_error(s);
          return(FALSE);
        }
      }
    }
    return(TRUE);
  }

  /*-----------------------------------------------------------
  | Check HYPERFIX
  | In the pattern there must be a master and a slave station.
  | These may not be the same station
  ------------------------------------------------------------*/
  else if (nav_system->sys == HYPERFIX)
  {
    for (i=0 ; i<nav_system->max_patterns ; i++)
    {
      if (nav_system->p[i])    /* If pattern available */
	  {
 	    master = find_master(nav_system->p[i]);
	    slave  = find_slave(nav_system->p[i]);
	    if (!master || !slave || (master == slave))
        {
	      sprintf(s,"error in pattern code %2d",nav_system->p[i]);
          wn_error(s);
          return(FALSE);
        }
	  }
    }
    return(TRUE);
  }

  /*-------------------------------------------------------------
  | Check Range/Bearing stations
  | There is only 1 pattern available, and the
  | code has to be assigned to a station in the list of stations
  --------------------------------------------------------------*/
  else if (nav_system->mode == RANGE_BEARING)
  {
    j = find_station(nav_system->p[0]);
    if (j >= 0)
        return(TRUE);
    else
    {
      sprintf(s,"no station available with code %2d",nav_system->p[0]);
      wn_error(s);
      return(FALSE);
    }
  }

  /*-------------------------------------------------------------
  | Check SYLEDIS
  | This is a special case. The station selection is embedded in
  | 'nav_system->format_str'. Codes 11...18 will give rough
  | distance output for a selected beacon. We will check if for
  | each given code a station is available.
  --------------------------------------------------------------*/
  else if (nav_system->sys == SYLEDIS)
  {
    j = 0;
    strcpy(s,nav_system->format_names[nav_system->format]);
    token = strtok(s," \t,");   /* Token separtors are SPACE, TAB or ','   */
    while (token)
    {
      i = atoi(token) - 11;     /* Convert to int and adjust for 0 base    */
      if (i >= 0 && i<nav_system->max_patterns) /* Do we deal with a raw range code ? */
      {
        nav_system->p[j++] = station[i].code;
      }
      token = strtok(NULL," \t,");
    }
  }

  /*--------------------------------------------
  | All other systems return success as default
  ----------------------------------------------*/
  return(TRUE);

}

/*-------------------------------------------------------------------
|    FUNCTION: popup_parities()
|     PURPOSE: Popup available parities
| DESCRIPTION: -
|     RETURNS: (char) parity selected
|     HISTORY: 920407 V0.1 - Initial version
---------------------------------------------------------------------*/
char popup_parities(void);    /* prototype */
char popup_parities()
{

  #define  NO_PARITIES 3

  int choise;
  MENU menu[NO_PARITIES];

  setup_menu(menu, 0, "   None   ", 3, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 1, "   Odd    ", 3, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 2, "   Even   ", 3, NULL, NULL, NULL, NULL, 0);

  choise = popup(NO_PARITIES,menu,10,10);

  switch (choise)
  {
    case 'N':
    case 'O':
    case 'E': return((char)choise);
              break;
    default:  break;
  }

  return(ESC);
}

/*-------------------------------------------------------------------
|    FUNCTION: popup_databits()
|     PURPOSE: Popup available databits
| DESCRIPTION: -
|     RETURNS: (char) number of databits selected
|     HISTORY: 920407 V0.1 - Initial version
---------------------------------------------------------------------*/
char popup_databits(void);    /* prototype */
char popup_databits()
{

  #define  NO_DATABITS 2

  int choise;
  MENU menu[NO_DATABITS];

  setup_menu(menu, 0, "   7  ", 3, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 1, "   8  ", 3, NULL, NULL, NULL, NULL, 0);

  choise = popup(NO_DATABITS,menu,10,10);

  switch (choise)
  {
    case '7': return(7);
    case '8': return(8);
    default:  return(8);
  }

  return(8);
}

/*-------------------------------------------------------------------
|    FUNCTION: popup_stopbits()
|     PURPOSE: Popup available stopbits
| DESCRIPTION: -
|     RETURNS: (char) number of stopbits selected
|     HISTORY: 920407 V0.1 - Initial version
---------------------------------------------------------------------*/
char popup_stopbits(void);    /* prototype */
char popup_stopbits()
{

  #define  NO_STOPBITS 2

  int choise;
  MENU menu[NO_STOPBITS];

  setup_menu(menu, 0, "   1  ", 3, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 1, "   2  ", 3, NULL, NULL, NULL, NULL, 0);

  choise = popup(NO_STOPBITS,menu,10,10);

  switch (choise)
  {
    case '1': return(1);
    case '2': return(2);
    default:  return(1);
  }

  return(1);
}

/*-------------------------------------------------------------------
|    FUNCTION: popup_handshakes()
|     PURPOSE: Popup available handshakes
| DESCRIPTION: -
|     RETURNS: (char) handshake selected
|     HISTORY: 910909 V0.1 - Initial version
---------------------------------------------------------------------*/
char popup_handshakes(void);    /* prototype */
char popup_handshakes()
{

  #define  NO_HANDSHAKES 3

  int choise;
  MENU menu[NO_HANDSHAKES];

  setup_menu(menu, 0, "   None      ", 3, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 1, "   Xon/Xoff  ", 3, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 2, "   DSR/CTS   ", 3, NULL, NULL, NULL, NULL, 0);

  choise = popup(NO_HANDSHAKES,menu,10,10);

  switch (choise)
  {
    case 'N':
    case 'X':
    case 'D': return((char)choise);
    default:  return('N');
  }

  return('N');
}

/*-------------------------------------------------------------------
|    FUNCTION: popup_baudrates()
|     PURPOSE: Popup available baudrates
| DESCRIPTION: -
|     RETURNS: Baudrate selected
|     HISTORY: 910909 V0.1 - Initial version
---------------------------------------------------------------------*/
int popup_baudrates(void);    /* prototype */
int popup_baudrates()
{

  #define  NO_BAUDRATES 7

  int choise;
  MENU menu[NO_BAUDRATES];

  setup_menu(menu, 0, "    300  ", 4, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 1, "    600  ", 4, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 2, "   1200  ", 4, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 3, "   2400  ", 4, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 4, "   4800  ", 4, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 5, "   9600  ", 3, NULL, NULL, NULL, NULL, 0);
  setup_menu(menu, 6, "  19200  ", 2, NULL, NULL, NULL, NULL, 0);

  choise = popup(NO_BAUDRATES,menu,10,10);
  if (choise == ESC)
    return(0);

  switch (choise)
  {
    case '3': return(300);
    case '6': return(600);
    case '2': return(1200);
    case '4': return(2400);
    case '8': return(4800);
    case '9': return(9600);
    case '1': return(19200);
    default: return(0);
  }
  return(0);
}

/*-------------------------------------------------------------------
|    FUNCTION: popup_navsystems()
|     PURPOSE: pop up available navsystems and return choise
| DESCRIPTION: -
|     RETURNS: choise (int)
|     HISTORY: 910827 V0.1
---------------------------------------------------------------------*/
static int popup_navsystems(void);   /* Prototype */
static int popup_navsystems()
{
  int  choise;
  char s[MAX_SYSTEMS][40];
  MENU menu[MAX_SYSTEMS];
  int  i,j;

  init_menu(menu,MAX_SYSTEMS);

  menu[0].header = "...NONE...";

  j = 1;
  for (i=0 ; i<MAX_SYSTEMS ; i++)
  {
    if (sys_array[i] != NULL)
    {
      sprintf(s[j],"  %-s  ",sys_array[i]->name);
      setup_menu(menu, i+1, s[j], -1, NULL, sys_array[i]->name, NULL, NULL, 0);
      j++;
    }
  }

  choise = popup(no_navsystems+1,menu,5,5);

  return(choise);
}

/*-------------------------------------------------------------------
|   FUNCTION: popup_formats()
|    PURPOSE: change output format of a given navigation system
| DESRIPTION: For Hyper-Fix, Syledis and NMEA-0183, the format choise
|             will be handled in seperate functions. For the other
|             systems, a popup menu will appear.
|    RETURNS: Nothing
|    VERSION: 900103 V0.3
|             911111 V0.4 - ESC will not change the current format
---------------------------------------------------------------------*/
static void popup_formats(NAV_SYSTEM *sys);
static void popup_formats(NAV_SYSTEM *sys)
{
  extern void change_syledis_format(void);
  extern void change_NMEA_format(void);

  char s[MAX_FORMATS][80];
  int i;
  MENU menu[MAX_FORMATS];

  nav_system = sys;

  /*--------------------
  | Handle exceptions
  --------------------*/
  #if SYLEDIS
  if (sys->sys == SYLEDIS)
  {
    change_syl_format();
    return;
  }
  #endif

  #if HYPERFIX
  if (sys->sys == HYPERFIX)
  {
    sys->format = 0;    /* Default */
    change_pattern();
    return;
  }
  #endif

  #if NMEA_0183
  if (sys->sys == NMEA_0183)
  {
    change_NMEA_format();
    return;
  }
  #endif

  if (sys->no_formats > 0)
  {
    /*---------------------------------
       Fill menu item with format text
    -----------------------------------*/
    for (i=0 ; (i<sys->no_formats && i<MAX_FORMATS) ; i++)
    {
      sprintf(s[i],"%2d   %s   ",i+1,sys->format_names[i]);
      setup_menu(menu, i, s[i], -1, NULL, "formats", NULL, NULL, 0);
    }

    /*-----------------------------------------------
      Make a choise from possible formats
      if ESC is used, format is will not be changed
    ------------------------------------------------*/
    i = popup(sys->no_formats,menu,5,5);
    if (i != ESC)
    {
      sys->format = i;
      sys->output_func = sys->output_list[sys->format];
    }
  }
  else
  {
    sprintf(s[0],"Only default format available for %s",sys->name);
    wn_error(s[0]);
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: popup_sysparams()
|     PURPOSE: Define new system parameters
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 910905 V0.1 - Initial version
|              911111 V0.2 - Parameters indicated by 'define'
---------------------------------------------------------------------*/
static void popup_sysparams(int sysnr, NAV_SYSTEM *sys);   /* Prototype */
static void popup_sysparams(int sysnr, NAV_SYSTEM *sys)
{
  #define NO_PARAMS   12
  #define SYSTEM       0
  #define FORMAT       1
  #define CHANNEL      2
  #define BAUDRATE     3
  #define DATABITS     4
  #define PARITY       5
  #define STOPBITS     6
  #define HANDSHAKE    7
  #define FREQUENCIES  8
  #define PATTERNS     9
  #define INTERVAL    10
  #define SPECIAL     11

  int i,j;
  int choise = ESC;
  MENU menu[NO_PARAMS];
  char pattern_str[80];
  char t[40];

  do
  {
    choise = ESC;
    i = 0;
    nav_system = sys;

    /*---------------------------
      Initialize pattern string
    ----------------------------*/
    strcpy(pattern_str,"");
    for (j = 0 ; j<nav_system->max_patterns ; j++)
    {
       sprintf(t,"%2d ",nav_system->p[j]);
       strcat(pattern_str,t);
    }

    if (sys == NULL)
    {
       /*--------------------------------------
         Reset all parameters to "-" except
         system function
       --------------------------------------*/
      setup_menu(menu, 0,  "System     ",  1, NULL, NULL, NULL, NULL, NO_FLAGS);
      setup_menu(menu, 1,  "Format     ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 2,  "Channel    ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 3,  "Baudrate   ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 4,  "Databits   ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 5,  "Parity     ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 6,  "Stopbits   ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 7,  "Handshake  ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 8,  "Frequencies", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 9,  "Patterns   ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 10, "Interval   ", -1, NULL, NULL, NULL, NULL, DISABLED);
      setup_menu(menu, 11, "Special    ", -1, NULL, NULL, NULL, NULL, DISABLED);
    }
    else
    {
      setup_menu(menu, 0, "System     ", 1,NULL, sys->name,  sys->name,      "%-10.10s", NO_FLAGS);
      setup_menu(menu, 1, "Format     ", 0,NULL, sys->name,  sys->format_names[sys->format], "%-10.10s", NO_FLAGS);
      setup_menu(menu, 2, "Channel    ", 0,NULL, "channel",  &sys->channel,  "%1d",  EDIT_STR);
      setup_menu(menu, 3, "Baudrate   ", 0,NULL, "baudrate", &sys->baudrate, "%d",   NO_FLAGS);
      setup_menu(menu, 4, "Databits   ", 0,NULL, "databits", &sys->databits, "%d",   NO_FLAGS);
      setup_menu(menu, 5, "Parity     ", 0,NULL, "parity",   &sys->parity,   "%c",   NO_FLAGS);
      setup_menu(menu, 6, "Stopbits   ", 0,NULL, "stopbits", &sys->stopbits, "%1d",  NO_FLAGS);
      setup_menu(menu, 7, "Handshake  ", 0,NULL, "handshake",&sys->handshake,"%c",   NO_FLAGS);
      setup_menu(menu, 8, "Frequencies", 1,change_frequency, NULL, NULL, NULL, NO_FLAGS);
      setup_menu(menu, 9 ,"Patterns   ", 2,change_pattern,   NULL, pattern_str, "%s", NO_FLAGS);
      setup_menu(menu, 10,"Interval   ", 0,NULL, NULL, &sys->dt,  "%lf", EDIT_STR);
      setup_menu(menu, 11,"Special    ", 2,NULL, sys->name, NULL, NULL, 0);
    }

    choise = popup(NO_PARAMS,menu,5,5);

    switch (choise)
    {
      case 'Y':
        i = popup_navsystems();
        if (i > 0)
          systeem[sysnr] = sys_array[i-1];    /* -1 because 1st one is ..NONE.. */
        else
          systeem[sysnr] = NULL;
        sys = systeem[sysnr];
        nav_system = sys;
        break;
      case 'F':
        popup_formats(nav_system);
        break;
      case 'B':
        sys->baudrate = popup_baudrates();
        break;
      case 'D':
        sys->databits = popup_databits();
        break;
      case 'S':
        sys->stopbits = popup_stopbits();
        break;
      case 'P':
        sys->parity = popup_parities();
        break;
      case 'H':
        sys->handshake = popup_handshakes();
        break;
      case 'E':
        change_spec_params();
        break;
      default:
        break;
    }

    /*-------------------------------------------------
      Set serial i/o if one of the channel parameters
      have been changed
    --------------------------------------------------*/
    switch (choise)
    {
      case 'C':
      case 'B':
      case 'D':
      case 'P':
      case 'S':
      case 'H':
             set_io(nav_system);
             break;
      default:
             break;
    }

  }
  while (choise != ESC);

  #undef NO_PARAMS
}

/*-------------------------------------------------------------------
|   FUNCTION: change_spec_params()
|    PURPOSE: Change special (navigation system dependent) parameters
| DESRIPTION: The called functions are within the system source files
|    RETURNS: Nothing
|    VERSION: 901113 V0.1
---------------------------------------------------------------------*/
void change_spec_params()
{
  extern void change_ucm40_params(void);

  switch(nav_system->sys)
  {
    #if UCM40
    case UCM40: change_ucm40_params(); break;
    #endif
    default   : wn_error("no special parameters for this system");
                break;
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: change_xsystems()
|     PURPOSE: Define new navigation systems to simulate
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 910823 V0.1 - Initial version
---------------------------------------------------------------------*/
void change_xsystems()
{
  int i;
  int choise = ESC;
  int choise2 = ESC;
  char s[MAX_CHANNELS][80];
  MENU menu[MAX_CHANNELS];

  /*-------------------------------
  | Fill menu with systems defined
  ---------------------------------*/
  do
  {
    #define SYS systeem[i]
    for (i=0 ; i<MAX_CHANNELS ; i++)
    {
      if (SYS != NULL)
      {
        sprintf(s[i],"%2d %-10.10s %-10.10s %c [%5d/%1d/%c/%1d] %c",
                i+1,
                SYS->name,
                SYS->format_names[SYS->format],
                ((SYS->channel == -1) ? '-' : SYS->channel + '0'),
                SYS->baudrate,
                SYS->databits,
                SYS->parity,
                SYS->stopbits,
                SYS->handshake);

      }
      else
      {
        sprintf(s[i],"%2d %-10.10s %-10.10s %1s [%-5.5s/%1s/%1s/%1s] %1s",
                i+1,"-","-","-","-","-","-","-","-");
      }
      setup_menu(menu, i, s[i], -1, NULL, NULL, NULL, NULL, NO_FLAGS);
      menu[i].help = SYS->name;        /* Setup help for this system */
    }
    #undef SYS

    choise = popup(MAX_CHANNELS,menu,10,10);

    if (choise >= 0 && choise < MAX_CHANNELS)
      popup_sysparams(choise,systeem[choise]);

  }
  while (choise != ESC);

  nav_system = systeem[0];
}


/*-----------------------------------------------------------------------
   Change frequencies

   900710 V0.2
-----------------------------------------------------------------------*/
void change_frequency()
{
  int i;
  int choise;
  char s[MAX_FREQS][40];
  char s1[40];                     /* Edit buffer */
  char *header = "frequency xxx";
  int ret;
  MENU menu[2];

  init_menu(menu,2);

  menu[0].header = "Ok";
  menu[0].hotkey = 0;
  menu[0].flags = EXIT;
  menu[1].header = "Edit";
  menu[1].hotkey = 0;
  menu[1].flags = EXIT;

  /*----------------------------------------
  | Check if frequency editing is possible
  -----------------------------------------*/
  if (nav_system->no_frequencies == 0)
  {
    wn_error("No frequency editing possible");
    return;
  }

  do
  {
    /*---------------
    | GENERATE TEXT
    ----------------*/
    for (i=0 ; i<nav_system->no_frequencies ; i++)
      sprintf(s[i],"f%01d: %10.0lf",i+1,nav_system->f[i]);

    choise = wn_dialog(13,40,2,menu,nav_system->no_frequencies,
             s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7]);

    if (choise == 'O' || choise == ESC) break;

    /*-----------------
    | Edit frequencies
    ------------------*/
    if (choise == 'E')
    {
      for (i=0 ; i<nav_system->no_frequencies ; i++)
      {
        sprintf(s1,"%10.0lf",nav_system->f[i]);
        sprintf(header,"frequency %01d",i+1);
        ret = editstr(10,10,20,header,s1,_DRAW);
        if (ret == ESC)
          continue;
        nav_system->f[i] = atof(s1);
      }
    }
  } while (TRUE);

}

/*-------------------------------------------------------------------
|   FUNCTION: change_pattern()
|    PURPOSE: Change pattern for current system
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901103 V0.2 - ...
|             911111 V0.3 - Pattern editting by dialog menu
---------------------------------------------------------------------*/
void change_pattern()
{
  extern void change_syl_format();

  #define NO_ITEMS MAX_PATTERNS+1

  int  i;
  int  choise;
  char s[NO_ITEMS][80];
  char s1[40];
  char *header = "pattern xx";
  int ret;
  MENU menu[2];

  setup_menu(menu, 0, "Ok",   0, NULL, NULL, NULL, NULL, EXIT);
  setup_menu(menu, 1, "Edit", 0, NULL, NULL, NULL, NULL, EXIT);

  if (nav_system->max_patterns == 0)
  {
    wn_error("No pattern selection possible");
    return;
  }

  #if SYLEDIS
  if (nav_system->sys == SYLEDIS)
  {
    change_syl_format();
    return;
  }
  #endif

  do
  {
    /*---------------
    | GENERATE TEXT
    ----------------*/
    for (i=0 ; i<nav_system->max_patterns ; i++)
      sprintf(s[i],"pattern %2d:  %2d",i+1,nav_system->p[i]);

    choise = wn_dialog(13,40,2,menu,nav_system->max_patterns,
             s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7]);

    if (choise == 'O' || choise == ESC) break;

    /*---------------
    | Edit patterns
    ----------------*/
    if (choise == 'E')    /* E(dit) */
    {
      for (i=0 ; i<nav_system->max_patterns ; i++)
      {
        sprintf(s1,"%2d",nav_system->p[i]);
        sprintf(header,"pattern %02d",i+1);
        ret = editstr(10,10,20,header,s1,_DRAW);
        if (ret == ESC)
          continue;
        nav_system->p[i] = atoi(s1);
      }
    }

  }
  while (TRUE);

}

/*-------------------------------------------------------------------
|   FUNCTION: toggle_printer()
|    PURPOSE: Toggles the global variable 'printer_out'
| DESRIPTION: If user wants to toggle printer on, this functions tests
|             if printer is online and returns no errors
|    RETURNS: TRUE if printer_out is set TRUE, FALSE if not
|    VERSION: 901121 V0.1
---------------------------------------------------------------------*/
void toggle_printer()
{
  if (printer_out == FALSE)
  {
    if (check_printer())
      printer_out = TRUE;
    else
      wn_error("Printer error");
  }
  else
    printer_out = FALSE;
}

