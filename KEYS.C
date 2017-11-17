/*------------------------------------------------------------------

  SIM_KEYS.C  900603 V1.2

  Unit with key-handling routines:

--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include "navsim.h"

#define FALSE 0
#define TRUE !FALSE

/*-----------------------------------------------------------------------
    A key has been pressed.
    Change or toggle parameters if a special key has been pressed.

    900603 V1.2
------------------------------------------------------------------------*/
void process_key()
{
  int   c,ch,i;
  char  help_str[40];
  NAV_SYSTEM *sys;

  extern int waitkey();
  extern void help(char *s);
  extern void gen_posstr(int posmode);

  c = waitkey();
  if (c >= 256)
  {
    /*--------------------
      Extended key code
    ---------------------*/
    switch (c)
    {
      case 328:   /* cursor up = increase speed */
  	    ship.speed = ship.speed + 1.0;
	    display(DISP_SPEED);
	    break;

      case 336:   /* cursor down = decrease speed */
	    ship.speed = ship.speed - 1.0;
	    display(DISP_SPEED);
	    break;

      case 331:   /* cursor left = decrease helm */
 	    if (ship.helm > -30.0)
	    {
	      ship.helm -= 1.0;
	      display(DISP_HELM);
        }
	    break;

      case 333:   /* cursor right = increase helm */
	    if (ship.helm < 30.0)
	    {
	      ship.helm += 1.0;
	      display(DISP_HELM);
        }
	    break;

      case 371 :   /* CTRL cursor left = decrease heading */
	    ship.heading -= 1.0;
	    check_bear(&ship.heading);
	    display(DISP_HEADING);
	    break;

      case 372 :   /* CTRL cursor right = increase heading */
	    ship.heading += 1.0;
	    check_bear(&ship.heading);
	    display(DISP_HEADING);
	    break;

      case 315 :  /* F1 function key */
        sprintf(help_str,"%s",nav_system->name);
        strlwr(help_str);
        update_display = FALSE;  /* Disable display updating     */
        help(help_str);          /* Get help for this nav system */
        update_display = TRUE;   /* Enable display updating      */
        break;

      default:
        break;

    } /* switch c of ... */

  }
  else
  {
    /*-------------------------
      Key had a normal code
    --------------------------*/
    ch = toupper(c);
    switch (ch)
    {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        i = ch - '1';
        if (systeem[i] != NULL)
          system_in_command = i;
        display(DISP_SYSTEM_IN_COMMAND);
        break;

      case 'L':  /* Toggle between display/calculation of lat/lon or grid coordinates */
        switch (posmode)
        {
          case UTM:         posmode = LATLON_DDMM; break;
          case LATLON_DD:   posmode = LATLON_DDMM; break;
          case LATLON_DDMM: posmode = LATLON_DDMMSS; break;
          default:          posmode = UTM;
        }
        gen_posstr(posmode);
	    break;

      case 'P':  /* Toggle printer output */
        toggle_printer();
        break;

      case '+':  /* Increase ship height */
        ship.h += 0.1;
        break;

      case '-':  /* Decrease ship heigth */
        ship.h -= 0.1;
        break;

    } /* switch ch of ...*/

    /*---------------------------------------------------
      Check if special system dependent keyboard actions
      should be taken. If so, go for it...
    ------------------------------------------------------*/
    sys = systeem[system_in_command];   /* Get pntr to sytem in command */
    if (sys->keybd_func)
      (*(sys->keybd_func))(ch);

  } /* else */
}


