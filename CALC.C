
/*********************************************************************
*                                                                    *
*    NAVCALC.C                                                       *
*    890603 V1.2                                                     *
*                                                                    *
*    calculations for NAVSIM navigation simulator                    *
*                                                                    *
*********************************************************************/

#include <stdio.h>
#include <math.h>

#ifdef __MSC__
#include <memory.h>
#else
#include <mem.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <front9.h>
#include <time.h>
#include <window.h>
#include "navsim.h"

/*-------------------
| Local variables
--------------------*/
static double utm_qp;
static double old_x,
              old_y,
              old_heading;

/*-------------------------------------------------------------------
|   FUNCTION: dms_to_deg()
|    PURPOSE: Convert deg/min/sec to degrees
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901125 V0.1
---------------------------------------------------------------------*/
void dms_to_deg(int deg, int mm, double ss, double *d)
{
  *d = (deg + mm/60.0 + ss/3600.0);
}

/*-------------------------------------------------------------------
|   FUNCTION: deg_to_dms()
|    PURPOSE: Convert degrees to deg/min/sec
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901125 V0.1
---------------------------------------------------------------------*/
void deg_to_dms(double r, int *deg, int *mm, double *ss)
{
  *deg = (int) floor(r);
  r = 60.0 * (r - *deg);
  *mm = (int) floor(r);
  *ss = 60.0 * (r - *mm);
  if (fabs(*ss) >= 60.0)
  {
    *mm = *mm + 1;
    *ss = 0.0;
    if (*mm == 60)
    {
      *deg = *deg + 1;
      *mm = 0;
    }
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: dms_to_rad()
|    PURPOSE: Convert deg/min/sec to radians
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: V0.1
---------------------------------------------------------------------*/
void dms_to_rad(int deg, int mm, double ss, double *rad)
{
  *rad = (deg + mm/60.0 + ss/3600.0) * DEG_TO_RAD;
}

/*-------------------------------------------------------------------
|   FUNCTION: rad_to_dms()
|    PURPOSE: Convert radians to deg/min/sec
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 900328 V0.1
---------------------------------------------------------------------*/
void rad_to_dms(double r, int *deg, int *mm, double *ss)
{

  r = fabs(r) * RAD_TO_DEG;
  *deg = (int) floor(r);
  r = 60.0 * (r - *deg);
  *mm = (int) floor(r);
  *ss = 60.0 * (r - *mm);
  if (fabs(*ss) >= 60.0)
  {
    *mm = *mm + 1;
    *ss = 0.0;
    if (*mm == 60)
    {
      *deg = *deg + 1;
      *mm = 0;
    }
  }
}

/*------------------------------------------------------------------------
|   FUNCTION: calc_geog_const()
|    PURPOSE: Calculate geografic constants
| DESRIPTION: Calculate constants, used in conversion from projection to lat/long
|    RETURNS: Nothing
|    VERSION: 900328 V0.1
-------------------------------------------------------------------------*/
void calc_geog_const()
{
  dash();

  utm_qp = SQR(1.0 - spheroid.e2 * SQR(sin(projection.lat0 * DEG_TO_RAD))) /
           (6.0 * projection.k0 * SQR(spheroid.a) * (1.0-spheroid.e2));
}

/*-------------------------------------------------------------------
|   FUNCTION: calc_sf()
|    PURPOSE: Calculate scale factor
| DESRIPTION: Calculate scale factor and true distance.
|             Derivated from PDS1000 routine.
|    RETURNS: The calculated scale factor
|    VERSION: 900217 V0.1
---------------------------------------------------------------------*/
double calc_sf(double e1, double n1, double e2, double n2, double *trd, double *sf)
{
  double dx,dy;
  double xst_tem, xrf_tem;
  double s;

  xst_tem = e1 - projection.e0;
  xrf_tem = e2 - projection.e0;

  s  = SQR(xst_tem + xrf_tem);
  dx = SQR(xst_tem - xrf_tem);
  dy = SQR(n1 - n2);
  s = s * (s - 6.0*dx + 4.0*dy);
  s = 3.0 / 35.0 * utm_qp * s;
  s = projection.k0 +
      utm_qp * (SQR(xst_tem) + xst_tem * xrf_tem + SQR(xrf_tem) + s);
  *trd = sqrt(dx+dy)/s;
  *sf = s;

  return(*sf);
}

/*-------------------------------------------------------------------
|   FUNCTION: check_bear()
|    PURPOSE: Check bearing
| DESRIPTION: Checks if bearing between 0 and 360 degrees. Adjusts
|             bearing if needed.
|    RETURNS: Nothing
|    VERSION: V0.1
---------------------------------------------------------------------*/
void check_bear(double *b)
{
  if (*b >= 360.0)	*b = *b - 360.0;
  if (*b  < 0.0)	*b = *b + 360.0;
}

/*-------------------------------------------------------------------
|   FUNCTION: new_position()
|    PURPOSE: Predic new position
| DESRIPTION: Calculates new ship heading (degrees) and position (X,Y).
|             which will be valid after a interval of dt seconds.
|             mode = 0: Don't save new ship's position
|             mode = 1: Update ship's position
|    RETURNS: Nothing
|    VERSION: 910903 V0.4 - Prediction of a new position
---------------------------------------------------------------------*/
void new_position(int mode, double *x, double *y, double *heading, double dt);
void new_position(int mode, double *x, double *y, double *heading, double dt)
{
  double dist, dx, dy;

  /*-----------------------------------------
    If in profile mode, check if EOL reached.
    If so, reset ship variables for next line
  --------------------------------------------*/
  if (profile_mode == ON)
  {
    check_profile(*x,*y,&profile);
  }

  /*------------------------------------
    Save ships coordinates and heading
  -------------------------------------*/
  if (mode == 1)
  {
    old_x = *x;
    old_y = *y;
    old_heading = *heading;
    calc_depths(*x,*y,&depth1,&depth2,dt);
  }

  /*------------------------------------------
  | Calculate new heading
  | 'helm' is expressed in degrees per second
  -------------------------------------------*/
  *heading = old_heading + (ship.helm * dt);
  check_bear(heading);

  /*-------------------
  | Calculate new x/y
  --------------------*/
  dist = ship.speed * dt * KNOTS_TO_MS;
  dx = dist * sin(*heading*DEG_TO_RAD);
  dy = dist * cos(*heading*DEG_TO_RAD);

  /*-------------
    Add noise
  -------------*/
  /*
  x_noise = generate_noise(max_noise);
  y_noise = generate_noise(max_noise);
  ship.x = old_x + dx + x_noise;
  ship.y = old_y + dy + y_noise;
  */

  *x = old_x + dx;
  *y = old_y + dy;

}

/*-------------------------------------------------------------------
|   FUNCTION: calc_position()
|    PURPOSE: Calculate position
| DESRIPTION: Calculates new ship heading (degrees) and position (X,Y).
|             which will be valid after a interval of dt seconds.
|             mode = 0: Don't save new ship's position
|             mode = 1: Update ship's position
|    RETURNS: Nothing
|    VERSION: 910903 V0.4 - Prediction of a new position
---------------------------------------------------------------------*/
void calc_position(int mode, double dt)
{
  /*----------------------
    Predict new position
  -----------------------*/
  new_position(mode,&ship.x,&ship.y,&ship.heading,dt);

  /*------------------------------------------
  | Calculate lat/long of ship if in UTM mode
  -------------------------------------------*/
  utmgeo(ship.x,ship.y,&ship.lat,&ship.lon);
  deg_to_dms(ship.lat,&ship.lat_deg,&ship.lat_mm,&ship.lat_ss);
  deg_to_dms(ship.lon,&ship.lon_deg,&ship.lon_mm,&ship.lon_ss);

  /*-----------------------------
  | Perform height calculations
  ------------------------------*/
  ;

}

/*-------------------------------------------------------------------
|   FUNCTION: calc_range()
|    PURPOSE: Calculate range
| DESRIPTION: Calculate range from station 'i' to the ship
|    RETURNS: The calculated range
|    VERSION: V0.1
---------------------------------------------------------------------*/
double calc_range(int i)
{
  double r,sf;

  sf = calc_sf(station[i].x,station[i].y,ship.x,ship.y,&r,&sf);
  return(r);
}

/*-------------------------------------------------------------------
|   FUNCTION: pround()
|    PURPOSE: Power round
| DESRIPTION: Rounds an double to a specified power of 10
|    RETURNS: The rounded double
|    VERSION: V0.1
---------------------------------------------------------------------*/
double pround( double number, int power)
{
  double mul;
  int	n;

  mul = 1.0;
  for (n=0 ; n<abs(power) ; n++)
    mul = mul * 10;
  if (power < 0)
    mul = 1 / mul;
  return(floor(number/mul + 0.5) * mul);
}

/*-----------------------------------------------------------------------
|   FUNCTION: calc_lr_coeff()
|    PURPOSE: Calculates left/right coefficients
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 900331 V0.2 - ...
|             911112 V0.3 - Generalised this function by using a pntr
|                           to a L/R record and input of line coordinates
-------------------------------------------------------------------------*/
void calc_lr_coeff(LEFT_RIGHT *lr,     /* Pointer to L/R record */
                   double xstart,      /* X start of line       */
                   double ystart,      /* Y start of line       */
                   double xend,        /* X end of line         */
                   double yend)        /* Y end of line         */
{
  lr->distance = sqrt( SQR(yend-ystart) + SQR(xend - xstart) );
  if (lr->distance != 0.0)
  {
    lr->a = (yend - ystart) / lr->distance;
    lr->b = (xstart - xend) / lr->distance;
    lr->c = lr->a * xstart - lr->b * ystart;
    lr->d = lr->b * xstart - lr->a * ystart;
  }
}

/*------------------------------------------------------------------------
|   FUNCTION: calc_leftright()
|    PURPOSE: Calculate left/right data:
|             # distance to go to waypoint along defined line
|             # distance travelled along line
|             # distance off line
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 900307 V0.1 - Initial version
|             911112 V0.2 - Generalised this function by using a pntr
|                           to a L/R record and input of actual coordinates
|             911113 V0.3 - Removed fabs() form 'distance to go' calculation
---------------------------------------------------------------------------*/
void calc_leftright(LEFT_RIGHT *lr, double x, double y)
{
  lr->rtlft  = lr->a * x + lr->b * y + lr->c;
  lr->travel = lr->a * y - lr->b * x + lr->d;
  lr->togo   = lr->distance - lr->travel;           /* Was with fabs() */
}

/*-------------------------------------------------------------------
|   FUNCTION: calc_bearing(int i)
|    PURPOSE: Calculates bearing of station 'i' towards the ship
| DESRIPTION: -
|    RETURNS: Calculated bearing in degrees.
|    VERSION: V0.1
---------------------------------------------------------------------*/
double calc_bearing(int i)
{
  double de,dn;  /* differences in northing and easting */
  double b;

  de = station[i].x - ship.x;
  dn = station[i].y - ship.y;

  if (de == 0.0 && dn == 0.0)
    b = 0.0;
  else
    b = atan2(de,dn);            /* between -pi and pi */

  b *= RAD_TO_DEG;               /* Convert to degrees */

  return(b);
}

/*-------------------------------------------------------------------
|   FUNCTION: find_master()
|    PURPOSE: find master station from Hyperfix pattern code
| DESRIPTION: -
|    RETURNS: Number of master station, 0 if none is available
|    VERSION: V0.1
---------------------------------------------------------------------*/
int find_master(int patcode)
{
  int master;         /* Patcode 24 --> master = 4, slave = 2 */

   master = patcode % 10;
   if (master)        /* && station[master].code) */
     return(master);
   else
     return(0);
}

/*-------------------------------------------------------------------
|   FUNCTION: find_slave()
|    PURPOSE: find slave station from Hyperfix pattern code
| DESRIPTION: -
|    RETURNS: Number of slave station, 0 if none is available
|    VERSION: V0.1
---------------------------------------------------------------------*/
int find_slave(int patcode)
{
  int slave;    /* patcode 24 ---> slave = 2, master = 4 */

  slave = patcode / 10;
  if (slave) /* && station[slave].code) */
    return(slave);
  else
    return(0);
}

/*-------------------------------------------------------------------
|   FUNCTION: calc_baselines()
|    PURPOSE: Calculate baselines
| DESRIPTION: Calculate baselines between hyperbolic stations
|    RETURNS: Nothing
|    VERSION: V0.1
---------------------------------------------------------------------*/
void calc_baselines()
{
  int	 i;
  double sf;
  int	 master,slave;
  int	 pattern;

  switch (nav_system->sys)
  {
    case HYPERFIX :
      for (i=0 ; i<nav_system->max_patterns ; i++)
      {
        pattern = nav_system->p[i];
        if (pattern)
        {
          master = find_master(pattern);
          slave  = find_slave(pattern);

          if (master && slave)
          {
             sf = calc_sf(station[master-1].x,station[master-1].y,
                    station[slave-1].x,station[slave-1].y,&baseline[i],&sf);
          }
        }
      }
      break;

    default:
      break;

  } /* switch nav_system.sys of */
}

/*-------------------------------------------------------------------
|   FUNCTION: find_station()
|    PURPOSE: Find station number for given code
| DESRIPTION: -
|    RETURNS: nothing
|    HISTORY: 900501 V0.1 - Initial version
---------------------------------------------------------------------*/
int find_station(int code)
{
  int i;

  for (i=0 ; i<MAX_STATIONS ; i++)
  {
    if (station[i].code == code && station[i].name[0] != '\0')
      return(i);
  }
  return(-1);
}


/*-------------------------------------------------------------------
|   FUNCTION: calc_patterns()
|    PURPOSE: Calculate patterns
| DESRIPTION: Calculates ranges, bearing, lane reading etc. from
|             ship to selected stations
|    RETURNS: Nothing
|    VERSION: V0.2
---------------------------------------------------------------------*/
void calc_patterns()
{
  int i,j;

  switch (nav_system->mode)
  {
    case RANGE_RANGE:
      for (i=0 ; i<nav_system->max_patterns; i++)
      {
        j = find_station(nav_system->p[i]);
        if (j<0)       /* If not found, return */
          return;
        station[j].range = ABS(calc_range(j) + station[j].C_O);

        /*--------------------------
          Perform check on ranges
        ---------------------------*/
        if (nav_system->sys == TRISPONDER)
        {
          if (station[j].range > 9999.9)
          {
            station[j].range = 9999.9;
            disp_status("Trisponder station %02d out of range",nav_system->p[i]);
          }
        }

      }
      break;

    case RANGE_BEARING :
      j = find_station(nav_system->p[0]);
      if (j >= 0)
      {
        station[j].range = ABS(calc_range(j) + station[j].C_O);
        ship.bearing     = calc_bearing(j);
        ship.azimuth     = ship.bearing + 180.0;
        check_bear(&ship.bearing);    /* Check if in range 0...360 */
        check_bear(&ship.azimuth);    /* Idem dito		 */
      }
      if (nav_system->sys == ARTEMIS)
      {
        if (station[j].range > 30000.0)
        {
          disp_status("Artemis station %02d out of range",nav_system->p[0]);
        }
      }
      break;

    case HYPERBOLIC :
      #if (HYPERFIX || PULSE8)
        ; /*
             Appropiate calculations are done in message building funcs
             of Hyper-Fix and Pulse 8
          */
      #endif
      break;

    default:
      break;

    } /* switch nav_system->sys of */
}

/*-------------------------------------------------------------------
|   FUNCTION: generate_noise()
|    PURPOSE: Random noise generator
| DESRIPTION: This function will generate a random double number
|             within the given limits  -max_noise ... +max_noise
|    RETURNS: Generated random double number
|    VERSION: 901125 V0.2
---------------------------------------------------------------------*/
double generate_noise(double max_noise)
{
  #define MAXNOISE INT_MAX
  double n;
  int    i;

  #ifdef TURBOC
  n = (double) random(MAXNOISE);   /*  [0 ... MAXNOISE>           */
  n /= MAXNOISE;
  #else
  i = rand();                      /*  [0.... 32767               */
  n = (double) i / 32767;
  #endif

  n /= MAXNOISE;                   /*  [0 ... 1>                  */
  n -= 0.5;                        /*  [-0.5 ... 0.5>             */
  n *= 2.0;                        /*  [-1.0 ... 1.0>             */
  n *= max_noise;                  /*  [-max_noise ... max_noise> */
  return(n);
}

/*-------------------------------------------------------------------
|   FUNCTION: Gaussian_noise()
|    PURPOSE: Generate a double gaussian distributed random number
| DESRIPTION: Token from RACAL's 'navsim' by J. Finch
|    RETURNS: Generated random number
|    VERSION: 901125 V0.1
---------------------------------------------------------------------*/
double gaussian_noise(void);
double gaussian_noise()
{
  double g1,g2,g3,g4,noys;
  int neg_sign = FALSE;

  #ifdef TURBOC
  g1 = (double) random(INT_MAX)/INT_MAX;
  #else
  g1 = (double) rand() / 32767;
  #endif

  if (g1 > 0.5)
  {
    neg_sign = TRUE;
    g1 = 1.0 - g1;
  }

  g2 = SQR(log(1.0/(g1*g1)));
  g3 = 2.515517 + g2*(0.802853 + 0.010328*g2);
  g4 = 1.0 + g2 * (1.432788 + g2 * (0.189269 + g2 * 0.001308));
  noys = (g2-g3/g4);
  if (neg_sign)
    noys = -noys;

  return(noys);
}

