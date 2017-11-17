/*=========================================================================

  NAVSIM.H   900602 V0.1

  Header file for NAVSIM

==========================================================================*/

#include <time.h>

#ifndef FALSE
#define FALSE 0
#define TRUE !FALSE
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#define ABS(x)  ( (x)>0   ? (x) : (-x) )
#define FABS(x) ( (x)>0.0 ? (x) : (-x) )

/*------------------
  MODE definitions
-------------------*/
#define RANGE_RANGE            0x0001
#define RANGE_BEARING          0x0002
#define HYPERBOLIC             0x0004
#define NONE_STATION           0x0008  /* Reference to coordinates (0,0) */
#define RANGE_BEARING_VERTICAL 0x0010

#define  MAX_SYSTEMS       20
#define  MAX_STATIONS	   8
#define  MAX_PATTERNS      8
#define  MAX_FREQS         4    /* Maximum number of freq's for a system */
#define  MAX_DEVS          3    /* Maximum number of standard deviations */
#define  MAX_FORMATS       8    /* Maximum number of formats in a system */

#define  ON  TRUE
#define  OFF FALSE

/*---------------------
  Character constants
----------------------*/
#define  BELL  '\a'
#define  CR    13
#define  LF    10
#define  ESC   27
#define  STX    2
#define  ETX    3
#define  SPACE ' '
#define  COMMA ','

#define  DATA	  1
#define  COMMAND  2

/*------------------------
  Conversion constantes
-------------------------*/
#define  KNOTS_TO_MS 0.514781	     /* 1 NM = 1853.21 meter */
#define  MS_TO_KNOTS 1.942575

#define  PI		     3.14159265358979323846
#define  RAD_TO_DEG  57.2957795130823229	     /* 180 / Pi */
#define  DEG_TO_RAD  0.0174532925199432955	     /* Pi / 180 */

/*----------------------
| Type definitions
----------------------*/
#define SQR(x) ((x)*(x))

/*---------------------------------------
  Type definition of nav_system record
---------------------------------------*/
typedef struct
{
  char    *name;              /* name of navigation system            */
  unsigned int sys;           /* system number for case statements    */
  int     no_frequencies;     /* Number of frequencies                */
  double  f[MAX_FREQS];       /* operation frequencies                */
  double  vc[MAX_FREQS];      /* propagation velocity                 */
  int     mode;               /* mode, (Range/Range, Hyperbolic       */
  int     max_patterns;       /* maximum number of patterns           */
  int     no_patterns;        /* Number of patterns system is using   */
  int     p[MAX_PATTERNS];    /* Selected stations or patterns        */
  int     no_formats;         /* Number of formats                    */
  char    *format_names[MAX_FORMATS];  /* list of format names        */
  void    *output_list[MAX_FORMATS];   /* list of output functions    */
  int     format;                      /* Actual output format        */
  int     no_std_devs;                 /* No. of standard deviations  */
  double  std_dev[MAX_DEVS];           /* Standard deviations         */
  void    (*init_func)();              /* Initialization function     */
  void    (*output_func)();            /* Ptr to message builder      */
  void    (*input_func)(char *s);      /* Input msg handler           */
  void    (*keybd_func)(int c);        /* Special keyboard actions    */
  int     channel;                     /* Output channel              */
  int     slot;                        /* Slot for input              */
  int     baudrate;                    /* Baudrate for this system    */
  int     databits;                    /* No of databits              */
  int     stopbits;                    /* No of stopbits              */
  char    parity;                      /* Parity                      */
  char    handshake;                   /* Handshake                   */
  int     terminator;                  /* Terminator of input msg     */
  int     term_count;                  /* Maximum no. of input chars  */
  double  dt;                          /* Time interval for system    */
  int     eventnr;                     /* Event number of this system */
  long    update_time;                 /* New update time for system  */
  char    out_msg[256];                /* Output message              */
} NAV_SYSTEM;

/*-----------------------------------
  Type definition of station record
------------------------------------*/
typedef struct
{
  char  name[80];        /* station name or place indication             */
  int	code;		 /* station code. If -1, station not selected    */
  double x,y,h;          /* station coordinates                          */
  double range;          /* Distance between ship and station            */
  double C_O;            /* Calculated Minus Observed (fixed error       */
} STATION;

/*------------------------------------
  Type definition of spheroid record
-------------------------------------*/
typedef struct
{
  char    name[40];     /* Name of spheroid                     */
  double  a,            /* Semi-Major axis                      */
          e2;           /* square of eccentricity of spheroid   */
} SPHEROID;

/*--------------------------------
  Type definition of projection record
---------------------------------*/
typedef struct
{
  char   name[40];              /* Name of projection                                         */
  double e0,		    /* False easting			      		*/
	n0,		    		/* False northing			     		*/
	k0,		    		/* Scale factor on central meridian     */
	lon0,		    	/* Central Meridian 		     		*/
	lat0;		    	/* Mean lattitude of work area	     	*/
} GRID;

/*--------------------------------
  Type definition of ship record
---------------------------------*/
typedef struct
{
  double x,y,		  	/* easting,northing		             */
         h,			    /* height			                 */
         speed,		    /* speed in knots		             */
         bearing,		/* bearing to a station or waypoint  */
         azimuth,		/* bearing from station to ship      */
         vert_angle, 	/* vertical angle to station	     */
         heading,		/* heading in RAD		             */
         true_heading,  /* true heading in RAD 	             */
         helm;		    /* position of rudder in RAD	     */
  double lat,lon;	    /* lattitude and longitude as double in degrees */
  int    lat_deg,       /* Degrees of lattitude              */
         lat_mm;        /* Minutes of lattitude              */
  double lat_ss;        /* seconds of lattitude              */
  int    lon_deg,       /* Degrees of longitude              */
         lon_mm;        /* Minutes of longitude              */
  double lon_ss;        /* seconds of longitude              */
  char   xpos_str[20];  /* Position as a string              */
  char   ypos_str[20];  /* Position as a string              */
} SHIP;

/*----------------------------------
  Type definition of waypoint record
-----------------------------------*/
typedef struct
{
  double  e,n;
}
WAYPOINT;

/*----------------------------------
  Type definition of profile record
-----------------------------------*/
#define MAX_PROFILE_POINTS 15
typedef struct
{
  double startx, starty;                  /* Start coordinates of block */
  double endx,   endy;                    /* End coordinates of block   */
  double length;                          /* Length of line 0           */
  double direction;                       /* Direction of line 0 in deg */
  double dx;                              /* X interval                 */
  double dy;                              /* Y interval                 */
  int    no_lines;                        /* Number of lines in block   */
  int    linenr;                          /* Current line number        */
  double x1,y1,x2,y2;                     /* SOL and EOL coordinates    */
  double dist[MAX_PROFILE_POINTS];        /* Array of distance points   */
  double depth[MAX_PROFILE_POINTS];       /* Array of depth points      */
}
PROFILE;

/*--------------------------------------
  Type definition of left/right record
---------------------------------------*/
typedef struct
{
  double a,b,c,d;              /* coefficients                 */
  double distance,             /* distance along line          */
         rtlft,                /* distance to line             */
         travel,               /* distance traveled along line */
         togo;                 /* distance to go along line    */
  long	 scale;		       /* scale of L/R display	       */
}
LEFT_RIGHT;

/*--------------------
| Navigation systems
---------------------*/
#define TRISPONDER 1
#define ARTEMIS    2
#define MICROFIX   3
#define HYPERFIX   4
#define DECCA      5
#define SYLEDIS    6
#define POLARTRACK 7
#define FALCON     8
#define UCM40      9
#define ECHOTRAC   10
#define NMEA_0183  11
#define NR53       12
#define DESO25     13
#define RADARFIX   14
#define PULSE8     15
#define TRIMBLE    16

#ifndef TRISPONDER
#define TRISPONDER 0
#else
extern NAV_SYSTEM sys_trisponder;
#endif

#ifndef ARTEMIS
#define ARTEMIS    0
#else
extern NAV_SYSTEM sys_artemis;
void bcd_01(void);
void bcd_001(void);
void bcd_adb(void);
#endif

#ifndef MICROFIX
#define MICROFIX   0
#else
extern NAV_SYSTEM sys_microfix;
#endif

#ifndef HYPERFIX
#define HYPERFIX   0
#else
extern NAV_SYSTEM sys_hyperfix;
#endif

#ifndef DECCA
#define DECCA      0
#else
extern NAV_SYSTEM sys_decca;
#endif

#ifndef SYLEDIS
#define SYLEDIS    0
#else
extern NAV_SYSTEM sys_syledis;
#endif

#ifndef POLARTRACK
#define POLARTRACK 0
#else
extern NAV_SYSTEM sys_polartrack;
#endif

#ifndef FALCON
#define FALCON     0
#else
extern NAV_SYSTEM sys_falcon;
#endif

#ifndef UCM40
#define UCM40      0
#else
extern NAV_SYSTEM sys_ucm40;
#endif

#ifndef ECHOTRAC
#define ECHOTRAC   0
#else
extern NAV_SYSTEM sys_echotrac;
#endif

#ifndef NMEA_0183
#define NMEA_0183  0
#else
extern NAV_SYSTEM sys_nmea_0183;
#endif

#ifndef NR53
#define NR53       0
#else
extern NAV_SYSTEM sys_nr53;
#endif

#ifndef DESO25
#define DESO25     0
#else
extern NAV_SYSTEM sys_deso25;
#endif

#ifndef RADARFIX
#define RADARFIX   0
#else
extern NAV_SYSTEM sys_radarfix;
#endif

#ifndef PULSE8
#define PULSE8     0
#else
extern NAV_SYSTEM sys_pulse8;
#endif

#ifndef TRIMBLE
#define TRIMBLE    0
#else
extern NAV_SYSTEM sys_trimble;
#endif

/*-----------------------
|  Function prototypes
-----------------------*/

/* NAVSIM.C */
void simulate(void);
int  selection_ok(void);
void process_command(void);
void dummy_help(void);
int  main(void);
void gen_posstr(int posmode);
void display_all(void);
void clear_all_windows(void);

/*----------
  FILIO.C
----------*/
void base_name(char *name);

int  load_system(char *filename);
int  save_system(char *filename);
int  save_system_as(void);

void load_projection(void);
int  load_prj_file(char *filename);

void load_spheroid(void);
int  load_sph_file(char *filename);

/*-------------
|  SIMDISP.C
-------------*/
#define INPUT_WINDOW    1
#define OUTPUT_WINDOW   2
#define SHIP_WINDOW     3
#define POSITION_WINDOW 4

enum disp_items {DISP_X,DISP_Y,DISP_H,DISP_SPEED,DISP_HEADING,DISP_HELM,
                 DISP_BEARING,DISP_AZIMUTH,DISP_VERTICAL,DISP_RANGE,
                 DISP_INPUT,DISP_OUTPUT,DISP_TIME,DISP_SYSTEM_IN_COMMAND,
                 DISP_DEPTH};

void disp_serial_error(int *error);
void display(int item_no);
void disp_input(char *s);
void disp_output(char *s);
void disp_param_info(void);
void open_disp_window(int w);
void clear_disp_window(int w);
void close_disp_window(int w);
void disp_status(char *format, ...);

/*-------
  INIT.C
---------*/
void exit_prog(void);
void init_format(void);
void initialize(void);
void set_io(NAV_SYSTEM *sys);

/*--------
  CALC.C
---------*/

double calc_sf(double e1, double n1, double e2, double n2,
               double *true_dist, double *sf);
double calc_range(int i);
double pround(double number, int power);
double calc_bearing(int i);
double calc_lane(double range_master, double range_slave,
		double baseline, double lambda);
void  deg_to_dms(double r, int *deg, int *mm, double *ss);
void  dms_to_deg(int deg, int mm, double ss, double *d);
void  dms_to_rad(int deg, int mm, double ss, double *rad);
void  rad_to_dms(double rad, int *deg, int *mm, double *ss);
void  calc_geog_const(void);
void  projection_to_geog(double e, double n , double *lat, double *lon);
void  geog_to_projection(double lat, double lon, double *x, double *y);
void  calc_spheroid_const(void);
void  check_bear(double *b);
void  calc_position(int mode, double dt);
void  calc_lr_coeff(LEFT_RIGHT *lr, double x1, double y1, double x2, double y2);
void  calc_leftright(LEFT_RIGHT *left_right, double x, double y);
int   find_station(int code);
void  calc_patterns(void);
void  get_paramfile_name (char *filename);
long  lane_reading(int i, int f);
double generate_noise(double max_noise);
int   find_master(int code);
int   find_slave(int code);
void  calc_baselines(void);
int   dash(void);

/*-----------
  UTMCON.C
------------*/
#define UTM            0
#define LATLON         1
#define LATLON_DD      2
#define LATLON_DDMM    3
#define LATLON_DDMMSS  4
#define UTM_X          5
#define UTM_Y          6

void geoutm(double lat, double lon, double *x, double *y);
void utmgeo(double x, double y, double *lat, double *lon);

/*-------------
  GETPOS.C
--------------*/
int get_pos(int *mode, char *pos_str, double *lat, double *lon, double *x, double *y);

/*------------
  MENU.C
-------------*/
void change_debug(void);
void debug_help(void);
void change_projection(void);
void select_std_spheroid(void);
void change_spheroid(void);
void change_pattern(void);
void change_format(void);
void change_xsystems(void);
void change_spec_params(void);
void change_frequency(void);
void change_output(void);
int  check_pattern(void);
void toggle_printer(void);

/*------------
  UTIL.C
-------------*/
void decode_bcd(char *s, int n);
void wait(void);
void statusline(char *msg, ...);
int  yes_no(char *msg);
void msg_output(char *msg);
void print_params(void);
int  check_printer(void);
void print_debug(char *format, ...);

/*------------
| KEYS.C
-------------*/
void process_key(void);

/*-------------
  SYLEDIS.C
-------------*/
void change_syl_format(void);

/*---------------
  PROFILE.C
-------------*/
void default_profile(PROFILE *profile);
void calc_profile_line(PROFILE *profile, int linenr);
double calc_direction(double x1, double y1, double x2, double y2);
void check_profile(double x, double y, PROFILE *p);
void profile_menu(void);
void edit_profile(void);
int  calc_depths(double x, double y, double *depth1, double *depth2, double dt);
void load_profile(void);
int  load_prf_file(char *filename);
void save_profile(void);
int  save_prf_file(char *filename);
void save_prof_as(void);

/*--------
  NMEA.C
---------*/
unsigned int add_checksum(char *m);
void nmea_gll(char *talker, char *m, int res3d);
void nmea_sll(char *talker, char *m);
void nmea_vhw(char *talker, char *m);
void nmea_zzu(char *talker, char *m);
void nmea_gga(char *talker, char *m);
void nmea_vtg(char *talker, char *m);

/*-------------
  STATIONS.C
-------------*/
int  load_chain(char *filename);
int  save_chain(char *filename);
void edit_stn_data(void);
void load_stations(void);
void save_stations(void);
void save_stations_as(void);

/**********************************************************************
*	VARIABLE DECLARATIONS		                    			      *
**********************************************************************/

#ifdef MAIN_MODULE

/*--------------------------------------------------------------
    Global simulation variables
---------------------------------------------------------------*/

NAV_SYSTEM *systeem[MAX_CHANNELS];   /* MAX_SYSTEMS pointers to systems      */
NAV_SYSTEM *sys_array[MAX_SYSTEMS];  /* Array of pointers to system struct's */
NAV_SYSTEM *nav_system;
int      system_in_command = 0;
int      no_navsystems;          /* Actual number of nav systems         */
int      no_stations;	         /* number of stations	                 */
STATION  station[MAX_STATIONS];
double   baseline[MAX_STATIONS];
int      new_start;
char     chain_name[20];
char     systemfile_name[20];

  /*--- Simulation variables ---*/

SHIP	   ship;

char	    systemdata_str[80];	/* name of file with system data	  */
GRID        projection;
SPHEROID    spheroid;
PROFILE     profile;
int         profile_mode = OFF;
char        profile_name[80];

/*--- Waypoint and left/right parameters ---*/
WAYPOINT    waypoint;
LEFT_RIGHT  left_right;
int 	    lr_enabled;

  /*--- Parameter file contents ---*/
char	    data_dir[80];
char	    p_date[80];
char	    p_area[80];
char	    p_remarks[80];

/*--- Time variables ---*/
long       realtime;
struct tm  *tmnow;

int   lr_enabled;
int   latlong;

char  out_msg[256];

void  (interrupt far *old_int1b_vec)(void);

int   printer_out;
int   debug;
int   update_display = TRUE;
double max_noise;
double depth1 = 5.0;                      /* Depth of channel 2         */
double depth2 = 6.0;                      /* Depth of channel 1         */

int posmode = UTM;

#else

extern  NAV_SYSTEM  *systeem[];
extern  NAV_SYSTEM  *sys_array[];
extern  NAV_SYSTEM  *nav_system;
extern  int         system_in_command;
extern  char        *nav_system_name[];
extern  SHIP        ship;
extern  struct tm   *tmnow;
extern  char        systemdata_str[];     /* file name with system data  */
extern  GRID        projection;
extern  SPHEROID    spheroid;
extern  PROFILE     profile;
extern  int         profile_mode;
extern  char        profile_name[];
extern  WAYPOINT    waypoint;
extern  LEFT_RIGHT  left_right;
extern  int         lr_enabled;
extern  char        data_dir[];
extern	char	    p_date[];
extern	char	    p_area[];
extern	char	    p_remarks[];
extern  STATION     station[];
extern  double      baseline[];
extern  int         new_start;
extern  char        chain_name[];
extern  char        systemfile_name[];

extern  int         no_navsystems;
extern  int         no_stations;
extern  long        realtime;
extern  int         lr_enabled;
extern  int         latlong;
extern  char        out_msg[];

extern  void  (interrupt far *old_int1b_vec)(void);
extern  int   printer_out;
extern  int   debug;
extern  int   update_display;

extern  double max_noise;
extern  double depth1;               /* Depth of channel 2         */
extern  double depth2;               /* Depth of channel 1         */

extern  int posmode;

#endif

