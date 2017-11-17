#include "navsim.h"

#if HYPERFIX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/*----------
  Defines
----------*/
#define F1 0
#define F2 1

/*-------------------
  Local parameters
-------------------*/
static double lane[9][2];

/*-------------------------------
  Prototypes of local functions
--------------------------------*/
static void hyperfix_msg(void);
static void calc_hyperfix_patts(void);

/*--------------------
  System definition
--------------------*/
NAV_SYSTEM sys_hyperfix =
{
  "Hyper-Fix",                /* name of navigation system           */
  HYPERFIX,                   /* system number for case statements   */
  2,                          /* number of frequencies               */
  { 2149.35e3,                /* operation frequencies /Thames chain */
    1920.50e3,
  },
  { 299.65e6,                 /* propagation velocity               */
    299.65e6,
  },
  HYPERBOLIC,                 /* mode                               */
  3,                          /* maximum number of patterns         */
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns      */
  1,                          /* Number of formats                  */
  { "Default" },              /* list of format names               */
  { hyperfix_msg, },          /* List of output functions           */
  0,                          /* Actual format                      */
  1,                          /* Number of standard deviations      */
  {0.02},                     /* Standard deviations                */
  NULL,                       /* Initialization function            */
  hyperfix_msg,               /* Output function                    */
  NULL,                       /* Input function                     */
  NULL,                       /* Special keyboard functions         */
  1,                          /* Channel                            */
  1,                          /* Slot1                              */
  4800,                       /* Baudrate                           */
  7,                          /* Databits                           */
  1,                          /* Stopbits                           */
  'E',                        /* Parity                             */
  'N',                        /* Handshake                          */
  LF,                         /* Terminator                         */
  120,                        /* Terminal count                     */
};

/*-------------------------------------------------------------------
|   FUNCTION: hyperfix_msg()
|    PURPOSE: Hyperfix message
| DESRIPTION: This function generates a hyperfix message. The ambigity
|             part of this message will not be simulated and contains
|             all zero's.
|    RETURNS: Nothing
|    VERSION: 901108 V0.3
---------------------------------------------------------------------*/
static void hyperfix_msg()
{
  static int counter = 0;
  int  i,pattern;
  long l1,l2;
  char ch;
  char tmp[40];
  char msg[132];

  /*--------------------------------
    Calculate the hyperfix patterns
  ----------------------------------*/
  calc_hyperfix_patts();

  /*-------------------------------------------------------
    For a description of the serial message, see HYPERFIX
    Technical Manual, STM 1215 pp 5.154 - 4.158
  --------------------------------------------------------*/
  strcpy(msg,"");  /* clear output message */
  {
    for (i=0 ; i<3 ; i++)
    {
      /*------------------------------------------
        pattern pair, f1 pattern and f2 pattern
      -------------------------------------------*/
      pattern = nav_system->p[i];
      if (pattern)
      {
        l1 = (long) lane_reading(i,F1);
        l2 = (long) lane_reading(i,F2);
        sprintf(tmp,"%2d %06ld %06ld ",pattern,l1,l2);
        strcat(msg,tmp);
      }
      else
        strcat(msg,"00 000000 000000 ");
    }

    /*----------------------------------------------------------
      ambigity resolution (fixed data for simulation purposes)
    -----------------------------------------------------------*/
    strcat(msg,"0000 0000 0000 0000 0000 0000 ");

    /*--------------------------------------------------------
      Calculate status codes (lock flag indicators)
      See hyperfix technical manual STM 1215 page 4.157.
      We assume a lock on a frequency if there is a
      pattern pair defined and a frequency is available
      Every 8 outputs, the lockflags will be forced '111' as
      is done in Hyperfix, when Sequence Number is showed on
      it's LCD display.
    ---------------------------------------------------------*/
    counter++;
    counter = counter % 8;
    for (i=0 ; i<3 ; i++)	    /* for 3 pattern pairs */
    {
      if (!counter)
      {
        /*--------------------------
          Force lockflags to '111'
        --------------------------*/
        ch = '1';
      }
      else
      {
        if (!nav_system->p[i])
          ch = '@';             /* No lock for F1 and F2 */
        else
        {
          if ( nav_system->f[0] > 0.5)
            ch = 'E';                   /* Lock on F1 */
          if ( nav_system->f[1] > 0.5)
            ch = 'J';                   /* lock on F2 */
          if ( nav_system->f[0] > 0.5  &&
           nav_system->f[1] > 0.5 )
          ch = 'O';                     /* lock on F1 and F2 */
        }
      }
      msg[81+i] = ch;
    }

    /* Terminate the message */
    msg[84] = CR;
    msg[85] = LF;
    msg[86] = '\0';

    strcpy(sys_hyperfix.out_msg,msg);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: lane_reading(int i, int f)
|    PURPOSE: Calculate F1/F2 lane reading of pattern i
| DESRIPTION: -
|    RETURNS: Calculated lane reading
|    VERSION: 901107 V0.1
---------------------------------------------------------------------*/
long lane_reading(int i, int f)
{
  long l;

  l = (long) floor(lane[i][f] * 100 + 0.5);
  l = ABS(l);
  return(l);
}

/*-------------------------------------------------------------------
|   FUNCTION: calc_lane()
|    PURPOSE: Calculate lane
| DESRIPTION: Calculates lane reading for hyperfix from ship to
|             a master/slave pair by giving the ranges
|    RETURNS: Calculated lane
|    VERSION: V0.1
---------------------------------------------------------------------*/
double calc_lane( double range_master, double range_slave,
		 double baseline, double lambda)
{
  if (lambda != 0.0)		/* avoid division by zero */
    return((range_slave - range_master + baseline) / lambda);
  else
    return(1.0e9);
}

/*-------------------------------------------------------------------
|   FUNCTION: calc_hyperfix_patts()
|    PURPOSE: Calculate hyperfix hyperbolic patterns
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901108 V0.2
---------------------------------------------------------------------*/
static void calc_hyperfix_patts()
{
  int i;
  int pattern,master,slave;
  double rng_m,rng_s;

  calc_baselines();

  for (i=0 ; i<nav_system->max_patterns ; i++)
  {
    pattern = nav_system->p[i];
    if (pattern)
    {
      /*--- calculate ranges ---*/
      master = find_master(pattern);
      slave  = find_slave(pattern);
      rng_m  = calc_range(master-1);
      rng_s  = calc_range(slave-1);

      /*--- calculate lane's for hyper-fix ---*/
      lane[i][0] = calc_lane(rng_m,rng_s,baseline[i],(nav_system->vc[0] / nav_system->f[0]) );
      lane[i][1] = calc_lane(rng_m,rng_s,baseline[i],(nav_system->vc[1] / nav_system->f[1]) );

      /*--- Perform range checking for hyperfix ---*/
      if (lane[i][0] > 9999.99)
      {
        lane[i][0] = 9999.99;
        disp_status("Hyperfix pattern %2d / F1 > 9999.99",pattern);
      }
      if (lane[i][1] > 9999.99)
      {
        lane[i][1] = 9999.99;
        disp_status("Hyperfix pattern %2d / F2 > 9999.99",pattern);
      }
    }
  }
}

#endif
