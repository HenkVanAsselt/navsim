/**************************************************************
*                                                             *
*   Artemis MKIV functions                                    *
*                                                             *
*   901105 V0.1                                               *
*                                                             *
**************************************************************/

#include "navsim.h"

#if ARTEMIS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <window.h>
#include <front9.h>

/* prototypes */

void modify_art_env(char *cmd);

/*-----------------------
| Artemis constants
------------------------*/
#define  MOB	  0
#define  FIX	  1

typedef struct
{
  double
    setpoint,		     /* B */
    freq_code,
    tx_addr_code,	     /* C */
    low_addr_code_lim,
    high_addr_code_lim,
    rcvd_addr_code,
    dist_deviation,	     /* E */
    PRF_sel_switch,
    PRF_sel_code,
    tx_microwave_freq,	 /* F */
    freq_ctrl_DAC,
    freq_sense_ADC,
    alpha,		         /* H */
    betha,
    preamp_gain_switch,  /* J */
    tx_power,
    aut_long_short,
    per_out_tlgrm_type,
    per_in_tlgrm_type,
    in_lock_flag,
    left_scan_limit,	 /* L */
    right_scan_limit,
    tuning,		         /* N */
    ref_azimuth,	     /* R */
    ref_azimuth_180,
    setup_flag, 	     /* U */
    operating_mode,
    station_is_fix,
    remote_mod_protect,
    audio_volume,	     /* V */
    wake_hh,
    wake_mm,
    wake_ss,
    wake_day;	         /* W */
}
ARTEMIS_ENV;

ARTEMIS_ENV    *artemis;
char	       artemis_format[132], *art_format;
int            artemis_status;
short	       stn_addr;	  /* Controller is addressing MOB or FIX   */
short	       host_on; 	  /* Controller is connected to MOB or FIX */
int            other;             /* command is addressed to other station */

static int request;                   /* cmd requests a telegram    */
static char output_msg[132];          /* Output message             */

static char crlf[3] = {CR,LF,'\0'};   /* CR/LF                      */
static char *cmd;                     /* Copy of pointer to command */
static int  rate,data_par,stop;

/* static int  i; */
static char m[132];                   /* message when building it   */

#define  ART     artemis[stn_addr]
#define  ART_FIX artemis[FIX]
#define  ART_MOB artemis[MOB]

void init_artemis(void);
void art_telegram(void);

/*-------------------------------------------------------------------
|   FUNCTION: artemis_cmd(char *command)
|    PURPOSE: execute an artemis MK-IV command
| DESRIPTION: Checks if the commandstring is a valid command, and if
|             it is a data request or a command to change parameters.
|             It then takes the apropiate action
|    RETURNS: Nothing
|    VERSION: 910425 V0.4
---------------------------------------------------------------------*/
void artemis_cmd(char *command)
{
  char       tmp_str[80];
  int        i = 1;
  char       *t,*cmd;

  /*-----------------------------------------
  | Skip this function if not Mark IV mode
  -----------------------------------------*/
  if (nav_system->output_func != art_telegram)
    return;

  /*--- Initializations ---*/
  cmd      = command;         /* Copy pointer to incoming command string */
                              /*   This pointer is static for this file  */
  request  = FALSE;           /* Assume telegram is no request           */
  other    = FALSE;           /* Assume telegram is for this unit        */
  strcpy(artemis_format,"");  /* Reset telegram format                   */
  stn_addr  = host_on;        /* Default: message is for station on host */

  strupr(cmd);

  /*-------------------------------------------------------
    for debugging only : strip leading CR and LF (if any)
  --------------------------------------------------------*/
  while ((*cmd == LF) || (*cmd == CR)) cmd++;

  /*-----------------------------------
    Convert trailing CR/LF in a '\0'
  -----------------------------------*/
  t = cmd;
  while (*t)
  {
    if ((*t == CR) || (*t == LF)) *t = '\0';
    t++;
  }

  /*---------------------------------------------------------
    Exit this routine if an syntax error is detected
  ---------------------------------------------------------*/
  if ( !isupper(*cmd) && *cmd != '?')
  {
    disp_status("ERROR in received command '%s'",cmd);
    return;
  }

  /*---------------------------------------------------------
    Check if it is a command for this station or for the
    other and test if it is a request for a telegram or
    a modifing command
  ---------------------------------------------------------*/
  if (*cmd == 'O')
  {
    other = TRUE;
    cmd++;
    if (host_on == MOB)       /* command is for other station    */
      stn_addr = FIX;
    else
      stn_addr = MOB;
  }

  if (*cmd == '?')
  {
    request = TRUE;
    cmd++;
  }

  /*----------------------------------------
    If it is not a request telegram, then
    modify the artemis enviroment
  -----------------------------------------*/
  if (!request)
  {
    modify_art_env(cmd);     /* modify parameters of artemis enviroment */
    return;
  }

  /*-----------------------------------------------------------------
    If this command is a request for a telegram,
    build the format string, which will be used in format_artemis()
  ------------------------------------------------------------------*/
  if (request)
  {
    i = 0;
    while (isupper(*cmd))
      artemis_format[i++] = *cmd++;
    artemis_format[i] = '\0';           /* Terminate format string */
  }

  /*--------------------------------------------------------
    If next character is '\'
      process the new update time, given in the command.
    else we have to send the message only once.
  ---------------------------------------------------------*/
  if ((request) && (*cmd == '\\'))
  {
    cmd++;
    if (isdigit(*cmd))
    {
      /*------------------------------------------------
        Decode new time interval
        calculate time interval dt and new update time
      -------------------------------------------------*/
      i = 0;
      while(isdigit(*cmd))
      tmp_str[i++] = *cmd++;
      nav_system->dt = 0.25 * atoi(tmp_str);    /* new update time */
      nav_system->update_time = realtime + (long) floor(nav_system->dt * 100 + 0.5);
      /*------------------------------------------------
        Format output message to send it continuously
      ------------------------------------------------*/
      if (nav_system->output_func)
        (*(nav_system->output_func))(nav_system->out_msg);
    }
    else
    {
      /*---------------------------------------
        No interval given,
        stop sending data repeatedly
      ---------------------------------------*/
      nav_system->dt = 10000.0;                     /* new update interval ...     */
      nav_system->update_time = realtime + 10000;   /* new update time is far away */
      output_msg[0] = '\0';             /* clear output message        */
    }
  }
  else
  {
    /*------------------------------------------------
      Format output message and send it only once
    ------------------------------------------------*/
    nav_system->dt = 10000.0;
    nav_system->update_time = realtime + 10000L;
    if (nav_system->output_func)
      (*(nav_system->output_func))(nav_system->out_msg);
    display(DISP_OUTPUT);
    msg_output(nav_system->out_msg);
  }

  if (nav_system->output_func)
    (*(nav_system->output_func))(nav_system->out_msg);
  display(DISP_OUTPUT);
  msg_output(nav_system->out_msg);
}

/*------------------------------------------------------------------
  Get value(s) of paramters to modify out of the command string
  If a '$' was found the parameter of this field will not be
  modified. Also the parameter will not be modified if an prematur
  end of string was encounterd.

  900605 V0.2
-------------------------------------------------------------------*/
void get_value(double *r)
{
  char s[20];
  int  i=0;

  while (isspace(*cmd))      /* Skip blanks */
    cmd++;

  if (*cmd == '$')        /* No modification for this parameter */
  {
    cmd++;
    return;
  }

  if (!*cmd)          /* End of string reached, so return */
    return;
  else
  {
    i = 0;
    while (strchr("0123456789+-.",*cmd))
      s[i++] = *cmd++;
    *r = (double) atof(s);
    return;
  }
}

/*------------------------------------------------------------------
  Modify parameters of artemis enviroment, because the received
  command was not a request

  910425 V0.2
-------------------------------------------------------------------*/
void modify_art_env(char *cmd)
{
  int i;
  double r;
  double dummy;
  char id;

  if (artemis == NULL) init_artemis();

  if (*cmd)
    id = *cmd++;
  else
    return;

  switch (id)
  {
    case 'A':  /* Azimuth, read only */
      break;

    case 'B':
      get_value(&dummy);           /* Mobile ant. bearing (not implemented */
      get_value(&ART.setpoint);    /* Antenna direction setpoint           */
      break;

    case 'C' :
      get_value(&ART.freq_code);            /* Frequency code */
      i = (int) floor(ART.freq_code+0.5);
      switch (i)
      {
        case 1 : nav_system->f[1] = 9200.0; nav_system->f[2] = 9230.0; break;
        case 2 : nav_system->f[1] = 9300.0; nav_system->f[2] = 9270.0; break;
        case 3 : nav_system->f[1] = 9230.0; nav_system->f[2] = 9200.0; break;
        default: nav_system->f[1] = 9270.0; nav_system->f[2] = 9300.0; break;
      }
      get_value(&ART.tx_addr_code);         /* Transmit address code   */
      get_value(&ART.low_addr_code_lim);    /* Low address code limit  */
      get_value(&ART.high_addr_code_lim);   /* High address code limit */
      get_value(&dummy);                    /* Receive address code    */
      break;

    case 'D' :   /* Distance, read only */
      break;

    case 'E' :
      if (stn_addr == MOB)
      {
        get_value(&dummy);            /* Deviation distance measurement */
        get_value(&ART.PRF_sel_switch);
        get_value(&ART.PRF_sel_code);
        get_value(&dummy);            /* Distance (before calibration   */
      }
      break;

    case 'F' : /* Transmitted microwave freq (read only),
                  freq. control DAC setting (r/w) and
                  freq. sense ADC setting (read only)
               */
      get_value(&dummy);
      get_value(&ART.freq_ctrl_DAC);
      get_value(&dummy);
      break;

    case 'G' :   /* Gyro. Read only */
      break;


    case 'H' :  /* Heading (MOB r/w) and tracking filter settings (FIX r/w) */
      if (stn_addr == FIX)
      {
        get_value(&dummy);
        get_value(&ART_FIX.alpha);
        get_value(&ART_FIX.betha);
      }

    case 'I' :  /* Servo voltage & Phase detector output. Read only */
      break;

    case 'J' :  /* Different kinds of parameters */
      get_value(&ART.preamp_gain_switch);
      get_value(&ART.tx_power);
      get_value(&ART.aut_long_short);
      get_value(&ART.per_out_tlgrm_type);
      get_value(&ART.per_in_tlgrm_type);
      break;

    case 'K' : /* AGC voltage and lock in flag. Read only */
      break;

    case 'L' : /* Autosearch left and right scan limits. MOB read only */
      if (stn_addr == FIX)
      {
        get_value(&ART_FIX.left_scan_limit);
        get_value(&ART_FIX.right_scan_limit);
      }
      break;

    case 'R' : /* Reference azimuths. MOB read only */
      if (stn_addr == FIX)
      {
        get_value(&r);
        ART.ref_azimuth = r;
        r = r + 180.0;
        check_bear(&r);
        ART.ref_azimuth_180 = r;
      }
      break;

    case 'T' :  /* Time and date. Read Only */
      break;

    case 'U' : /* Set-up flag, operating mode, station is FIX flag,
                  Remote modify protect flag */
      get_value(&ART.setup_flag);
      get_value(&ART.operating_mode);
      get_value(&ART.station_is_fix);
      if (stn_addr == FIX)
        get_value(&ART.remote_mod_protect);
      break;

    case 'V' :  /* Audio volume and software version (last is read only) */
      get_value(&ART.audio_volume);
      break;

    case 'W' : /* Wake time and wake day of month (not implemented) */
      break;

    case 'Y' : /* Parameters about serial comm's. not implemented for writing */
      break;

  } /* switch id of ... */

}

void art_telegram()
{
  char msg[80];

  if (artemis == NULL) init_artemis();    /* if not already done ... */

  strcpy(artemis_format,"ADB");

  if (!artemis_format[0]) return;	  /* no format specified */

  if (other)
    strcpy(msg,"O");  /* Identifier of other station */
  else
    msg[0] = '\0';	/* Clear total message	  */
  m[0]   = '\0';	/* Clear sub	message    */

  art_format = artemis_format;
  while (*art_format)
  {
    m[0] = '\0';        /* Reset sub message */
    switch (*art_format)
    {
    case 'A' :    /* Azimuth (same for fix and mobile) */
     sprintf(m,"A%.3f%s",ship.azimuth,crlf);
     break;

    case 'B' :   /* MOB ant. bearing and antenna direction setpoint */
      if (stn_addr == MOB)
        sprintf(m,"B%.1f %.1f%s", ship.azimuth, ART.setpoint,crlf);
      if (stn_addr == FIX)
        sprintf(m,"B 0 %.1f%s", ART.setpoint,crlf);
      break;

  case 'C' :
    sprintf(m,"C%1.0f %2.0f %2.0f %2.0f %2.0f%s",
	  ART.freq_code,
	  ART.tx_addr_code,
	  ART.low_addr_code_lim,
	  ART.high_addr_code_lim,
	  ART.rcvd_addr_code,
	  crlf);
    break;

  case 'D' :  /* Distance from pattern[0] */
    sprintf(m,"D%.1f%s",station[0].range,crlf);
    break;

  case 'E' :
    if (stn_addr == MOB)
    {
      sprintf(m,"E%.2f %1.0f %1.0f %.2f%s",
      ART.dist_deviation,     /* Deviation dist. measurement */
      ART.PRF_sel_switch,     /* Auto PRF selection switch   */
	  ART.PRF_sel_code,       /* PRF selection code          */
      station[0].range,       /* Distance before calibration */
      crlf);
    }
    if (stn_addr == FIX)
      sprintf(m,"E0 0 0 0%s",crlf);  /* Only Mobile reading possible */
    break;

  case 'F' :
    sprintf(m,"F%.0f %.0f %.0f%s",
	  ART.tx_microwave_freq,
	  ART.freq_ctrl_DAC,
	  ART.freq_sense_ADC,
	  crlf);
    break;

  case 'G' :   /* Gyro */
    sprintf(m,"G%.1f%s",ship.heading,crlf);
    break;

  case 'H' :
  /*-------------------------------------------------------------------
    Heading, max 14,
     1 : FIX - , MOB RW, max 359.9, Heading,
     2 : FIX RW, MOB - , max 99,    Alpha azimuth tracking filter,
     3 : FIX RW, MOB - , max 99,    Betha azimuth tracking filter
  ---------------------------------------------------------------------*/
    if (stn_addr == MOB)
    {
      sprintf(m,"H%.1f%s",ship.heading,crlf);  /* Heading */
    }
    if (stn_addr == FIX)
    {
      sprintf(m,"H0 %.0f %.0f%s",
       ART_FIX.alpha,                 /* Alpha azimuth tracking filter */
       ART_FIX.betha,                 /* Betha azimuth tracking filter */
       crlf);
    }
    break;

  case 'I' :
    sprintf(m,"I30 30%s",crlf); /* Servo voltage and Phase detector output */
    break;

  case 'J' :
    sprintf(m,"J%1.0f %1.0f %1.0f %1.0f %1.0f%s",
	  ART.preamp_gain_switch,
	  ART.tx_power,                   /* (long/short) */
	  ART.aut_long_short,
	  ART.per_out_tlgrm_type,
	  ART.per_in_tlgrm_type,
	  crlf);
    break;

  case 'K' :   /* AGC voltage and 'in lock' flag */
    sprintf(m,"K31 %1.0f%s", ART.in_lock_flag, crlf);
      ART.in_lock_flag = 1.0;     /* Set flag to one after one check */
    break;

  case 'L' :   /* Autosearch left/right scan limits */
    if (stn_addr == FIX)
    {
      sprintf(m,"L%.0f %.0f%s",
        ART_FIX.left_scan_limit,
        ART_FIX.right_scan_limit,
        crlf);
    }
    else
      sprintf(m,"L0 0%s",crlf);
    break;

  case 'M' :   /* Battry/supply voltage and gunn supply voltage */
    sprintf(m,"M25.1 15.8%s",crlf);
    break;

  case 'N' :  /* Tuning, tep. dist. circuit and temp. waveguide */
    sprintf(m,"N%.0f 30 32%s", ART.tuning, crlf);
    break;

  case 'P' :  /* Voltages of 5V, -6V, 12V, -12V and 6V) */
    sprintf(m,"P5.1 -6.2 12.3 -12.2 6.3%s",crlf);
    break;

  case 'Q' :   /* Quality figures */
    sprintf(m,"Q7 7 7%s",crlf);
    break;

  case 'R' : /* Reference azimuth and ref. azimuth + 180 */
    if (stn_addr == FIX)
    {
      sprintf(m,"R%.3f %.3f%s",
       ART.ref_azimuth,
       ART.ref_azimuth_180,
       crlf);
    }
    else
      sprintf(m,"R0 0%s",crlf);
    break;

  case 'S' :  /* Signal levels: digital (dB) and analogue (dB) */
    sprintf(m,"S-50 52%s", crlf);
    break;

  case 'T' : /* Time and date */
    sprintf(m,"T,%02d:%02d:%02d %02d/%02d/%02d%s",
      tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec,
      tmnow->tm_year, tmnow->tm_mon, tmnow->tm_mday, crlf);
    break;

  case 'U' :   /* Different kind of flags */
    if (stn_addr == MOB)
    {
      sprintf(m,"U%1.0 %1.0 %1.0%s",
        ART.setup_flag,
        ART.operating_mode,
        ART.station_is_fix,
        crlf);
    }
    else
    {
      sprintf(m,"U%1.0 %1.0 %1.0 %1.0%s",
        ART.setup_flag,
        ART.operating_mode,
        ART.station_is_fix,
        ART.remote_mod_protect,
        crlf);
    }
    break;

  case 'V' :   /* Audio volume and software version */
    sprintf(m,"V%1.0f Z99z9Z9/==%s", ART.audio_volume, crlf);
    break;

  case 'W' : /* Wake time and wake day of month */
    sprintf(m,"W%02d:%02d:%02d %02d%s",    /* current time + 2 hours */
      tmnow->tm_hour+2,tmnow->tm_min,tmnow->tm_sec,tmnow->tm_mday,crlf);
    break;

  case 'X' :  /* Xtal currents */
    sprintf(m,"X-100 99 -111 88%s",crlf);
    break;

  case 'Y' :  /* Serial parameters of host and peripheral device */
              /* Implemented for host ...                   */
    switch (sys_artemis.baudrate)
    {
      case   50 : rate = 0; break;
      case  110 : rate = 1; break;
      case  134 : rate = 2; break;
      case  200 : rate = 3; break;
      case  300 : rate = 4; break;
      case  600 : rate = 5; break;
      case 1200 : rate = 6; break;
      case 1050 : rate = 7; break;
      case 2400 : rate = 8; break;
      case 4800 : rate = 9; break;
    }

    switch (toupper(sys_artemis.parity))
    {
      case 'E' : data_par = 0; break;
      case 'O' : data_par = 2; break;
      case 'N' : data_par = 4; break;
    }

    if (sys_artemis.databits == 8) data_par++;

    stop = sys_artemis.stopbits - 1;

    /*--- For periphial device: fixed data of 9 9 5 0 ---*/
    sprintf(m,"Y%1d %1d %1d %1d 9 9 5 0%s",
	  rate,rate,data_par,stop,crlf);

  default:
    break;

   } /* switch *art_format ... */
   strcat(sys_artemis.out_msg,m);
   art_format++;

   } /* while (*art_format) */

}

/*-------------------------------------------------------------------
|   FUNCTION: init_artemis()
|    PURPOSE: initialize artemis enviroment
| DESRIPTION:
|    RETURNS: Nothing
|    VERSION: 910425 V0.2
---------------------------------------------------------------------*/
void init_artemis()
{
  int i;

  /*-----------------------------------------
  | Initialize input- and output terminators
  -------------------------------------------*/

  if (nav_system->output_func == bcd_01  ||
      nav_system->output_func == bcd_001 ||
      nav_system->output_func == bcd_adb )
  {
    nav_system->terminator = 0xFF;
    set_io(nav_system);
  }
  else
  {
    nav_system->terminator = LF;
    set_io(nav_system);
  }

  /*---------------------------------------------------------
  | Perform this function only if:
  | 1) The telegram output format is choosen (Mark IV) and
  | 2) If this is the first time
  ----------------------------------------------------------*/
  if (nav_system->output_func != art_telegram)
    return;
  if (artemis)
    return;

  /*-------------------------------
  | Setup of artemis enviroment
  -------------------------------*/
  if (!artemis)               /* Check if memory allocated */
  {
    artemis = (ARTEMIS_ENV *)calloc(2,sizeof(ARTEMIS_ENV));
    if (!artemis)
    {
      puts("Not enough memory to allocate ARTEMIS enviroment");
      exit(-1);
    }
  }
  artemis_format[0] = '\0';   /* Clear format line		 */
  artemis_status    = 1;
  host_on = MOB;		  /* Controller connected to MOB */
  stn_addr = MOB;		  /* Addressing MOB		 */
  artemis[MOB].station_is_fix = 0.0;
  artemis[FIX].station_is_fix = 1.0;

  for (i=MOB ; i<FIX ; i++)
  {
	artemis[i].setpoint	      = 123.5;
	artemis[i].freq_code	      = 0.0;
	artemis[i].tx_addr_code       = 21.0;
	artemis[i].low_addr_code_lim  = 0.0;
	artemis[i].high_addr_code_lim = 63.0;
	artemis[i].rcvd_addr_code     = 34.0;
	artemis[i].dist_deviation     = 1.23;
	artemis[i].PRF_sel_switch     = 0.0;
	artemis[i].PRF_sel_code       = 0.0;
	artemis[i].tx_microwave_freq  = 9290.0;
	artemis[i].freq_ctrl_DAC      = 50.0;
	artemis[i].freq_sense_ADC     = 60.0;
	artemis[i].alpha	      = 50.0;
	artemis[i].betha	      = 60.0;
	artemis[i].tx_power	      = 0.0;
	artemis[i].aut_long_short     = 0.0;
	artemis[i].per_out_tlgrm_type = 0.0;
	artemis[i].per_in_tlgrm_type  = 0.0;
	artemis[i].in_lock_flag       = 0.0;
	artemis[i].left_scan_limit    = 100.0;
	artemis[i].right_scan_limit   = 300.0;
	artemis[i].tuning	      = 60.0;
	artemis[i].ref_azimuth	      = 0.0;
	artemis[i].ref_azimuth_180    = 180.0;
	artemis[i].setup_flag	      = 0.0;
	artemis[i].operating_mode     = 1.0;
	artemis[i].remote_mod_protect = 0.0;
	artemis[i].audio_volume       = 5.0;
	artemis[i].wake_hh	      = 12.0;
	artemis[i].wake_mm	      = 34.0;
	artemis[i].wake_ss	      = 56.0;
	artemis[i].wake_day	      = 31.0;
  }

}

#endif
