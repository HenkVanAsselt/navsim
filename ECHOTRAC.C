/*********************************************************************
*                                                                    *
*   ECHOTRAC.C                                                       *
*                                                                    *
*   Sourcefile for echotrac message generation                       *
*                                                                    *
*   901115 V0.1                                                      *
*                                                                    *
*********************************************************************/

#include "navsim.h"

#if ECHOTRAC

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

static void echotrac_key_action(int key);
static void echo_default(void);
static void echo_low(void);
static void echo_high(void);
static void echo_high_low(void);

NAV_SYSTEM sys_echotrac =
{
  "Echotrac",   	          /* name of navigation system	 		*/
  ECHOTRAC, 		          /* system number for case statements 	*/
  0,                          /* number of frequencies              */
  {0.0},                      /* operation frequencies		  		*/
  {1.0},                      /* propagation velocity		  		*/
  NONE_STATION,		          /* mode            		 			*/
  0,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  4,                          /* Number of formats                  */
  {"Default",                 /* list of format names               */
   "Low",
   "High",
   "High/Low",
  },
  {
    echo_default,
    echo_low,                 /* List of output functions           */
    echo_high,
    echo_high_low,
  },
  1,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0},                      /* Standard deviations                */
  NULL,                       /* Initialization function            */
  echo_default,               /* Output function                    */
  NULL,                       /* Input function                     */
  echotrac_key_action,        /* Special keyboard functions         */
  -1,                         /* Channel                            */
  -1,                         /* Slot1                              */
  9600,                       /* Baudrate                           */
  8,                          /* Databits                           */
  1,                          /* Stopbits                           */
  'N',                        /* Parity                             */
  'N',                        /* Handshake                          */
  CR,                         /* Terminator                         */
  120,                        /* Terminal count                     */
};


/*------------------
| Local variables
------------------*/
static int fix    = ' ';          /* Fix is ' ' or 'F'     */
static int alarm  = ' ';          /* Alarm is ' ' or 'E'   */
static double factor = 10.0;      /* Depth factor (cm/dm)  */
static char *user_id = "ET";      /* dm --> user_id = "ET" */

/*-------------------------------------------------------------------
|   FUNCTION: echo_default()
|    PURPOSE: Generate ECHOTRAC DEFAULT message
| DESRIPTION: Generate message and always reset fix character
|    RETURNS: Nothing
|    VERSION: 910410 V0.2
---------------------------------------------------------------------*/
static void echo_default()
{
  sprintf(sys_echotrac.out_msg,"%c%s%c%6.0lf%c",fix,user_id,alarm,depth1*factor,CR);
  fix = ' ';
}

/*-------------------------------------------------------------------
|   FUNCTION: echo_low(char *msg)
|    PURPOSE: Generate ECHOTRAC LOW message
| DESRIPTION: Generate message and always reset fix character
|    RETURNS: Nothing
|    VERSION: 910410 V0.2
---------------------------------------------------------------------*/
static void echo_low()
{
  sprintf(sys_echotrac.out_msg,"%c%s%cL %5.0lf%c",
    fix,user_id,alarm,depth1*factor,CR);
  fix = ' ';
}

/*-------------------------------------------------------------------
|   FUNCTION: echo_high()
|    PURPOSE: Generate ECHOTRAC HIGH message
| DESRIPTION: Generate message and always reset fix character
|    RETURNS: Nothing
|    VERSION: 910410 V0.2
---------------------------------------------------------------------*/
static void echo_high()
{
  sprintf(sys_echotrac.out_msg,"%c%s%cH %5.0lf%c",
    fix,user_id,alarm,depth2*factor,CR);
  fix = ' ';
}

/*-------------------------------------------------------------------
|   FUNCTION: echo_high_low(char *msg)
|    PURPOSE: Generate ECHOTRAC HIGH and LOW message
| DESRIPTION: Generate message and always reset fix character
|    RETURNS: Nothing
|    VERSION: 910410 V0.2
---------------------------------------------------------------------*/
static void echo_high_low()
{
  sprintf(sys_echotrac.out_msg,"%c%s%cB %5.0lf %5.0lf%c",
    fix,user_id,alarm,depth1*factor,depth2*factor,CR);
  fix = ' ';
}

/*-------------------------------------------------------------------
|   FUNCTION: echotrac_key_action(int key)
|    PURPOSE: Take action on operator keyboard input
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901115 V0.1
---------------------------------------------------------------------*/
static void echotrac_key_action(int key)
{
  switch (key)
  {
    /*-------------------------------------------------------
    | Simulate MANUAL FIX. Fix character will be reset after
    | each messag output
    --------------------------------------------------------*/
    case '*':
      fix = 'F';
      break;

    /*-------------------------
    | Toggle ALARM character
    -------------------------*/
    case 'A':
      if (alarm == ' ')
        alarm = 'E';
      else
        alarm = ' ';
      break;

    /*--------------
    | Change depth
    ----------------*/
    case '+':
      depth1 += 1.0;
      depth2 += 1.0;
      break;
    case '-':
      depth1 -= 1.0;
      depth2 -= 1.0;
      break;

    /*------------------------------------
    | Toggle depth conversion factor.
    | factor = 10  --> output in dm ("ET")
    | factor = 100 --> output in cm ("et")
    -------------------------------------*/
    case 'c':
    case 'C':
      if (fabs(factor) < 10.5)
      {
        factor  = 100.0;
        user_id = "et";
      }
      else
      {
        factor  = 10.0;
        user_id = "ET";
      }
      break;
  }

}

#endif
