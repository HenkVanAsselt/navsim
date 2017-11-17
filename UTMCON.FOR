c---------------------------------------------------------------------
c File UTMCON contains all routines for geogs <--> grid conversions
c
c DASH		  initialization
c GEOUTM	  geogs --> grid
c UTMGEO	  grid --> geogs
c
C date 16-05-1990
c
C--------------------------- GEOUTM ----------------------------------
C
	SUBROUTINE GEOUTM(ALAT,RLONGD,TMX,TMY)
c
c alat     = latitude in degrees
c            + for north
c            - for south
c rlongd   = longitude in degrees
c            + for east
c            - for west
C
C THIS ROUTINE CALCULATES UTM COORDINATES GIVEN LAT/LONGS
c
	IMPLICIT REAL*8 (A-H,O-Z)
c
$include: 'poscom.f'
C
	real*8 ADASH,BDASH,CDASH,DDASH,EDASH,EPRIMS,
     1	Y_OFFSET,GRDSIN,GRDCOS
c
	COMMON /DASBLK/ADASH,BDASH,CDASH,DDASH,EDASH,EPRIMS,
     1	Y_OFFSET,GRDSIN,GRDCOS
c
	real*8 d_t_r,sin1t4,sin3tt,sin2t8,sin4tt,rlat,rlong
	real*8 rlongd1,alat,rlongd,tmx,tmy
	real*8 sinlat,coslat,cos2lt,sr,tempi,four,five,truex,one
	real*8 two,three,truey,modgeo
	data d_t_r/.017453292519943d0/
C
C THE FORMULAS USED FOR COMPUTATION ARE FROM THE FOCAL 801C PROGRAM AND 
C UTILIZE SOME GONIOMETRIC FORMULAS TO FACTOR OUT TEMPI (BELOW)
C (E.G. COS**2(T) - SIN**2(T)= COS(2*T)
C
	DATA SIN1T4 /.48481377579780065D-1/
	DATA SIN3TT /.18992128188729535D-4/
	DATA SIN2T8 /.11752219970342576D-2/
	DATA SIN4TT /.23019104470651324D-6/
c
c bring the value in the valid segment
c
	RLAT=modgeo(ALAT)
	RLONGD1=modgeo(RLONGD)
C
c convert degrees to radians
c
	RLAT = RLAT * d_t_r
	RLONG = RLONGD1 * d_t_r
C
C CALCULATE SIN/COS VALUES USED MORE THAN ONCE
C
	SINLAT = DSIN(RLAT)
	COSLAT = DCOS(RLAT)
	COS2LT = DCOS(2D0*RLAT)
C
	SR = modgeo(RLONGD1 - utm_cmd) * .36D0
C
C CALCULATE A TEMP
C
	TEMPI = sph_sma * utm_cnk * COSLAT /
     $		DSQRT(1D0 - sph_se * SINLAT**2)
C
C CALCULATE IV, V AND UTM X
C
	FOUR = TEMPI * SIN1T4
	FIVE = SIN3TT * TEMPI * (COS2LT + EPRIMS * COSLAT**4)
	TRUEX= SR*(FOUR + FIVE*SR**2)
C
C CALCULATE I, II, III AND UTM Y
C
	ONE=(ADASH*RLAT-BDASH*DSIN(2D0*RLAT)+CDASH*DSIN(4D0*RLAT)-
     $	    DDASH*DSIN(6D0*RLAT)+EDASH*DSIN(8D0*RLAT))*utm_cnk
c
	TWO = SIN2T8 * TEMPI * SINLAT
c
	THREE = SIN4TT * TEMPI * SINLAT*(2D0 + 3D0*COS2LT)
c
	TRUEY = (ONE + SR**2*(TWO + THREE*SR**2))-Y_OFFSET
c
	TMX=utm_cmx+TRUEX*GRDCOS-TRUEY*GRDSIN
	TMY=utm_fnt+TRUEX*GRDSIN+TRUEY*GRDCOS
C
	RETURN
	END
C
C----------------------UTMGEO------------------------------------
C
	SUBROUTINE UTMGEO(TMX,TMY,GLAT,GLONG)
c
c glat    = latude
c           + north
c           - south
c glong   = longitude
c           + east
c           - west
c
C THIS ROUTINE CONVERTS UTM X Y COORDINATES INTO LAT LONG
C
	IMPLICIT DOUBLE PRECISION (A-H,O-Z)
c
$include: 'poscom.f'
c
	real*8 ADASH,BDASH,CDASH,DDASH,EDASH,EPRIMS,
     1	Y_OFFSET,GRDSIN,GRDCOS
c
	COMMON /DASBLK/ADASH,BDASH,CDASH,DDASH,EDASH,EPRIMS,
     1	Y_OFFSET,GRDSIN,GRDCOS
C
	real*8 truex,argyx,yinit,yend,xinit,xend,qcon,phi,radphi
	real*8 d_t_r,sarc,compx,connu,sine,cose,temp1,temp2,sinsq
	real*8 cossq,tansq,seven,eigth,anine,ten,modgeo
	real*8 tmx,tmy,glat,glong
c
	data d_t_r/.017453292519943d0/
C
c correct for grid skew
c
	TRUEX=(TMX-utm_cmx)*GRDCOS+(TMY-utm_fnt)*GRDSIN
	ARGYX=(TMY-utm_fnt)*GRDCOS-(TMX-utm_cmx)*GRDSIN+Y_OFFSET
c
	YINIT=0.D0
	YEND=8881455.5D0
	XINIT=0.D0
	XEND=80.0D0
	QCON=sign(1.0D-6*DABS(TRUEX),truex)
20	PHI=XEND-(YEND-ARGYX)*(XEND-XINIT)/(YEND-YINIT)
	RADPHI=PHI*d_t_r
	SARC=ADASH*RADPHI-BDASH*DSIN(2.D0*RADPHI)
     1	+CDASH*DSIN(4.D0*RADPHI)-DDASH*DSIN(6.0D0*RADPHI)
     2	+EDASH*DSIN(8.D0*RADPHI)
C
	COMPX=utm_cnk*SARC
C
	IF(DABS(COMPX-ARGYX).LE..001D0) GO TO 100
	XEND=XINIT
	XINIT=PHI
	YEND=YINIT
	YINIT=COMPX
	GO TO 20
C
100	CONTINUE
	CONNU=sph_sma/DSQRT(1.D0-sph_se*DSIN(RADPHI)**2)
	SINE=DSIN(RADPHI)
	COSE=DCOS(RADPHI)
	TEMP1=206264.8408D0/COSE
	TEMP2=1.D6/(utm_cnk*CONNU)
	SINSQ=DSIN(RADPHI)**2
	COSSQ=DCOS(RADPHI)**2
	TANSQ=SINSQ/COSSQ
C
	SEVEN=SINE*TEMP1*TEMP2**2*(1.D0+EPRIMS*COSSQ)/2.D0
C
	EIGTH=SINE*TEMP1*TEMP2**4*(5.D0+3.D0*TANSQ
     1	+6.D0*EPRIMS*COSSQ-6.D0*EPRIMS*SINSQ
     2	-3.D0*EPRIMS**2*COSSQ**2-9.D0*EPRIMS**2
     3	*COSSQ*SINSQ)/24.D0
C
	ANINE=TEMP1*TEMP2
C
	TEN=TEMP1*TEMP2**3*(1.D0+2.D0*TANSQ+EPRIMS*COSSQ)/6.D0
C
	GLAT=PHI-(QCON**2*(SEVEN-EIGTH*QCON**2))/3600.D0
C
	GLONG=utm_cmd+QCON*(ANINE-TEN*QCON**2)/3600.D0
C
	GLONG=modgeo(GLONG)
c
c	 write(*,*) '********** utmgeo'
c	 write(*,*) glat,glong
C
	RETURN
	END
C
C-------------------------- DASH -----------------------
C
	SUBROUTINE DASH(dash_err)
C
C THIS ROUTINE CALCULATES NECESSARY CONSTANTS TO 
C CALCULATE MERIDIONAL ARC
c
	IMPLICIT DOUBLE PRECISION(A-H,O-Z)
	Integer*2 dash_err
c
$include: 'poscom.f'
C
	real*8 ADASH,BDASH,CDASH,DDASH,EDASH,EPRIMS,
     1	Y_OFFSET,GRDSIN,GRDCOS
c
	COMMON /DASBLK/ADASH,BDASH,CDASH,DDASH,EDASH,
     1	EPRIMS,Y_offset,grdsin,grdcos
c
	real*8 cn,cn2,cn3,cn4,cn5,d_t_r,rlat,one,argyx
c
	data d_t_r/.017453292519943d0/
C
	dash_err=0
	if(sph_sma.lt.6000000)then
c	    call errout('Check semi-major axis\0')
	   dash_err=-1
	   return
	endif
	if(sph_se.gt.1)then
c	    call errout('Check squared eccentricity\0')
	   dash_err=-2
	   return
	endif
	if(utm_cnk.lt..8)then
c	   call errout('Check scale factor\0')
	   dash_err=-3
	   return
	endif
c
c compute the sine and cosine of the grid skew
c
	grdsin=dsin(utm_grdskw*d_t_r)
	grdcos=dcos(utm_grdskw*d_t_r)
c
	sph_ma=sph_sma*DSQRT(1-sph_se)
	CN=(sph_sma-sph_ma)/(sph_sma+sph_ma)
	CN2=CN*CN
	CN3=CN2*CN
	CN4=CN3*CN
	CN5=CN4*CN
C
	ADASH=sph_sma*(1.D0-CN+5.D0/4.D0*(CN2-CN3)+81.D0/64.D0*(CN4-CN5))
	BDASH=3.D0/2.D0*sph_sma*(CN-CN2+7.D0/8.D0*(CN3-CN4)
     1	+55.D0/64.0D0*CN5)
C
	CDASH=15.0D0/16.0D0*sph_sma*(CN2-CN3+3.0D0/4.0D0*(CN4-CN5))
	DDASH=35.D0/48.D0*sph_sma*(CN3-CN4+11.0D0/16.0D0*CN5)
	EDASH=315.0D0/512.0D0*sph_sma*(CN4-CN5)
C
	EPRIMS=sph_se/(1.0D0-sph_se)
c
c compute the offset (lat of org - false northing)
c
	RLAT=utm_lat*D_T_R
	ONE=(ADASH*RLAT-BDASH*DSIN(2D0*RLAT)+CDASH*DSIN(4D0*RLAT)-
     $	    DDASH*DSIN(6D0*RLAT)+EDASH*DSIN(8D0*RLAT))*utm_cnk
c
	Y_OFFSET=ONE
c
c test if the refpoint is possible
c
	ARGYX=(pc_y(1)-utm_fnt)*GRDCOS-(pc_x(1)-utm_cmx)*GRDSIN+Y_OFFSET
	IF(ABS(ARGYX).GT.10000000D0)THEN
c	    CALL ERROUT('Check false northing or latitude\0')
	   dash_err=-4
	ENDIF
	RETURN
	END
