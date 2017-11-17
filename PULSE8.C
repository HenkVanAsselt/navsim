#include "navsim.h"

#if PULSE8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

static void mk4_msg(void);
static void mk6_msg(void);
static void mk7_msg(void);
static void mon_msg(void);

NAV_SYSTEM sys_pulse8 =
{
  "Pulse8",   	              /* name of navigation system	 		*/
  PULSE8,                     /* system number for case statements 	*/
  1,                          /* number of frequencies              */
  { 100e3,                    /* operation frequencies		  		*/
  },
  { 299.65e6,                 /* propagation velocity		  		*/
  },
  HYPERBOLIC, 		          /* mode            		 			*/
  7,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patterns system is using */
  {0},                        /* Selected stations or patterns	 	*/
  4,                          /* Number of formats                  */
  { "MK 4",
    "MK 6",
    "MK 7",
    "MON",
    ""
  },                     /* list of format names               */
  {
    mk4_msg,                  /* List of output functions           */
    mk6_msg,
    mk7_msg,
    mon_msg,
  },
  0,                          /* Actual format                      */
  1,                          /* Number of standard deviations      */
  {0.02},                     /* Standard deviations                */
  NULL,                       /* Initialization function            */
  mk7_msg,                    /* Output function                    */
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

/*-------------------------------------------------------------------
|   FUNCTION: mk4_msg()
|    PURPOSE: Generate Pulse-8 MK IV message
| DESRIPTION: -
|    RETURNS: nothing
|    HISTORY: 910802 V0.1 - Initial version with fixed string
---------------------------------------------------------------------*/
static void mk4_msg()
{
  sprintf(sys_pulse8.out_msg,
    "1220993 2481185 3673027 7570234 7777111 234000%c%c",CR,LF);
}

/*-------------------------------------------------------------------
|   FUNCTION: mk6_msg()
|    PURPOSE: Generate Pulse-8 MK VI message
| DESRIPTION: -
|    RETURNS: nothing
|    HISTORY: 910802 V0.1 - Initial version with fixed string
---------------------------------------------------------------------*/
static void mk6_msg()
{
   sprintf(sys_pulse8.out_msg,
     "0002804911733321H7570   7-1012209957-1424811797-0536730227+0148154597+0223342807+0211424377+02  0%c%c",CR,LF);
}

/*-------------------------------------------------------------------
|   FUNCTION: mk7_msg()
|    PURPOSE: Generate Pulse-8 MK VII message
| DESRIPTION: -
|    RETURNS: nothing
|    HISTORY: 910802 V0.1 - Initial version with fixed string
---------------------------------------------------------------------*/
static void mk7_msg()
{
  sprintf(sys_pulse8.out_msg,
    "0002804911728307570000000000000112770812122100811377080424811931147708+136730351157708+348154711357704+3233427814577+1+311424361347704+11191842%c%c",CR,LF);
}

/*-------------------------------------------------------------------
|   FUNCTION: mon_msg()
|    PURPOSE: Generate Pulse-8 Monitor message
| DESRIPTION: -
|    RETURNS: nothing
|    HISTORY: 910802 V0.1 - Initial version with fixed string
---------------------------------------------------------------------*/
static void mon_msg()
{
  sprintf(sys_pulse8.out_msg,
    "1127707113746122101031137707033744248119261147707+23739367302921157707+33735481546801357703+344352334275414577+2+33935114243911347703+2443911918365%c%c",CR,LF);
}

#endif
