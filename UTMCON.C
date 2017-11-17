/**********************************************************************
 File UTMCON contains all routines for geogs <--> grid conversions

 DASH            initialization
 GEOUTM          geogs --> grid
 UTMGEO          grid --> geogs

 date 16-05-1990

 Converted to C by HvA, 910815 V0.1
*************************************************************************/

/*  Define this for stand-alone testing
#define MAIN_MODULE
*/

#include <stdio.h>
#include <math.h>
#include "navsim.h"

#define SQR(a) ((a)*(a))

#define SIN1T4 0.48481377579780065e-1
#define SIN3TT 0.18992128188729535e-4
#define SIN2T8 0.11752219970342576e-2
#define SIN4TT 0.23019104470651324e-6

#define  sph_sma    spheroid.a
#define  sph_se     spheroid.e2

#define  utm_cnk    projection.k0
#define  utm_cmd    projection.lon0
#define  utm_cmx    projection.e0
#define  utm_fnt    projection.n0
#define  utm_lat    projection.lat0

double utm_grdskw = 0.0;

/*------------------
  Local variables
-------------------*/
static double ADASH,BDASH,CDASH,DDASH,EDASH,EPRIMS,Y_OFFSET,GRDSIN,GRDCOS;

static double xref = 652636.0;
static double yref = 5883591.0;

/*-------------------------
  Local prototypes
-------------------*/
static double dsign(double a, double b);
static double modgeo(double g);

/*-------------------------------------------------------------------
|   FUNCTION: dsign(double a, double b)
|    PURPOSE: Fortran dsign function
| DESRIPTION: -
|    RETURNS: Returns fabs(a) if b >= 0.0 else -fabs(a)
|    VERSION: 900504 V0.1
---------------------------------------------------------------------*/
double dsign(double a, double b)
{
  if (b >= 0.0)
    return(fabs(a));
  else
    return(-fabs(a));
}

/*-------------------------------------------------------------------
|    FUNCTION: modgeo()
|     PURPOSE: Convert input to a double number between -180.0 and +180.0
| DESCRIPTION: -
|     RETURNS: The 'modulo' of the input
|     HISTORY: 910815 V0.1
---------------------------------------------------------------------*/
double modgeo(double g)
{
  while (g < -180.0)
    g += 180.0;
  while (g > 180.0)
    g -= 180.0;
  return(g);
}

/*-------------------------------------------------------------------
|    FUNCTION: geoutm()
|     PURPOSE: Convert lat/lon to UTM coordinates
| DESCRIPTION: Adapted from PDS1000 Fortran source
|     RETURNS: nothing
|     HISTORY: 910815 V0.1
---------------------------------------------------------------------*/
void geoutm(double alat, double rlongd, double *tmx, double *tmy)

/*...................................
  alat     = latitude in degrees
             + for north
             - for south
  rlongd   = longitude in degrees
             + for east
             - for west
...................................*/

{
   double rlat;      /* ,rlong; */
   double rlongd1;
   double sinlat,coslat,cos2lt,sr,tempi,four,five,truex,one;
   double two,three,truey;

/*.......................................................................
 THE FORMULAS USED FOR COMPUTATION ARE FROM THE FOCAL 801C PROGRAM AND
 UTILIZE SOME GONIOMETRIC FORMULAS TO FACTOR OUT tempi (BELOW)
 (E.G. COS**2(T) - SIN**2(T)= COS(2*T)
..........................................................................*/


  /*--------------------------------------------------------------
    Bring the values in the valid segment ( -180.0 ... +180.0 )
  ---------------------------------------------------------------*/
  rlat = modgeo(alat);
  rlongd1 = modgeo(rlongd);

  /*-----------------------------
    Convert degrees to radians
  ------------------------------*/
  rlat = rlat * DEG_TO_RAD;
  /* rlong = rlongd1 * DEG_TO_RAD; */   /* Rlong is not used */

  /*-----------------------------------------------
    CALCULATE SIN/COS VALUES USED MORE THAN ONCE
  -----------------------------------------------*/
  sinlat = sin(rlat);
  coslat = cos(rlat);
  cos2lt = cos(2.0*rlat);

  sr = modgeo(rlongd1 - utm_cmd) * 0.36;

  /*-----------------
    CALCULATE A temp
  -------------------*/
  tempi = sph_sma * utm_cnk * coslat / sqrt(1.0 - sph_se * SQR(sinlat));

  /*---------------------------
    CALCULATE IV, V AND UTM X
  ----------------------------*/
  four = tempi * SIN1T4;
  five = SIN3TT * tempi * (cos2lt + EPRIMS * SQR(SQR(coslat)));
  truex= sr*(four + five*SQR(sr));

  /*--------------------------------
    CALCULATE I, II, III AND UTM Y
  ----------------------------------*/
  one=(ADASH*rlat-BDASH*sin(2.0*rlat)+CDASH*sin(4.0*rlat)-
       DDASH*sin(6.0*rlat)+EDASH*sin(8.0*rlat))*utm_cnk;

  two = SIN2T8 * tempi * sinlat;

  three = SIN4TT * tempi * sinlat*(2.0 + 3.0*cos2lt);

  truey = (one + SQR(sr)*(two + three*SQR(sr)))-Y_OFFSET;

  *tmx=utm_cmx+truex*GRDCOS-truey*GRDSIN;
  *tmy=utm_fnt+truex*GRDSIN+truey*GRDCOS;
}

/*-------------------------------------------------------------------
|    FUNCTION: utmgeo()
|     PURPOSE: Convert UTM X/Y coordinates into lat/lon
| DESCRIPTION: Adapted from PDS1000 Fortran sources, converted by HvA
|     RETURNS: nothing
|     HISTORY: 910815 V0.1 - Initial version
---------------------------------------------------------------------*/
void utmgeo(double tmx, double tmy, double *glat, double *glong)

/*.......................

  glat    = latude
            + north
            - south
  glong   = longitude
            + east
            - west
.........................*/

{
  double truex,argyx,yinit,yend,xinit,xend,qcon,phi,radphi;
  double sarc,compx,connu,sine,cose,temp1,temp2,sinsq;
  double cossq,tansq,seven,eigth,anine,ten;

  /*-----------------------
    correct for projection skew
  ------------------------*/
  truex=(tmx-utm_cmx)*GRDCOS+(tmy-utm_fnt)*GRDSIN;
  argyx=(tmy-utm_fnt)*GRDCOS-(tmx-utm_cmx)*GRDSIN+Y_OFFSET;

  yinit=0.0;
  yend=8881455.5;
  xinit=0.0;
  xend=80.0;
  qcon=dsign(1.0e-6*fabs(truex),truex);

  while (1)
  {
    phi=xend-(yend-argyx)*(xend-xinit)/(yend-yinit);
    radphi=phi*DEG_TO_RAD;
    sarc=ADASH*radphi-BDASH*sin(2.0*radphi)
         +CDASH*sin(4.0*radphi)-DDASH*sin(6.0*radphi)
         +EDASH*sin(8.0*radphi);

    compx=utm_cnk*sarc;

    if (fabs(compx-argyx) < 0.0001)
      break;
    xend=xinit;
    xinit=phi;
    yend=yinit;
    yinit=compx;
  }

  connu=sph_sma/sqrt(1.0-sph_se*SQR(sin(radphi)));
  sine=sin(radphi);
  cose=cos(radphi);
  temp1=206264.8408/cose;
  temp2=1.0e6/(utm_cnk*connu);
  sinsq=SQR(sin(radphi));
  cossq=SQR(cos(radphi));
  tansq=sinsq/cossq;

  seven=sine*temp1*SQR(temp2)*(1.0+EPRIMS*cossq)/2.0;

  eigth=sine*temp1*SQR(temp2)*SQR(temp2)*(5.0+3.0*tansq
        +6.0*EPRIMS*cossq-6.0*EPRIMS*sinsq
        -3.0*SQR(EPRIMS)*SQR(cossq)-9.0*SQR(EPRIMS)*cossq*sinsq)/24.0;

  anine=temp1*temp2;

  ten=temp1*SQR(temp2)*temp2*(1.0+2.0*tansq+EPRIMS*cossq)/6.0;

  *glat=phi-(SQR(qcon)*(seven-eigth*SQR(qcon)))/3600.0;

  *glong=utm_cmd+qcon*(anine-ten*SQR(qcon))/3600.0;

  *glong=modgeo(*glong);

}

/*-------------------------------------------------------------------
|    FUNCTION: dash()
|     PURPOSE: calculate constants to calculate meridional arc
| DESCRIPTION: Adapted from PDS1000 Fortran source, adapted by HvA
|     RETURNS: error found
|     HISTORY: 910815 V0.1 - Initial version
---------------------------------------------------------------------*/
int dash()
{
  double cn,cn2,cn3,cn4,cn5,rlat,one,argyx,sph_ma;
  int dash_err;

  /*------------------------------
    Check for errors in variables
  -------------------------------*/
  dash_err=0;
  if (sph_sma < 6000000.0)
  {
    fprintf(stderr,"Check semi-major axis\n");
    dash_err = -1;
    return(-1);
  }
  if (sph_se > 1.0 )
  {
    fprintf(stderr,"Check squared eccentricity\n");
    dash_err = -2;
    return(-2);
  }
  if (utm_cnk < 0.8)
  {
     fprintf(stderr,"Check scale factor\n");
     dash_err = - 3;
     return(-3);
  }

  /*----------------------------------------------
    compute the sine and cosine of the projection skew
  ------------------------------------------------*/
  GRDSIN=sin(utm_grdskw*DEG_TO_RAD);
  GRDCOS=cos(utm_grdskw*DEG_TO_RAD);

  sph_ma=sph_sma*sqrt(1.0-sph_se);
  cn=(sph_sma-sph_ma)/(sph_sma+sph_ma);
  cn2=cn*cn;
  cn3=cn2*cn;
  cn4=cn3*cn;
  cn5=cn4*cn;

  ADASH=sph_sma*(1.0-cn+5.0/4.0*(cn2-cn3)+81.0/64.0*(cn4-cn5));
  BDASH=3.0/2.0*sph_sma*(cn-cn2+7.0/8.0*(cn3-cn4)+55.0/64.0*cn5);

  CDASH=15.0/16.0*sph_sma*(cn2-cn3+3.0/4.0*(cn4-cn5));
  DDASH=35.0/48.0*sph_sma*(cn3-cn4+11.0/16.0*cn5);
  EDASH=315.0/512.0*sph_sma*(cn4-cn5);

  EPRIMS=sph_se/(1.0-sph_se);

  /*-------------------------------------------------
    compute the offset (lat of org - false northing)
  ---------------------------------------------------*/
  rlat=utm_lat*DEG_TO_RAD;
  one=(ADASH*rlat-BDASH*sin(2.0*rlat)+CDASH*sin(4.0*rlat)-
       DDASH*sin(6.0*rlat)+EDASH*sin(8.0*rlat))*utm_cnk;

  Y_OFFSET=one;

  /*---------------------------------
    test if the refpoint is possible
  ------------------------------------*/
  argyx=(yref-utm_fnt)*GRDCOS-(xref-utm_cmx)*GRDSIN+Y_OFFSET;
  if (fabs(argyx) > 10000000.00)
  {
    fprintf(stderr,"Check false northing or latitude\n");
    dash_err = -4;
  }

  return(dash_err);
}

#ifdef MAIN_MODULE
main()
{
  double x,y,lat,lon;
  int error;

  xref =  652636.0;
  yref = 5883591.0;

  spheroid.a = 6378388.0;
  spheroid.e2 = 0.006722670;
  projection.e0 = 500000.0;
  projection.n0 = 0.0;
  projection.k0 = 0.99996;
  projection.lon0 = 3.0;
  projection.lat0 = 0.0;

  error = dash();
  printf("x = %.2lf   y = %.2lf\n",xref,yref);
  utmgeo(xref,yref,&lat,&lon);
  printf("lat = %lf   lon = %lf\n",lat,lon);
  geoutm(lat,lon,&x,&y);
  printf("x = %.2lf    y = %.2lf\n",x,y);

  return(0);
}
#endif
