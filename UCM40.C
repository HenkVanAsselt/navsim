#include "navsim.h"

#if UCM40

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <front9.h>
#include <time.h>
#include <window.h>

/*------------------
| Local functions
------------------*/
static void update_ucm40_params(void);
static void ucm40_msg(void);

NAV_SYSTEM sys_ucm40 =
{
  "UCM_40", 	              /* name of navigation system	 		*/
  UCM40,                      /* system number for case statements      */
  0,                          /* number of frequencies              */
  {0.0,},                     /* operation frequencies		  		*/
  {1.0,},                     /* propagation velocity		  		*/
  NONE_STATION,               /* mode                                               */
  0,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  0,                          /* Number of formats                  */
  {""},                       /* list of format names               */
  {ucm40_msg},                /* List of output functions           */
  0,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0},                      /* Standard deviations                */
  NULL,                       /* Initialization function            */
  ucm40_msg,                  /* Output function                    */
  NULL,                       /* Input function                     */
  NULL,                       /* Special keyboard functions         */
  -1,                         /* Channel                            */
  -1,                         /* Slot1                              */
  300,                        /* Baudrate                           */
  8,                          /* Databits                           */
  1,                          /* Stopbits                           */
  'N',                        /* Parity                             */
  'N',                        /* Handshake                          */
  LF,                         /* Terminator                         */
  120,                        /* Terminal count                     */
};


/*-------------------
| Local variables
-------------------*/
static int   id = 4;               /* Instrument identification number */
static int   e  = 0;               /* Error code                       */
static float h_vl = 81.0;          /* Vertical velocity (mm/s)         */
static float h_vl_min = 73.0;
static float h_vl_max = 89.0;
static float d_h_vl = 1.0;
static float s_vl = 1482.0;        /* Sound velocity (m/s)             */
static float s_vl_min = 1470.0;
static float s_vl_max = 1490.0;
static float d_s_vl = 1.0;
static float temp = 1926.0;        /* Sea temperature (1/100 C)        */
static float temp_min = 1800.0;
static float temp_max = 1999.0;
static float d_temp = 1.0;
static float dept = 0.0001;          /* Current meter depth (m)          */
static float dept_min = 0.0;
static float dept_max = 20.0;
static float d_dept = 1.0;
static float cond = 4841.0;        /* Conductivity (1e-5 mho/cm)       */
static float saln = 3603.0;        /* Salinity (ppm)                   */
static float saln_min = 3550.0;
static float saln_max = 3650.0;
static float d_saln = 1.0;
static double dens = 102774.0;  /* Density (1e-5 kg/dm^3)           */
static float tilt_x = -1.0;      /* Tilt in X direction (deg.)       */
static float tilt_y = -1.0;      /* Tilt in Y direction (deg.)       */

/*-------------------------------------------------------------------
|   FUNCTION: ucm40_msg()
|    PURPOSE: Generate UCM40 message in string 'msg'
| DESRIPTION: The UCM40 message contains some parameters which will
|             change during the simulation. After each output the
|             parameters will be updated.
|    RETURNS: Nothing
|    VERSION: 901108 V0.1
---------------------------------------------------------------------*/
void ucm40_msg()
{
  #define PAGELEN 20

  static int line = PAGELEN;      /* Start with printing header */
  static char crlf[3] = {CR,LF,'\0'};
  int len;
  char msg[132];

  /*----------------------------------------------------------------
  | Check if we have to output a header line. This has to be done
  | if 'PAGELEN' data lines have been send.
  -----------------------------------------------------------------*/
  if (line == PAGELEN)
  {
    sprintf(msg,"*H-ID-DATE---TIME--E-H.VL-DIR--V.VL-S.VL--TEMP-DEPT-COND_SALN-DENS---TILT XY");
    strcat(msg,crlf);
    len = strlen(msg);
    strcpy(sys_ucm40.out_msg,msg);
    msg_output(sys_ucm40.out_msg);
    line = 0;         /* Reset line counter */
  }

  sprintf(sys_ucm40.out_msg,"*_%2d %02d%02d%02d %02d%02d%02d %1d %04.0lf %03.0lf %5.0f %4.0f  %4.0f %4.0f %4.0f %4.0f %6.0lf %3.0f %3.0f%s",
               id,
               tmnow->tm_year, tmnow->tm_mon, tmnow->tm_mday,
               tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec,
               e, h_vl, ship.heading, 0.0, s_vl, temp, dept,
               cond, saln, dens, tilt_x, tilt_y, crlf);

  line++;        /* Increment line counter (after output) */
  update_ucm40_params();

}

/*-------------------------------------------------------------------
|   FUNCTION: update_ucm40_params()
|    PURPOSE: Will update UCM40 parameters between user defined
|             boundaries.
| DESRIPTION: All the 'd_' values are given in value per second.
|             So take care of the actual update time 'dt'.
|    RETURNS: Nothing
|    VERSION: 901113 V0.1
---------------------------------------------------------------------*/
static void update_ucm40_params()
{
  double dt;

  dt = sys_ucm40.dt;

  if (h_vl >= h_vl_max || h_vl <= h_vl_min)
    d_h_vl = -d_h_vl;
  h_vl += (d_h_vl * dt);

  if (s_vl >= s_vl_max || s_vl <= s_vl_min)
    d_s_vl = -d_s_vl;
  s_vl += (d_s_vl * dt);

  if (temp >= temp_max || temp <= temp_min)
    d_temp = -d_temp;
  temp += (d_temp * dt);

  if (dept >= dept_max || dept <= dept_min)
    d_dept = -d_dept;
  dept += (d_dept * dt);

  if (saln >= saln_max || saln <= saln_min)
    d_saln = -d_saln;
  saln += (d_saln * dt);

}

/*-------------------------------------------------------------------
|   FUNCTION: ucm40_key_action(int key)
|    PURPOSE: Take action on operator keyboard input
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901115 V0.1
---------------------------------------------------------------------*/
void ucm40_key_action(int key)
{
  switch (key)
  {
    /*-----------------------------------------------------
    | On G(o) we simulate the UCM40 to start from surface
    ------------------------------------------------------*/
    case 'G':
      dept = 0.001;          /* Give it a small offset to start */
      break;

    default:
      break;
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: change_ucm40_params()
|    PURPOSE: Change internal UCM40 parameters
| DESRIPTION: Lets the user change the minimum and maximum values
|             of parameters such as vertical velocity, temperature
|             and salinity
|    RETURNS: Nothing
|    VERSION: 901113 V0.1
---------------------------------------------------------------------*/
void change_ucm40_params()
{
  #define NO_ITEMS 17

  int choise;
  MENU menu[NO_ITEMS];

  setup_menu(menu, 0,  "1 Horz. veloc. min", 0, NULL, "UCM40", &h_vl_min, "%4.0f", 0);
  setup_menu(menu, 1,  "2              max", 0, NULL, "UCM40", &h_vl_max, "%4.0f", 0);
  setup_menu(menu, 2,  "3               dv", 0, NULL, "UCM40", &d_h_vl,   "%4.1f", 0);
  setup_menu(menu, 3,  "4 Sound veloc. min", 0, NULL, "UCM40", &s_vl_min, "%4.0f", 0);
  setup_menu(menu, 4,  "5              max", 0, NULL, "UCM40", &s_vl_max, "%4.0f", 0);
  setup_menu(menu, 5,  "6               ds", 0, NULL, "UCM40", &d_s_vl,   "%4.1f", 0);
  setup_menu(menu, 6,  "7 Temperatur   min", 0, NULL, "UCM40", &temp_min, "%4.0f", 0);
  setup_menu(menu, 7,  "8              max", 0, NULL, "UCM40", &temp_max, "%4.0f", 0);
  setup_menu(menu, 8,  "9               dt", 0, NULL, "UCM40", &d_temp,   "%4.1f", 0);
  setup_menu(menu, 9,  "A Depth        min", 0, NULL, "UCM40", &dept_min, "%4.0f", 0);
  setup_menu(menu, 10, "B              max", 0, NULL, "UCM40", &dept_max, "%4.0f", 0);
  setup_menu(menu, 11, "C               dd", 0, NULL, "UCM40", &d_dept,   "%4.1f", 0);
  setup_menu(menu, 12, "D Salinity     min", 0, NULL, "UCM40", &saln_min, "%4.0f", 0);
  setup_menu(menu, 13, "E              max", 0, NULL, "UCM40", &saln_max, "%4.0f", 0);
  setup_menu(menu, 14, "F               ds", 0, NULL, "UCM40", &d_saln,   "%4.1f", 0);
  setup_menu(menu, 15, "                  ", 0, NULL, "UCM40", NULL,      NULL,    0);
  setup_menu(menu, 16, "Q Quit            ", 0, NULL, "UCM40", NULL,      NULL,    0);

  do
  {
    choise = popup(NO_ITEMS,menu,10,10);
  }
  while (choise != ESC && choise != 'Q');

  #undef NO_ITEMS

}

#endif
