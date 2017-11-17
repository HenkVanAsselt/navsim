/*==================================================================

  SIMUTIL.C	  901115 V1.5

  Include file with utility's for the NAVSIM navigation simulator

===================================================================*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <dos.h>
#include <conio.h>
#include <window.h>
#include <front9.h>
#include "navsim.h"

#ifdef __MSC__
#include <stdarg.h>
#endif

/*-------------------------------------------------------------------
|   FUNCTION: decode_bcd(char *msg, int n)
|    PURPOSE: Decode BCD message (e.g. Artemis Kongsberg format)
| DESRIPTION: This function decodes an 'n' byte BCD message by means of
|             conversion of a BCD byte to 2 characters,
|             representing the hexadecimal value of this byte.
|    RETURNS: Nothing
|    VERSION: 891106 V0.1
---------------------------------------------------------------------*/
void decode_bcd(char *s, int n)
{
  BYTE a;
  char t[40];
  int  i,j;

  i = 0;
  j = 0;

  do
  {
    a = ((BYTE) s[i] & 0xF0) >> 4;
    if (a < 10)
      t[j++] = a + '0';
    else
      t[j++] = a - 10 + 'A';
    a = ((BYTE) s[i] & 0x0F);
    if (a < 10)
      t[j++] = a + '0';
    else
      t[j++] = a - 10 + 'A';
    i++;
    n--;
  } while (n > 0);
  t[j] = '\0';               /* Terminate decoded string */

  strcpy(s,t);               /* Copy decoded string back in s */
}


/*-----------------------------------------------------------------
    Wait for a keypress
------------------------------------------------------------------*/
void wait()
{
  getch();
}

/*-------------------------------------------------------------------
    Read an key until Y(es) or N(o) or <RETURN> (== defualt)
    is pressed and return TRUE or FALSE
-------------------------------------------------------------------*/
int yes_no(char *msg)
{
  char *s = "N\0         ";

  editstr(10,10,20,msg,s,_DRAW);
  switch (s[0])
  {
    case 'Y':
    case 'y':
    case 'J':
    case 'j': return(TRUE);
    case 'N':
    case 'n': return(FALSE);
    default:  return(FALSE);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: msg_output(char *msg)
|    PURPOSE: Message output
| DESRIPTION: Function sends message serial port and/or printer
|             Artemis BCD message get a special treatment.
|    RETURNS: Nothing
|    HISTORY: 910425 V0.4 - #ifdef PC_SPEED then add LF when last
|                           char is CR
---------------------------------------------------------------------*/
void msg_output(char *msg)
{
  int  n;
  int  len;

  #if ARTEMIS
  /*-----------------------------------------------------------------
  | If this is a ARTEMIS BCD message, determine the number of bytes
  | to send and send these one by one
  ------------------------------------------------------------------*/
  #define OUT nav_system->output_func
  if (nav_system->sys == ARTEMIS)
  {
    if (OUT == bcd_01 || OUT == bcd_001 || OUT == bcd_adb)
    {
      n = 0;
      if (OUT == bcd_01 || OUT == bcd_001) n = 7;
      if (OUT == bcd_adb) n = 9;
      len = strlen(msg);
      output_string(&nav_system->channel,msg,&len);
      return;
    }
  }
  #undef OUT
  #endif

  /*---------------------
  | Normal ASCII output
  ----------------------*/
  len = strlen(msg);
  output_string(&nav_system->channel,msg,&len);
  msg[0] = '\0';   /* Reset message */

  /*------------------------------------------------
  | If printer output wanted, dump 'msg' to printer
  ------------------------------------------------*/
  if (printer_out)
    fprintf(stdprn,"%s",msg);

}

/*-------------------------------------------------------------------
|   FUNCTION: print_params()
|    PURPOSE: Print parameters on 'stdprn'
| DESRIPTION: Send parameters to printer after checking if prn online
|    RETURNS: Nothing
|    VERSION: 901103 V0.1
---------------------------------------------------------------------*/
void print_params()
{
  int i,j;
  union REGS reg;
  NAV_SYSTEM *sys;

  /*-------------------------
  | Check if printer online
  --------------------------*/
  reg.h.ah = 2;           /* function 2 = init printer */
  reg.x.dx = 0;           /* printer #0                */
  int86(0x17,&reg,&reg);
  if (reg.h.ah & 0x29)
  {
    wn_error("printer error");
    return;
  }

  fprintf(stdprn,"------------------------------\n");
  fprintf(stdprn,"|    NAVSIM - parameters     |\n");
  fprintf(stdprn,"------------------------------\n");

  /*------------------------
  | Print global parameters
  -------------------------*/
  fprintf(stdprn,"\n--------------------  SYSTEM PARAMETERS  --------------------\n");

  for (i=0 ; i<MAX_CHANNELS ; i++)
  {
    if (systeem[i] != NULL)
    {
      sys = systeem[i];
      fprintf(stdprn,"  System %2d: %s\n",i,sys->name);
      fprintf(stdprn,"      Format: %s\n",sys->format_names[sys->format]);
      fprintf(stdprn,"    Baudrate: %d\n",sys->baudrate);
      fprintf(stdprn,"    Patterns: \n");
      for (j=0 ; j<sys->max_patterns ; j++)
      {
        fprintf(stdprn,"patt %1d = %02d (%-15.15s)\n",
          j,sys->p[j], station[sys->p[j]].name);
      }
      fprintf(stdprn," Frequencies:\n");
      for (j=0 ; j<sys->no_frequencies ; j++)
      {
        fprintf(stdprn,"f%01d: %.0lf Hz    vc%01d: %.1lf m/s\n",
                 j,sys->f[j], j,sys->vc[j]);
      }
      fprintf(stdprn,"-------------------------------------------------------------\n");
    }
  }

  /*-----------------------------------
  | Print spheroid and projection parameters
  ------------------------------------*/

  fprintf(stdprn,"\n--------  SPHEROIDE  -----\n");
  fprintf(stdprn,"Name: %-20s\n",spheroid.name);
  fprintf(stdprn,"   a: %-20.0lf\n",spheroid.a);
  fprintf(stdprn," e^2: %-20.0lf\n",spheroid.e2);

  fprintf(stdprn,"\n--------   GRID  -----\n");
  fprintf(stdprn,"Name: %-20s\n", projection.name);
  fprintf(stdprn,"  e0: %-20.0lf\n",projection.e0);
  fprintf(stdprn,"  n0: %-20.0lf\n",projection.n0);
  fprintf(stdprn,"  k0: %-20lf\n",projection.k0);
  fprintf(stdprn,"lon0: %-20.0lf\n",projection.lon0);
  fprintf(stdprn,"lat0: %-20.0lf\n",projection.lat0);

  /*-------------------------
  | Print station parameters
  ---------------------------*/
  fprintf(stdprn,"\n--------------------  STATIONS  --------------------\n");
  fprintf(stdprn,"File name: %s\n",chain_name);
  fprintf(stdprn,"     date: %s\n",p_date);
  fprintf(stdprn,"     area: %s\n",p_area);
  fprintf(stdprn,"  remarks: %s\n\n",p_remarks);
  fprintf(stdprn,"   %10.10s  %4s  %-10s  %-10s  %-5s\n",
                    "name","code","X","Y","H");
  for (i=0 ; i<no_stations ; i++)
  {
    fprintf(stdprn,"%01d: %10.10s  %4d  %10.2f  %10.2f  %5.2f\n",
             i,station[i].name,station[i].code,
             station[i].x,station[i].y,station[i].h);
  }

  /*-----------------------
  | Print ship parameters
  ------------------------*/
  fprintf(stdprn,"\n--------------------  SHIP  --------------------\n");
  fprintf(stdprn,"      x: %.2lf\n",ship.x);
  fprintf(stdprn,"      y: %.2lf\n",ship.y);
  fprintf(stdprn,"  speed: %.1lf m/s\n",ship.speed);
  fprintf(stdprn,"heading: %.1lf degrees\n",ship.heading);
  fprintf(stdprn,"   helm: %.1lf\n",ship.helm);

  fprintf(stdprn,"\n------------------------------------------------------------\n");
  fprintf(stdprn,"\n\n\n");
}

/*-------------------------------------------------------------------
|   FUNCTION: check_printer()
|    PURPOSE: Checks if printer on line
| DESRIPTION: -
|    RETURNS: TRUE if printer ok, FALSE if not
|    VERSION: 901121 V0.1
---------------------------------------------------------------------*/
int check_printer()
{
  union REGS reg;

  /*-------------------------
  | Check if printer online
  --------------------------*/
  reg.h.ah = 2;           /* function 2 = init printer */
  reg.x.dx = 0;           /* printer #0                */
  int86(0x17,&reg,&reg);
  if (reg.h.ah & 0x29)
    return(FALSE);
  else
    return(TRUE);
}

/*-------------------------------------------------------------------
|    FUNCTION: print_debug()
|     PURPOSE: print debug info on terminal on COM2:
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 910828 V0.1 - Initial version
---------------------------------------------------------------------*/
void print_debug(char *format, ...)
{
  va_list arguments;
  static unsigned first_time = TRUE;
  int channel = 2;
  int error;
  char s[256];
  int  len;

  if (first_time)
  {
    set_serial_io(2,'B','N',19200,8,1,'N',LF,120);
    first_time = FALSE;
  }

  /*------------------------------------
    Write debug information on screen
  -------------------------------------*/
  va_start(arguments,format);
  vsprintf(s,format,arguments);
  strcat(s,"\r");
  len = strlen(s);
  output_string(&channel,s,&len);

  va_end(arguments);
}
