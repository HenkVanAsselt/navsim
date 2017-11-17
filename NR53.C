/******************************************************
*
*  Module with routines for SERCEL NR53 GPS ontvanger
*
*  Includes:
*
*  HISTORY: 910425 V0.1
*
*******************************************************/

#include "navsim.h"

#if NR53

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <window.h>
#include <front9.h>

/*------------------
  Local Prototypes
-------------------*/
static void msg_a1_a4(void);
static void msg_a5(void);
static void msg_b6(void);
static void msg_b8(void);
static void msg_b10(void);

static char tmp[250];

NAV_SYSTEM sys_nr53 =
{
  "NR_53_103",                /* name of navigation system                      */
  NR53,                       /* system number for case statements      */
  1,                          /* number of frequencies              */
  {9.0e9,},                   /* operation frequencies		  		*/
  {299.650e6,},               /* propagation velocity		  		*/
  NONE_STATION,               /* mode                                               */
  0,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  7,                          /* Number of formats                  */
  {                           /* list of format names               */
    "A1...A4",
    "A5",
    "B6",
    "B7",
    "B8",
    "B9",
    "B10",
  },
  {                           /* List of output functions           */
    msg_a1_a4,
    msg_a5,
    msg_b6,
    msg_b6,                   /* B7 is same as B6 message */
    msg_b8,
    msg_b8,                   /* B9 is same as B8 message */
    msg_b10,
  },
  0,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0},                      /* Standard deviations                */
  NULL,                       /* Initialization function            */
  msg_a1_a4,                  /* Output function                    */
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
static double hdop = 1.0;
static double lpme = 1.0;

/*-------------------------------------------------------------------
|    FUNCTION: space_to_nul()
|     PURPOSE: Convert space to '0' characters in a string
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 911120 V0.1 - Initial version
---------------------------------------------------------------------*/
static void space_to_nul(char *s);   /* Prototype */
static void space_to_nul(char *s)
{
  int i = 0;

  while(s[i] != '\0')
  {
    if (s[i] == ' ')
      s[i] = '0';
    i++;
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: msg_a1_a4()
|    PURPOSE: Generate Sercel NR53 A1,A2,A3 and A4 messages
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910425 V0.1 - First version
---------------------------------------------------------------------*/
static void msg_a1_a4()
{
  char msg[256];

  /*-----------------------------------
  | A1 - Time, date, ship lat/lon/alt
  ------------------------------------*/
  sprintf(msg,"%c%c$%c%cZDA,%02d%02d%02d.0,%02d,%02d,%4d,6.0 %c%c",
             CR,LF,CR,LF,tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec,
             tmnow->tm_mday,tmnow->tm_mon,tmnow->tm_year+1900,CR,LF);
  sprintf(tmp,"GLL,%02d%08.5f,N,",ship.lat_deg,ship.lat_mm+ship.lat_ss/60.0);
  strcat(msg,tmp);
  sprintf(tmp,"%03d%08.5f,E,",ship.lon_deg,ship.lon_mm+ship.lon_ss/60.0);
  strcat(msg,tmp);
  sprintf(tmp,"%08.2f,M%c%c",ship.h,CR,LF);
  strcat(msg,tmp);

  space_to_nul(msg);

  /*--- PERFORM INTERMEDIATE OUTPUT */
  strcpy(sys_nr53.out_msg,msg);
  display(DISP_OUTPUT);
  msg_output(sys_nr53.out_msg);

  /*-------------------------
  | A2 - VTG, SGD and SYS
  -------------------------*/
  sprintf(msg,"VTG,%5.1lf,T,%5.2lf,M,00.00,V%c%c",
               ship.heading,ship.speed,CR,LF);
  sprintf(tmp,"SGD,%6.1f,H,%6.1f,M%c%c",hdop,lpme,CR,LF);
  strcat(msg,tmp);
  sprintf(tmp,"SYS,3T,0009%c%c",CR,LF);
  strcat(msg,tmp);

  space_to_nul(msg);

  /*--- PERFORM INTERMEDIATE OUTPUT  */
  strcpy(sys_nr53.out_msg,msg);
  display(DISP_OUTPUT);
  msg_output(sys_nr53.out_msg);

  /*-----------------------------
  | A3 - ZEV and NSV[0]..NSV[1]
  ------------------------------*/
  sprintf(msg,"ZEV,x.xxxxxExxx%c%c",CR,LF);
  sprintf(tmp,"NSV,03,00,000,00, 1.2,00.0,00,00,D%c%c",CR,LF);
  strcat(msg,tmp);
  sprintf(tmp,"NSV,02,00,000,00, 1.2,00.0,00,00,D%c%c",CR,LF);
  strcat(msg,tmp);

  space_to_nul(msg);

  /*--- PERFORM INTERMEDIATE OUTPUT  */
  strcpy(sys_nr53.out_msg,msg);
  display(DISP_OUTPUT);
  msg_output(sys_nr53.out_msg);

  /*----------------------
  | A4 - NSV[2]..NSV[4]
  -----------------------*/
  sprintf(msg,"NSV,11,00,000,00,-1.1,00.0,00,00,D%c%c",CR,LF);
  sprintf(tmp,"NSV,09,00,000,00, 2.2,00.0,00,00,D%c%c",CR,LF);
  strcat(msg,tmp);
  sprintf(tmp,"NSV,04,00,000,00,-0.9,00.0,00,00,D%c%c",CR,LF);
  strcat(msg,tmp);
  sprintf(tmp,"&");
  strcat(msg,tmp);

  space_to_nul(msg);

  /*--- NO INTERMEDIATE OUTPUT                  */
  /*--- This will be performed by main routine  */
  strcpy(sys_nr53.out_msg,msg);

}

/*-------------------------------------------------------------------
|   FUNCTION: msg_a5()
|    PURPOSE: Generate Sercel NR53 A5 message
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910425 V0.1 - First version
---------------------------------------------------------------------*/
static void msg_a5()
{
  char msg[132];

  sprintf(msg,"%c%c$%c%c",CR,LF,CR,LF);
  sprintf(tmp,"MRK,%02d%c%c",nav_system->eventnr%100,CR,LF);
  strcat(msg,tmp);
  sprintf(tmp,"ZDA,%02d%02d%02d.0,%02d,%02d,%4d,%c%c",
             tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec,
             tmnow->tm_mday,tmnow->tm_mon,tmnow->tm_year,CR,LF);
  strcat(msg,tmp);
  sprintf(tmp,"GLL,%02d%07.5f,N,",ship.lat_deg,ship.lat_mm+ship.lat_ss/60.0);
  strcat(msg,tmp);
  sprintf(tmp,"%03d%07.5f,E,",ship.lon_deg,ship.lon_mm+ship.lon_ss/60.0);
  strcat(msg,tmp);
  sprintf(tmp,"%08.2f,M%c%c",ship.h,CR,LF);
  strcat(msg,tmp);

  space_to_nul(msg);

  strcpy(sys_nr53.out_msg,msg);
}

/*-------------------------------------------------------------------
|   FUNCTION: msg_b6()
|    PURPOSE: Generate Sercel NR53 B6 and B7 message
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910425 V0.1 - First version
---------------------------------------------------------------------*/
static void msg_b6()
{
  char msg[132];

  sprintf(msg,"$GPGGA,%02d%02d%02d,",
          tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);
  sprintf(tmp,"%02d%06.3lf,N,",
          ship.lat_deg,(double)((double)ship.lat_mm+ship.lat_ss/60.0));
  strcat(msg,tmp);
  sprintf(tmp,"%03d%06.3lf,E,",
          ship.lon_deg,(double)((double)ship.lon_mm+ship.lon_ss/60.0));
  strcat(msg,tmp);
  sprintf(tmp,"9,4,3,%.0lf,M,%.0lf,M%c%c",
          ship.h,ship.h+0.5,CR,LF);
  strcat(msg,tmp);
  sprintf(tmp,"$GPVTG,%5.1lf,T,,,%04.1lf,N,,%c%c",
          ship.heading, ship.speed * MS_TO_KNOTS,CR,LF);
  strcat(msg,tmp);

  space_to_nul(msg);

  strcpy(sys_nr53.out_msg,msg);
}

/*-------------------------------------------------------------------
|   FUNCTION: msg_b8()
|    PURPOSE: Generate Sercel NR53 B8 and B9 message
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910425 V0.1 - First version
---------------------------------------------------------------------*/
static void msg_b8()
{
  char msg[80];

  sprintf(msg,"$GPGLL,%02d%06.3f,N,",ship.lat_deg,ship.lat_mm+ship.lat_ss/60.0);
  sprintf(tmp,"%03d%06.3f,E%c%c",ship.lon_deg,ship.lon_mm+ship.lon_ss/60.0,CR,LF);
  strcat(msg,tmp);

  space_to_nul(msg);

  strcpy(sys_nr53.out_msg,msg);
}

/*-------------------------------------------------------------------
|   FUNCTION: msg_b10()
|    PURPOSE: Generate Sercel NR53 B10 message
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910425 V0.1 - First version
---------------------------------------------------------------------*/
static void msg_b10()
{
  char msg[80];

  sprintf(msg," OOO %02d:%02d:%02d.000 Y=%11.2lf X=%11.2lf "
              "S= %5.2lf C= %3.0lf Q=3 N=5%c%c",
          tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec,
          ship.y,ship.x,ship.speed,ship.heading,CR,LF);

  space_to_nul(msg+1);

  strcpy(sys_nr53.out_msg,msg);
}

#endif
