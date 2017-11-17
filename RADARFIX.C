#include "navsim.h"

#if RADARFIX

#include <stdio.h>
#include <string.h>
#include <math.h>

/*--------------------------------
| Prototype's of local functions
---------------------------------*/
static void radarfix_b_msg(void);
static void radarfix_c_msg(void);

NAV_SYSTEM sys_radarfix =
{
  "Radar-Fix",                /* name of navigation system          */
  RADARFIX,                   /* system number for case statements  */
  1,                          /* number of frequencies              */
  {9.0e9},                    /* operation frequencies              */
  {299.65e6},                 /* propagation velocity               */
  RANGE_BEARING,              /* mode                               */
  1,                          /* maximum number of patterns         */
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns      */
  2,                          /* Number of formats                  */
  { "Message B",              /* list of format names               */
    "Message C",
  },
  { radarfix_b_msg,           /* List of output functions           */
    radarfix_c_msg,
  },
  1,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0,0.0},                  /* Standard deviations                */
  NULL,                       /* Initialization function            */
  radarfix_b_msg,             /* Output function                    */
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

static double erm = 0.3;
static double d_erm = 0.1;

/*-------------------------------------------------------------------
|   FUNCTION: radarfix_b_msg()
|    PURPOSE: Generate RADARFIX type B message
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910426 V0.1 - Initial version
---------------------------------------------------------------------*/
static void radarfix_b_msg()
{
  /*--------------------
    Calculate new ERM
  ---------------------*/
  d_erm -= d_erm;
  erm =  0.3 + d_erm;

  sprintf(sys_radarfix.out_msg,"B %06.2f deg %11.2f%11.2f%5.0f%6.1f%6.1f%6.2f%6.1f%c%c",
    ship.heading, ship.x, ship.y, ship.helm * 60.0,
    ship.speed*sin(ship.heading * DEG_TO_RAD),
    ship.speed*cos(ship.heading * DEG_TO_RAD),
    sys_radarfix.dt, erm, CR, LF);
}

/*-------------------------------------------------------------------
|   FUNCTION: radarfix_c_msg()
|    PURPOSE: Generate RADARFIX type C message
| DESRIPTION: -
|    RETURNS: Nothing
|    HISTORY: 910426 V0.1 - Initial version
---------------------------------------------------------------------*/
static void radarfix_c_msg()
{
  /*--------------------
    Calculate new ERM
  ---------------------*/
  d_erm -= d_erm;
  erm = 0.3 + d_erm;

  sprintf(sys_radarfix.out_msg,"C%11.2f%11.2f%6.1f%c%c",
    ship.x,ship.y,erm,CR,LF);

}

#endif
