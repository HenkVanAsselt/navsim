/*=================================================================

   SIMDISP.C

   900603  V1.1

   Contains display output routines for NAVSIM navigation simulator

=================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <window.h>
#include <front9.h>
#include <time.h>
#include "navsim.h"

#ifdef __MSC__
#include <stdarg.h>
#endif

/*------------------
| Local variables
------------------*/
static WINDOW *ship_window;      /* Ship info: x,y,speed,heading  */
static WINDOW *pos_window;       /* X, Y, Range, Bearing, Azimuth */
static WINDOW *output_window;
static WINDOW *input_window;

/*-------------------------------------------------------------------
|   FUNCTION: display_all()
|    PURPOSE: display all navsim parameters
| DESRIPTION: -
|    RETURNS: nothing
|    VERSION: 910408 V0.1
---------------------------------------------------------------------*/
void display_all()
{
  display(DISP_SYSTEM_IN_COMMAND);
  display(DISP_TIME);
  display(DISP_SPEED);
  display(DISP_HEADING);
  display(DISP_HELM);
  display(DISP_DEPTH);
  display(DISP_X);
  display(DISP_Y);
  display(DISP_H);
  display(DISP_RANGE);
  display(DISP_BEARING);
  display(DISP_AZIMUTH);
  display(DISP_VERTICAL);
}

/*-------------------------------------------------------------------
|   FUNCTION: clear_all()
|    PURPOSE: display all simulator windows
| DESRIPTION: -
|    RETURNS: nothing
|    VERSION: 911107 V0.1
---------------------------------------------------------------------*/
void clear_all_windows()
{
  wn_clear(output_window);
  wn_locate(output_window,0,0);

  wn_clear(input_window);
  wn_locate(input_window,0,0);

  wn_clear(pos_window);
  wn_locate(pos_window,0,0);

  wn_clear(ship_window);
  wn_locate(ship_window,0,0);
}


/*-------------------------------------------------------------------
|    FUNCTION:
|     PURPOSE:
| DESCRIPTION:
|     RETURNS:
|     HISTORY:
---------------------------------------------------------------------*/
void open_disp_window(int w)
{
  int output_row;
  int input_row;
  int output_ysize;
  int input_ysize;

  input_row    = MAXROW - 6;
  output_row   = 9;
  input_ysize  = 6;
  output_ysize = (input_row - output_row);

  switch (w)
  {
    case SHIP_WINDOW:
      ship_window   = wn_open(_DOUBLE_LINE,1,0,(int)MAXXSIZE/2,8,
                      _window_att,_window_att);
      wn_title(ship_window,"SHIP");
      break;

    case POSITION_WINDOW:
      pos_window = wn_open(_DOUBLE_LINE,1,(int)MAXXSIZE/2,(int)MAXXSIZE/2-1,8,
                      _window_att,_window_att);
      wn_title(pos_window,"POSITION");
      break;

    case OUTPUT_WINDOW:
      output_window = wn_open(_DOUBLE_LINE,output_row,0,MAXXSIZE,output_ysize,
                      _window_att,_window_att);
      wn_title(output_window,"OUTPUT");
      break;

    case INPUT_WINDOW:
      input_window  = wn_open(_DOUBLE_LINE,input_row,0,MAXXSIZE,input_ysize,
                      _window_att,_window_att);
      wn_title(input_window,"INPUT");
      break;

    default:
      break;
  }
}

/*-------------------------------------------------------------------
|    FUNCTION:
|     PURPOSE:
| DESCRIPTION:
|     RETURNS:
|     HISTORY:
---------------------------------------------------------------------*/
void clear_disp_window(int w)
{
  switch (w)
  {
    case INPUT_WINDOW:
      wn_clear(input_window);
      break;

    case OUTPUT_WINDOW:
      wn_clear(output_window);
      break;

    case SHIP_WINDOW:
      wn_clear(ship_window);
      break;

    case POSITION_WINDOW:
      wn_clear(pos_window);
      break;

    default:
      break;
  }
}

/*-------------------------------------------------------------------
|    FUNCTION:
|     PURPOSE:
| DESCRIPTION:
|     RETURNS:
|     HISTORY:
---------------------------------------------------------------------*/
void close_disp_window(int w)
{
  switch (w)
  {
    case INPUT_WINDOW:    input_window = wn_close(input_window);  break;
    case OUTPUT_WINDOW:   output_window = wn_close(output_window); break;
    case SHIP_WINDOW:     ship_window = wn_close(ship_window);   break;
    case POSITION_WINDOW: pos_window = wn_close(pos_window);   break;
    default:              break;
  }
}

/*-------------------------------------------------------------------
|    FUNCTION:
|     PURPOSE:
| DESCRIPTION:
|     RETURNS:
|     HISTORY:
---------------------------------------------------------------------*/
void disp_param_info()
{
  static MENU menu[] =
  {
    { "Ok", 0, NULL, NULL, NULL, NULL, EXIT},
    { NULL    }
  };

  wn_dialog(13,40,1,menu,5,
              "PARAMETER INFO",
              systemdata_str,
              p_date,
              p_area,
              p_remarks);
}

/*-------------------------------------------------------------------
|   FUNCTION: display(int item)
|    PURPOSE: Display an item on screen
| DESRIPTION: Function displays an item formatted on screen.
|    RETURNS: Nothing
|    VERSION: 901104 V0.3
---------------------------------------------------------------------*/
void display(int item_no)
{
  double r;

  /*------------------------------------
    If updating of display is disabled,
    then quit this function
  -------------------------------------*/
  if (!update_display) return;

  switch (item_no)
  {
    /*------------------------
      Information about ship
    -------------------------*/
    case DISP_TIME:
      wn_locate(ship_window,0,0);
      wn_printf(ship_window,"%8.8s: %02d:%02d:%02d",
              "time",tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);
      break;

    case DISP_SPEED:
      wn_locate(ship_window,1,0);
      wn_printf(ship_window,"%8.8s: %7.1f","speed",ship.speed);
      break;

    case DISP_HEADING:
      wn_locate(ship_window,2,0);
      wn_printf(ship_window,"%8.8s: %7.1lf","heading",ship.heading);
      break;

    case DISP_HELM:
      wn_locate(ship_window,3,0);
      wn_printf(ship_window,"%8.8s: %7.1lf","helm",ship.helm);
      break;

    case DISP_DEPTH:
      wn_locate(ship_window,4,0);
      wn_printf(ship_window,"%8.8s: %5.1lf %5.1lf","depths",depth1,depth2);
      break;

    case DISP_SYSTEM_IN_COMMAND:
      wn_locate(ship_window,5,0);
      wn_printf(ship_window,"%8.8s: %-10.10s",
        "CMD SYS",systeem[system_in_command]->name);
      break;

    /*--------------------------
      Information about postion
    ----------------------------*/
    case DISP_X:
      wn_locate(pos_window,0,0);
      wn_printf(pos_window,"%8.8s: %13s",
                (posmode == UTM ? "X" : "lon"), ship.xpos_str);
      break;

    case DISP_Y:
      wn_locate(pos_window,1,0);
      wn_printf(pos_window,"%8.8s: %13s",
                (posmode == UTM ? "Y" : "lat"), ship.ypos_str);
      break;

    case DISP_H:
      wn_locate(pos_window,2,0);
      wn_printf(pos_window,"%8.8s: %10.2lf","H",ship.h);
      break;

    case DISP_RANGE:
      if (nav_system->mode == RANGE_BEARING)
      {
        wn_locate(pos_window,3,0);
        r = station[0].range;
        wn_printf(pos_window,"%8.8s: %10.2lf","range",r);
      }
      break;

    case DISP_BEARING:
      if (nav_system->mode == RANGE_BEARING)
      {
        wn_locate(pos_window,4,0);
        wn_printf(pos_window,"%8.8s: %10.2lf","bearing",ship.bearing);
      }
      break;

    case DISP_AZIMUTH:
      if (nav_system->mode == RANGE_BEARING)
      {
        wn_locate(pos_window,5,0);
        wn_printf(pos_window,"%8.8s: %10.2lf","azimuth",ship.azimuth);
      }
      break;

    case DISP_VERTICAL:
      if (nav_system->mode == RANGE_BEARING_VERTICAL)
      {
        wn_locate(pos_window,6,0);
        wn_printf(pos_window,"%8.8s: %10.2lf","vertical",ship.vert_angle);
      }
      break;

    default:
      break;
  }
}

/*-------------------------------------------------------------------
   FUNCTION: disp_input()
    PURPOSE: Display inputstring in input window
 DESRIPTION: Function displays an string formatted on screen.
             An artemis BCD message will be decoded.
    RETURNS: Nothing
    VERSION: 911106 V0.1 - Initial version
             06-Sep-1993 12:51:29 V0.2 - Linefeed will be given at
               start of incoming message to make full use of window
---------------------------------------------------------------------*/
void disp_input(char *msg)
{
  char s[256];
  int i,len;

  extern void bcd_01();
  extern void bcd_001();
  extern void bcd_adb();

  #define SYS nav_system->sys
  #define OUT nav_system->output_func

  strcpy(s,msg);

  #if ARTEMIS
  /*----------------------------
  | Decode ARTEMIS BCD messages
  ------------------------------*/
  if (OUT == bcd_01 || OUT == bcd_001)
  {
    wn_printf(input_window,"%s\n",msg);
    for (i=0 ; i<7 ; i++)   /* Fill s again, but ignore NULL bytes */
      s[i] = msg[i+12];
    decode_bcd(s,7);
  }
  if (OUT == bcd_adb)
  {
    wn_printf(input_window,"%s\n",msg);
    for (i=0 ; i<9 ; i++)   /* Fill s again, but ignore NULL bytes */
      s[i] = msg[i+12];
    decode_bcd(s,9);
  }
  #endif

  /*-----------------------------------------------------
    Check if last character of the string is a linefeed.
    If not, add one to the string
  ------------------------------------------------------*/
  len = strlen(s);
  if (len < 256 && s[len-1] == LF)
    s[len-1] = '\0';

  wn_printf(input_window,"\n%s",s);

  #undef SYS
  #undef FMT
}

/*-------------------------------------------------------------------
   FUNCTION: disp_output()
    PURPOSE: Display outputstring in output window
 DESRIPTION: Function displays string formatted on screen.
             An artemis BCD message will be decoded.
    RETURNS: Nothing
    VERSION: 911106 V0.1 - Initial version
             06-Sep-1993 V0.2 - Linefeed in output window will be
               given at start of message
---------------------------------------------------------------------*/
void disp_output(char *msg)
{
  char s[256];
  int i,len;

  extern void bcd_01();
  extern void bcd_001();
  extern void bcd_adb();

  #define SYS nav_system->sys
  #define OUT nav_system->output_func

  strcpy(s,msg);

  #if ARTEMIS
  /*----------------------------
  | Decode ARTEMIS BCD messages
  ------------------------------*/
  if (OUT == bcd_01 || OUT == bcd_001)
  {
    for (i=0 ; i<7 ; i++)   /* Fill s again, but ignore NIL's */
      s[i] = nav_system->out_msg[i];
    decode_bcd(s,7);
  }
  if (OUT == bcd_adb)
  {
    for (i=0 ; i<9 ; i++)   /* Fill s again, but ignore NIL's */
      s[i] = nav_system->out_msg[i];
    decode_bcd(s,9);
  }
  #endif

  /*-----------------------------------------------------
    Check if last character of the string is a linefeed.
    If it is, delete this LF. The linefeed for the window
    will be given at the start of the message.
  ------------------------------------------------------*/
  len = strlen(s);
  if (len < 256 && s[len-1] == LF)
    s[len-1] = '\0';

  wn_printf(output_window,"\n%s",s);

  #undef SYS
  #undef FMT
}
