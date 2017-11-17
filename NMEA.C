/**********************************************************************
*                                                                     *
*        FILE: NMEA.C                                                 *
*                                                                     *
*     PURPOSE: Generate NMEA-0183 messages                            *
*                                                                     *
* DESCRIPTION: -                                                      *
*                                                                     *
*     HISTORY: 910502 V0.1 - Initial version                          *
*              911107 V0.2 - Splitted it up in seperate functions     *
*                            so it can be availabel from 'decca.c'    *
*                            and 'nr53.c'                             *
**********************************************************************/

#include "navsim.h"

#if NMEA_0183

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <window.h>

static void nmea_msg(void);
void change_NMEA_format(void);

NAV_SYSTEM sys_nmea_0183 =
{
  "NMEA_0183",   	          /* name of navigation system	 		*/
  NMEA_0183, 		          /* system number for case statements 	*/
  0,                          /* number of frequencies              */
  {0.0},                      /* operation frequencies		  		*/
  {1.0},                      /* propagation velocity		  		*/
  NONE_STATION,		          /* mode            		 			*/
  0,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  0,                          /* Number of formats                  */
  { "" },                     /* List of output formats             */
  { nmea_msg },               /* List of output functions           */
  0,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0},                      /* Standard deviations                */
  NULL,                       /* Initialization function            */
  nmea_msg,                   /* Output function                    */
  NULL,                       /* Input function                     */
  NULL,                       /* Special keyboard functions         */
  -1,                         /* Channel                            */
  -1,                         /* Slot1                              */
  4800,                       /* Baudrate                           */
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
static char talker[80] = "II";     /* Talker identifier */
static int  res3d = FALSE;         /* Resolution of GLL */
static int  gll = TRUE,            /* Message switches  */
            sll = TRUE,
            vhw = TRUE,
            zzu = TRUE,
            gga = FALSE,
            vtg = FALSE;

/*-------------------------------------------------------------------
|   FUNCTION: change_NMEA_format
|    PURPOSE: change Talker identifier and switch messages ON/OFF
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 910103 V0.1
---------------------------------------------------------------------*/
void change_NMEA_format()
{
  int choise;

  static MENU menu[] =
  {
    { "Talker ", -1, NULL, "NMEA", talker,  "%s" },
    { "   3D  ", -1, NULL, "NMEA", &res3d,  "%B" },
    { "   GLL ", -1, NULL, "NMEA", &gll,    "%B" },
    { "   VHW ", -1, NULL, "NMEA", &vhw,    "%B" },
    { "   SLL ", -1, NULL, "NMEA", &sll,    "%B" },
    { "   ZZU ", -1, NULL, "NMEA", &zzu,    "%B" },
    { "   GGA ", -1, NULL, "NMEA", &gga,    "%B" },
    { "   VTG ", -1, NULL, "NMEA", &vtg,    "%B" },
    { NULL }
  };

  do
  {
   trimstr(talker);
   talker[2] = '\0';              /* Only first 2 char's are valid */
   choise = popup(8,menu,-1,-1);   /* Handle menu                   */
  }
  while (choise != ESC);

}

/*-------------------------------------------------------------------
|   FUNCTION: unsigned int add_checksum(char *m)
|    PURPOSE: Calculate checksum of NMEA-0183 message 'm'
| DESRIPTION: The checksum value is calculated by XORing the 8 bits
|             of each valid data character in the sentence, except
|             the * used to indicate the checksum field. The absolute
|             value of this calculation is returned and will be
|             concatenated to the message as 2 characters representing
|             its hexadecimal value. Also a CR and LF will be added
|             to the message 'm';
|    RETURNS: The checksum.
|    VERSION: 910103 V0.1 - Initial version
|             911118 V0.2 - Also replaces spaces by character '0'.
---------------------------------------------------------------------*/
unsigned int add_checksum(char *m)
{
  int  i;
  int  len;
  BYTE sum = 0;
  char tmp[10];

  /*-----------------------------
    Replace spaces by '0' (zero).
  ------------------------------*/
  len = strlen(m);
  for (i=0 ; i<len ; i++)
    if (m[i] == ' ')
      m[i] = '0';

  /*---------------------
    Calculate checksum
  ----------------------*/
  i = 0;
  while (m[i])
  {
    sum = sum ^ (BYTE)m[i];
    i++;
  }

  /*-----------------------
    Add checksum to string
  -------------------------*/
  sprintf(tmp,"*%02X%c%c",sum,CR,LF);
  strcat(m,tmp);

  return(sum);
}

/*-------------------------------------------------------------------
|   FUNCTION: nmea_msg(char *msg)
|    PURPOSE: Generate NMEA-0183 message
| DESRIPTION: Print decca output message in string 'msg'.
|             We assume actual time is already determined in 'tmnow'.
|             If lat/long not already calculated, it will be done here.
|    RETURNS: Nothing
|    VERSION: 910103 V0.1
---------------------------------------------------------------------*/
static void nmea_msg()
{
  char m[80];
  char msg[256];

  msg[0] = '\0';     /* Clear message */
  m[0] = '\0';

  /*------------------------------------
  | GLL: Vessel position in lat/long
  -------------------------------------*/
  if (gll)
  {
    nmea_gll(talker,m,res3d);
    strcat(msg,m);
  }

  /*------------------------------------
  | SLL: Status of lat/long position
  -------------------------------------*/
  if (sll)
  {
    nmea_sll(talker,m);
    strcat(msg,m);
  }

  /*---------------------------------------------
  | VHW: True compass heading and log speed
  ---------------------------------------------*/
  if (vhw)
  {
    nmea_vhw(talker,m);
    strcat(msg,m);
  }

  /*----------------------
  | ZZU: Actual time
  -----------------------*/
  if (zzu)
  {
    nmea_zzu(talker,m);
    strcat(msg,m);
  }

  /*--------------------------------------------------------------
    GGA: time, lat, lon, qualty index, number of SV's used, HDOP,
         altitude above MSL, altitude above geoid
  ---------------------------------------------------------------*/
  if (gga)
  {
    nmea_vhw(talker,m);
    strcat(msg,m);
  }

  /*--------------------------------------------------------------
    VTG: Actual track and ground speed
  ---------------------------------------------------------------*/
  if (vtg)
  {
    nmea_vtg(talker,m);
    strcat(msg,m);
  }

  strcpy(sys_nmea_0183.out_msg,msg);

}

/*-------------------------------------------------------------------
|    FUNCTION: nmea_gll()
|     PURPOSE: Generate NMEA-0183 GLL message
| DESCRIPTION: Vessel postion in lat/long. This is in degrees, minutes
|              and hundreths of minutes. If res3d is TRUE, then it is
|              in degrees, minutes and thousend of minutes
|     RETURNS: nothing
|     HISTORY: 911107 V0.1 - Initial version
---------------------------------------------------------------------*/
void nmea_gll(char *talker, char *m, int res3d)
{
  char tmp[80];

  strcpy(m,"$");
  strcat(m,talker);
  strcat(m,"GLL,");
  if (res3d)
    sprintf(tmp,"%02d%06.3f,N,",ship.lat_deg,ship.lat_mm+ship.lat_ss/60.0);
  else
    sprintf(tmp,"%02d%05.2f,N,",ship.lat_deg,ship.lat_mm+ship.lat_ss/60.0);
  strcat(m,tmp);
  if (res3d)
    sprintf(tmp,"%03d%06.3f,E,",ship.lon_deg,ship.lon_mm+ship.lon_ss/60.0);
  else
    sprintf(tmp,"%03d%05.2f,E,",ship.lon_deg,ship.lon_mm+ship.lon_ss/60.0);
  strcat(m,tmp);
  add_checksum(m);
}

/*-------------------------------------------------------------------
|    FUNCTION: nmea_sll()
|     PURPOSE: Generate NMEA-0183 SLL message
| DESCRIPTION: Status of lat/long position
|     RETURNS: nothing
|     HISTORY: 911107 V0.1 - Initial version
---------------------------------------------------------------------*/
void nmea_sll(char *talker, char *m)
{
  strcpy(m,"$");
  strcat(m,talker);
  strcat(m,"SLL,");
  strcat(m,"A,");
  add_checksum(m);
}

/*-------------------------------------------------------------------
|    FUNCTION: nmea_vhw()
|     PURPOSE: Generate NMEA-0183 VHW message
| DESCRIPTION: VHW: True compass heading and log speed
|     RETURNS: nothing
|     HISTORY: 911107 V0.1 - Initial version
---------------------------------------------------------------------*/
void nmea_vhw(char *talker, char *m)
{
  char tmp[40];

  strcpy(m,"$");
  strcat(m,talker);
  strcat(m,"VHW,");
  sprintf(tmp,"%03.0f,T,,,%04.1f,N,,,",
          ship.heading, ship.speed * MS_TO_KNOTS);
  strcat(m,tmp);
  add_checksum(m);
}

/*-------------------------------------------------------------------
|    FUNCTION: nmea_zzu()
|     PURPOSE: Generate NMEA-0183 ZZU message
| DESCRIPTION: ZZU: Actual time
|     RETURNS: nothing
|     HISTORY: 911107 V0.1 - Initial version
---------------------------------------------------------------------*/
void nmea_zzu(char *talker, char *m)
{
  char tmp[40];

  strcpy(m,"$");
  strcat(m,talker);
  strcat(m,"ZZU,");
  sprintf(tmp,"%02d%02d%02d,",
          tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);
  strcat(m,tmp);
  add_checksum(m);
}

/*-------------------------------------------------------------------
|    FUNCTION: nmea_gga()
|     PURPOSE: Generate NMEA-0183 GGA message
| DESCRIPTION: GGA: time, lat, lon, qualty index, number of SV's used, HDOP,
|              altitude above MSL, altitude above geoid
|     RETURNS: nothing
|     HISTORY: 911107 V0.1 - Initial version
---------------------------------------------------------------------*/
void nmea_gga(char *talker, char *m)
{
  char tmp[80];

  strcpy(m,"$");
  strcat(m,talker);
  strcat(m,"GGA,");
  sprintf(tmp,"%02d%02d%02d,",
          tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);
  sprintf(tmp,"%02d%06.3f,N,",ship.lat_deg,ship.lat_mm+ship.lat_ss/60.0);
  strcat(m,tmp);
  sprintf(tmp,"%03d%06.3f,E,",ship.lon_deg,ship.lon_mm+ship.lon_ss/60.0);
  strcat(m,tmp);
  sprintf(tmp,"9,4,3,%3.0f,M,%4.0f,M",ship.h,ship.h+0.5);
  strcat(m,tmp);
  add_checksum(m);
}

/*-------------------------------------------------------------------
|    FUNCTION: nmea_vtg()
|     PURPOSE: Generate NMEA-0183 VTG message
| DESCRIPTION: VTG: actual track and ground speed
|     RETURNS: nothing
|     HISTORY: 911107 V0.1 - Initial version
---------------------------------------------------------------------*/
void nmea_vtg(char *talker, char *m)
{
  char tmp[80];

  strcpy(m,"$");
  strcat(m,talker);
  strcat(m,"VTG,");
  sprintf(tmp,"%5.1f,T,,,%4.1f,N,,",
        ship.heading, ship.speed * MS_TO_KNOTS);
  strcat(m,tmp);
  add_checksum(m);
}

#endif
