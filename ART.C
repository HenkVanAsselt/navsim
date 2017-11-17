/*-------------------------------------------------------------------

  ARTMIS.C 900603 V1.2

  Module with artemis routines:

--------------------------------------------------------------------*/

#include "navsim.h"

#if ARTEMIS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <conio.h>

/* Prototypes */

static void ascii_16(void);
static void ascii_17(void);
extern void art_telegram(void);
extern void init_artemis(void);
extern void artemis_cmd(char *cmd);
static void artemis_key_action(int key);

NAV_SYSTEM sys_artemis =      /* Artemis Mark III en Mark IV        */
{
  "Artemis",                  /* name of navigation system          */
  ARTEMIS,                    /* system number for case statements  */
  1,                          /* number of frequencies              */
  {9.2e9,},                   /* operation frequencies              */
  {299.65e6,},                /* propagation velocity               */
  RANGE_BEARING,              /* mode                               */
  1,                          /* maximum number of patterns         */
  1,                          /* Number of patters system is using  */
  {1},                        /* Selected stations or patterns      */
  6,                          /* Number of formats                  */
  { "ASCII 16",               /* list of format names               */
    "ASCII 17",
    "BCD (0.01)",
    "BCD (0.001)",
    "BCD (0.001) + ADB",
    "Telegram (Mark IV)",
  },
  {
    ascii_16,                 /* list of output functions           */
    ascii_17,
    bcd_01,
    bcd_001,
    bcd_adb,
    art_telegram,
  },
  0,                          /* Actual format                      */
  2,                          /* Number of standard deviations      */
  {0.75,0.0015},              /* Standard deviations                */
  init_artemis,               /* Initialization function            */
  ascii_16,                   /* Output function                    */
  artemis_cmd,                /* Input function                     */
  artemis_key_action,         /* Special keyboard functions         */
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

static int  artemis_status = 1;       /* ASCII 16/17 status byte    */
static double range, azimuth;
static BYTE bcd[20];

/*-------------------------------------------------------------------
|    FUNCTION: ascii_16()
|     PURPOSE: Format artemis ascii-16 message
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 900421 V0.2
---------------------------------------------------------------------*/
static void ascii_16()
{
  int i;

  i = find_station(nav_system->p[0]);
  if (i >= 0)
  {
    range = station[i].range * 10;   /* decimeters */
    azimuth = ship.azimuth * 100;
    sprintf(sys_artemis.out_msg,"%06.0f %05.0f %1d%c%c",range,azimuth,artemis_status,CR,LF);
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: ascii_17()
|     PURPOSE: Format artemis ascii-17 message
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 900421 V0.2
---------------------------------------------------------------------*/
static void ascii_17()
{
  int i;

  i = find_station(nav_system->p[0]);
  if (i >= 0)
  {
    range = station[i].range * 10;    /* decimeters */
    azimuth = ship.azimuth * 1000;
    sprintf(sys_artemis.out_msg,"%06.0f %06.0f %1d%c%c",range,azimuth,artemis_status,CR,LF);
  }
}

/*-------------------------------------------------------------------
|    FUNCTION: bcd_01()
|     PURPOSE: Format artemis (Kongsberg) KA_BCD(.01) message
| DESCRIPTION: 7 byte BCD telegram with distance resolution of 1.0 m
|              and azimuth resolution of 0.01 degree.
|     RETURNS: nothing
|     HISTORY: 900421 V0.2
---------------------------------------------------------------------*/
void bcd_01()
{
  int i;
  char s[10];
  char msg[20];

  for (i=0 ; i<7 ; i++) bcd[i] = '\0';  /* Clear byte buffer */

  sprintf(s,"%06.2lf",ship.azimuth);    /* 0.01 degrees */
  bcd[0] = (((s[0]-'0') & 0x0F) << 4) | ((s[1]-'0') & 0x0F);
  bcd[1] = (((s[2]-'0') & 0x0F) << 4) | ((s[4]-'0') & 0x0F);
  bcd[2] = (s[5]-'0')   & 0x0F;

  i = find_station(nav_system->p[0]);
  if (i >= 0)
  {
    sprintf(s,"%05.0lf",station[i].range);    /* meters */
    bcd[3] = (((s[0]-'0') & 0x0F) << 4) | ((s[1]-'0') & 0x0F);
    bcd[4] = (((s[2]-'0') & 0x0F) << 4) | ((s[3]-'0') & 0x0F);
    bcd[5] = ((s[4]-'0')  & 0x0F);
    bcd[6] = 0xFF;
  }

  for (i=0 ; i<7 ; i++)
    msg[i] = (char) bcd[i];
  msg[7]='\0';

  strcpy(sys_artemis.out_msg,msg);
}

/*-------------------------------------------------------------------
|    FUNCTION: bcd_001()
|     PURPOSE: Format artemis (Kongsberg) KA_BCD(.001) message
| DESCRIPTION: 7 byte BCD telegram with distance resolution of 1.0 m
|              and azimuth resolution of 0.001 degree.
|     RETURNS: nothing
|     HISTORY: 900421 V0.2
---------------------------------------------------------------------*/
void bcd_001()
{
  int i;
  char s[10];
  char msg[10];

  for (i=0 ; i<7 ; i++) bcd[i] = '\0';  /* Clear byte buffer */

  sprintf(s,"%07.3lf",ship.azimuth);       /* 0.001 degrees */
  bcd[0] = (((s[0]-'0') & 0x0F) << 4) | ((s[1]-'0') & 0x0F);
  bcd[1] = (((s[2]-'0') & 0x0F) << 4) | ((s[4]-'0') & 0x0F);
  bcd[2] = (((s[5]-'0') & 0x0F) << 4) | ((s[6]-'0') & 0x0F);

  i = find_station(nav_system->p[0]);
  if (i >= 0)
  {
    sprintf(s,"%05.0lf",station[i].range);    /* meters */
    bcd[3] = (((s[0]-'0') & 0x0F) << 4) | ((s[1]-'0') & 0x0F);
    bcd[4] = (((s[2]-'0') & 0x0F) << 4) | ((s[3]-'0') & 0x0F);
    bcd[5] = ((s[4]-'0')  & 0x0F);
    bcd[6] = 0xFF;
  }

  for (i=0 ; i<7 ; i++)
    msg[i] = (char) bcd[i];
  msg[7]='\0';

  strcpy(sys_artemis.out_msg,msg);

}

/*-------------------------------------------------------------------
|    FUNCTION: bcd_adb()
|     PURPOSE: Format artemis (Kongsberg) KA_BCD(.001) message
| DESCRIPTION: 9 byte BCD telegram with distance resolution of 0.1 m
|              and azimuth resolution of 0.001 degree and Mobile
|              Antenna Bearing with a resolution of 0.1 degree.
|     RETURNS: nothing
|     HISTORY: 900421 V0.2
---------------------------------------------------------------------*/
void bcd_adb()
{
  int i;
  double bearing;
  char s[10];
  char msg[10];

  for (i=0 ; i<9 ; i++) bcd[i] = '\0';  /* Clear byte buffer */

  sprintf(s,"%07.3lf",ship.azimuth);       /* 0.001 degrees */
  bcd[0] = (((s[0]-'0') & 0x0F) << 4) | ((s[1]-'0') & 0x0F);
  bcd[1] = (((s[2]-'0') & 0x0F) << 4) | ((s[4]-'0') & 0x0F);
  bcd[2] = (((s[5]-'0') & 0x0F) << 4) | ((s[6]-'0') & 0x0F);

  i = find_station(nav_system->p[0]);
  if (i >= 0)
  {
    sprintf(s,"%07.1lf",station[i].range);    /* meters */
    bcd[3] = (((s[0]-'0') & 0x0F) << 4) | ((s[1]-'0') & 0x0F);
    bcd[4] = (((s[2]-'0') & 0x0F) << 4) | ((s[3]-'0') & 0x0F);
    bcd[5] = (((s[4]-'0') & 0x0F) << 4) | ((s[6]-'0') & 0x0F);
  }

  bearing = ship.bearing + 180.0;
  check_bear(&bearing);
  sprintf(s,"%05.1lf",bearing);
  bcd[6] = (((s[0]-'0') & 0x0F) << 4) | ((s[1]-'0') & 0x0F);
  bcd[7] = (((s[2]-'0') & 0x0F) << 4) | ((s[4]-'0') & 0x0F);

  bcd[8] = 0xFF;
  for (i=0 ; i<9 ; i++)
    msg[i] = (char) bcd[i];
  msg[9]='\0';

  strcpy(sys_artemis.out_msg,msg);
}


/*-------------------------------------------------------------------
|   FUNCTION: artemis_key_action(int key)
|    PURPOSE: React on special operator keyboard inputs
| DESRIPTION: _
|    RETURNS: Nothing
|    VERSION: 901109 V0.1
---------------------------------------------------------------------*/
void artemis_key_action(int key)
{
  switch (key)
  {
    case 'S':
      artemis_status = !artemis_status;   /* Toggle status bit */
      break;
    default :
      break;
  }
}

#endif
