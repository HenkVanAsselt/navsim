#include "navsim.h"

#if DECCA

#include <stdio.h>
#include <string.h>
#include <time.h>

/*-------------
  Prototype's
--------------*/
static void decca_msg(void);

NAV_SYSTEM sys_decca =
{
  "Decca",                    /* name of navigation system           */
  DECCA,                      /* system number for case statements   */
  3,                          /* number of frequencies               */
  {1902.8e3,                  /* operation frequencies               */
   1752.8e3,
   1752.8e3,
  },
  {299.650e6,                 /* propagation velocity                */
   299.650e6,
   299.650e6
  },
  NONE_STATION,               /* mode                                */
  0,                          /* maximum number of patterns          */
  0,                          /* Number of patters system is using   */
  {0},                        /* Selected stations or patterns       */
  0,                          /* Number of formats                   */
  {""},                       /* list of format names                */
  {decca_msg},                /* List of output functions            */
  0,                          /* Actual format                       */
  1,                          /* Number of standard deviations       */
  {0.2},                      /* Standard deviations                 */
  NULL,                       /* Initialization function             */
  decca_msg,                  /* Output function                     */
  NULL,                       /* Input function                      */
  NULL,                       /* Special keyboard functions          */
  -1,                         /* Channel                             */
  -1,                         /* Slot                                */
  4800,                       /* Baudrate                            */
  8,                          /* Databits                            */
  1,                          /* Stopbits                            */
  'N',                        /* Parity                              */
  'N',                        /* Handshake                           */
  LF,                         /* Terminator                          */
  120,                        /* Terminal count                      */
};

/*-------------------------------------------------------------------
|   FUNCTION: decca_msg()
|    PURPOSE: Decca message
| DESRIPTION: Print decca output message in string 'msg'.
|             Messages: $IIGLL, $PRSLL, $IIVHW and $IIZZU.
|    RETURNS: Nothing
|    VERSION: 901104 V0.2
---------------------------------------------------------------------*/
static void decca_msg()
{
  char tmp[80];
  char msg[256];

  msg[0] = '\0';
  tmp[0] = '\0';

  nmea_gll("II",tmp,0);   /* Vessel position in lat/long in 2 decimals */
  strcat(msg,tmp);

  nmea_sll("PR",tmp);     /* Status of lat/long position */
  strcat(msg,tmp);

  nmea_vhw("II",tmp);     /* True compass heading and log speed */
  strcat(msg,tmp);

  nmea_zzu("II",tmp);     /* Actual time */
  strcat(msg,tmp);

  strcpy(sys_decca.out_msg,msg);
}

#endif
