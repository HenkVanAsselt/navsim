#include "navsim.h"

#if FALCON

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <front9.h>
#include <window.h>
#include <time.h>

#define REMOTE_XY               1
#define REMOTE_RANGES           2
#define PRINT_XY                3
#define PRINT_RANGES            4

/*------------------------------
| Prototypes of local functions
--------------------------------*/
static void remote_xy_msg(void);
static void remote_ranges_msg(void);
static void print_xy_msg(void);
static void print_ranges_msg(void);
static int  mnemonic(char *cmd);
static void init_falcon(void);
static void falcon_cmd(char *cmd);
static void site_selection(char *tmp);

NAV_SYSTEM sys_falcon =
{
  "Falcon",     	          /* name of navigation system	 		*/
  FALCON,   		          /* system number for case statements 	*/
  0,                          /* number of frequencies              */
  {0.0},                      /* operation frequencies		  		*/
  {1.0},                      /* propagation velocity		  		*/
  RANGE_RANGE, 		          /* mode            		 			*/
  4,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  4,                          /* Number of formats                  */
  { "Remote XY",              /* list of format names               */
    "Remote ranges",
    "Print  XY",
    "Print  ranges",
  },
  { remote_xy_msg,            /* List of output functions           */
    remote_ranges_msg,
    print_xy_msg,
    print_ranges_msg,
  },
  1,                          /* Actual format                      */
  1,                          /* Number of standard deviations      */
  {2.0},                      /* Standard deviations                */
  init_falcon,                /* Initialization function            */
  remote_xy_msg,              /* Output function                    */
  falcon_cmd,                 /* Input function                     */
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


/*---------------
| Local defines
----------------*/
#define DS 3      /* Side data                                  */
#define ER 9      /* Read/write of RS-232C output enable        */
#define IT 22     /* Read/write of time of day                  */
#define IU 23     /* Read/write of system units                 */
#define IW 24     /* Read/write of maximum range velocity       */
#define OM 26     /* Read/write of measurement and update types */
#define OR 28     /* Read/write of ranging type                 */
#define OS 29     /* Read/write of Site selections              */
#define OV 30     /* Read/write of Event status                 */
#define RE 31     /* End ranging mode                           */
#define RS 32     /* Begin ranging mode                         */
#define TS 34     /* Execute hardware self-test                 */
#define ERROR -1  /* Error in mnemonic                          */

#define ACK 0x06  /* Handshake characters */
#define NAK 0x15

/*-----------------
| Local variables
------------------*/
static int output = FALSE;

static char NAK_STR[] = {NAK,'\0'};
static char ACK_STR[] = {ACK,'\0'};
static int len1 = 1;

/*-------------------------------------------------------------------
|   FUNCTION: Mnemonic(char *s)
|    PURPOSE: Decode mnemonic of received command
| DESRIPTION: Decodes characater mnemonic and returns an integer,
|             representing the mnemonic. If the first character of
|             the message was not an STX, it will return ERROR (-1)
|    RETURNS: Integer code representing the mnemonic
|    VERSION: 901120 V0.1
---------------------------------------------------------------------*/
static int mnemonic(char *s)
{
  int i = 0;

  if ( s[i++] != STX )
    return(ERROR);

  switch(s[i++])
  {
    case 'D': if (s[i] == 'S') return(DS);
              break;
    case 'E': if (s[i] == 'R') return(ER);
              break;
    case 'I': if (s[i] == 'U') return(IU);
              else if (s[i] == 'T') return(IT);
              else if (s[i] == 'W') return(IW);
              break;
    case 'O': if (s[i] == 'M') return(OM);
              else if (s[i] == 'R') return(OR);
              else if (s[i] == 'S') return(OS);
              else if (s[i] == 'V') return(OV);
              break;
    case 'R': if (s[i] == 'E') return(RE);
              else if (s[i] == 'S') return(RS);
              break;
    case 'T': if (s[i] == 'S') return(TS);
              break;
    default : return(ERROR);
  }

  return(ERROR);

}

/*-------------------------------------------------------------------
|   FUNCTION: falcon_cmd(char *cmd)
|    PURPOSE: Process falcon command
| DESRIPTION: Decodes received command and acts on it. It will
|             answer the controller on serial line with ACK if
|             if it ready to receive the next command, or with
|             NAK if an error has been detected. It will also
|             attend the user on this fact.
|    REMARKS: Only the writing (not reading) of a small number of
|             variables is implemented at the moment.
|    RETURNS: Nothing
|    VERSION: 901120 V0.1
---------------------------------------------------------------------*/
static void falcon_cmd(char *cmd)
{
  int mne;
  char *delim = "\CR,";
  char *s;                  /* Pointer to part of command string */
  char *token;
  int  i;
  double f;

  mne = mnemonic(cmd);
  if (mne == ERROR)
  {
    output_string(&sys_falcon.channel,NAK_STR,&len1);
    return;
  }
  else
  {
    s = (cmd+3);
    switch(mne)
    {
      case RS: output = TRUE;         /* Ranging start */
               break;
      case RE: output = FALSE;        /* Ranging end   */
               break;

      /*----------------------------------------------
      | System units. 1 = meters, 2 = feet, 3 = yards
      ------------------------------------------------*/
      case IU: break;        /* No (re)action */

      /*------------------------
      | Maximum range velocity
      -------------------------*/
      case IW: break;       /* No (re)action */

      /*-----------
      | Site data
      -------------*/
      case DS: token = strtok(s,delim);           /* Site number */
               if (token)
                 i = atoi(token) - 1;             /* Adjust for zero base */

               token = strtok(NULL,delim);        /* Code */
               if (token)
                 station[i].code = atoi(token);

               token = strtok(NULL,delim);        /* X */
               if (token)
               {
                 f = atof(token);
                 if (f > 0.5) station[i].x = f;
               }

               token = strtok(NULL,delim);        /* Y */
               if (token)
               {
                 f = atof(token);
                 if (f > 0.5) station[i].y = f;
               }

               token = strtok(NULL,delim);        /* Z */
               if (token)
               {
                 f = atof(token);
                 if (f > 0.5) station[i].h = f;
               }
               break;

      /*----------------
      | Site selection
      ------------------*/
      case OS: i = 0;
               token = strtok(s,delim);
               while (token)
               {
                 nav_system->p[i++] = atoi(token);
                 token = strtok(NULL,delim);
               }
               nav_system->no_patterns = i;
               break;

      /*----------------------------
      | Measurement and update type
      ------------------------------*/
      case OM: token = strtok(s,delim);  /* 0 = range only, 1 = with XY */
               i = atoi(token);
               if (i == 0)
               {
                 nav_system->format = REMOTE_RANGES;
                 nav_system->output_func = remote_ranges_msg;
               }
               else if (i==1)
               {
                 nav_system->format = REMOTE_XY;
                 nav_system->output_func = remote_xy_msg;
               }
               token = strtok(NULL,delim);     /* 0=Automatic, 1=Trigger */
               token = strtok(NULL,delim);     /* Update rate in seconds */
                 nav_system->dt = atof(token);
               break;

      /*--------------
      | Range type
      --------------*/
      case OR: ;  break;       /* No (re)action */

      /*---------------
      | Event status
      ---------------*/
      case OV: token = strtok(s,delim);      /* Event status code   */
               token = strtok(NULL,delim);   /* Event start number  */
               nav_system->eventnr = atoi(token);
               token = strtok(NULL,delim);   /* Updates/event ratio */
               break;

      /*------------------
      | Hardware selftest
      --------------------*/
      case TS: break;   /* Not implemented */

      default: break;
    }
    output_string(&sys_falcon.channel,ACK_STR,&len1);
    return;
  }

}

/*-------------------------------------------------------------------
|   FUNCTION: init_falcon()
|    PURPOSE: Initializes output of FALCON
| DESRIPTION: The user has to choose between unconditional message
|             output or the following of commands, supplied by a
|             controller (e.g. AUTOCARTA).
|    RETURNS: Nothing
|    VERSION: 901120 V0.2
---------------------------------------------------------------------*/
static void init_falcon()
{
  int choise;

  static MENU menu[] =
  {
    { "Normal",        0, },
    { "Unconditional", 0, },
    { NULL }
  };

  setup_menu(menu, 0, "Normal",        0, NULL, NULL, NULL, NULL, EXIT);
  setup_menu(menu, 1, "Unconditional", 0, NULL, NULL, NULL, NULL, EXIT);

  output = FALSE;

  /*-----------------------
  | Ask for kind of action
  -------------------------*/
  choise = wn_dialog(13,40,2,menu,2,
           "NORMAL        : wait for start-up sequence from AUTOCARTA",
           "UNCONDITIONAL : start message output right-away          ");

  switch (choise)
  {
    case ESC: return;
    case 'N': return;
    case 'U': output = TRUE; break;
    default : break;
  }

}

/*-------------------------------------------------------------------
|    FUNCTION: site_selection()
|     PURPOSE: Generate data about channel and site
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 910904 V0.1 - Initial version
---------------------------------------------------------------------*/
static void site_selection(char *tmp)
{
  int i,j,b;

  b = 0;
  i = 0;
  j = find_station(0);
  if (j >= 0)
  {
    b += 0x08;  /* Channel D */
    i++;
  }
  j = find_station(1);
  if (j >= 0)
  {
    b += 0x04;  /* Channel C */
    i++;
  }
  j = find_station(2);
  if (j >= 0)
  {
    b += 0x02;  /* Channel B */
    i++;
  }
  j = find_station(3);
  if (j >= 0)
  {
    b += 0x01;  /* Channel A */
    i++;
  }

  if (b < 10)
    tmp[0] = (char) '0' + b;
  else
    tmp[0] = (char) ('A' - 10 + b);
  tmp[1] = (char) '0' + i;
  tmp[2] = ',';
  tmp[3] = '\0';
  return;
}

/*-------------------------------------------------------------------
|   FUNCTION: remote_xy_msg(char *msg)
|    PURPOSE: Generate Falcon 'Remote XY' message
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901111 V0.1
---------------------------------------------------------------------*/
static void remote_xy_msg()
{
  char tmp[80];
  int  signal = 45;
  double residual = 0.3333;
  double ecr = 3.9;
  double ssr = 2.0;
  int  i;
  char msg[256];

  if (!output)
  {
    msg[0] = '\0';
    return;
  }

  /*---------------------------
  | STX and time (HH:MM:SS.s)
  ----------------------------*/
  sprintf(msg,"%c%-d,%-d,%-d.0,",
    STX,tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);

  /*----------------------------------------
  | Station codes, ranges and signal levels
  ------------------------------------------*/
  for (i=0 ; i<4 ; i++)       /* For channels A,B,C,D */
  {
    sprintf(tmp,"%-d,%-.1lf,%-d,%-.1lf,",
      station[i].code,station[i].range,signal+i,residual*i);
    strcat(msg,tmp);
  }

  /*-----------------------
  | X/Y coordinate, speed
  ------------------------*/
  sprintf(tmp,"%-.0lf,%-.0lf,%-.1lf,",ship.x,ship.y,ship.speed);
  strcat(msg,tmp);

  /*----------------------------
    Channel and Site selection
  -----------------------------*/
  site_selection(tmp);
  strcat(msg,tmp);

  /*----------------------------
  | Filter status (0 = locked)
  -----------------------------*/
  strcat(msg,"0,");

  /*----------------------------------
  | Low signal strength status (hex)
  -----------------------------------*/
  strcat(msg,"0,");

  /*-------------------
  | ECR and SSR (???)
  -------------------*/
  sprintf(tmp,"%-.1lf,%-.1lf,",ecr,ssr);
  strcat(msg,tmp);

  /*---------------------------
  | Event number + CR/LF + ETX
  ----------------------------*/
  sprintf(tmp,"%-d%c",nav_system->eventnr,ETX);
  strcat(msg,tmp);

  strcat(sys_falcon.out_msg,msg);
}

/*-------------------------------------------------------------------
|   FUNCTION: remote_ranges_msg(char *msg)
|    PURPOSE: Generate Falcon 'Remote ranges-only' message
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901111 V0.1
---------------------------------------------------------------------*/
static void remote_ranges_msg()
{
  char tmp[80];
  int  signal = 45;
  int  i;
  char msg[256];

  if (!output)
  {
    msg[0] = '\0';
    return;
  }

  /*---------------------------
  | STX and time (HH:MM:SS.s)
  ----------------------------*/
  sprintf(msg,"%c%-d,%-d,%-d.0,",
    STX,tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);

  /*----------------------------------------
  | Station codes, ranges and signal levels
  ------------------------------------------*/
  for (i=0 ; i<4 ; i++)       /* For channels A,B,C,D */
  {
    sprintf(tmp,"%-d,%-.1lf,%-d,",station[i].code,station[i].range,signal+i);
    strcat(msg,tmp);
  }

  /*----------------------------
    Channel and Site selection
  -----------------------------*/
  site_selection(tmp);
  strcat(msg,tmp);

  /*----------------------------------
  | Low signal strength status (hex)
  -----------------------------------*/
  strcat(msg,"0,");

  /*---------------------------
  | Event number + CR/LF + ETX
  ----------------------------*/
  sprintf(tmp,"%-d%c",nav_system->eventnr,ETX);
  strcat(msg,tmp);

  strcat(sys_falcon.out_msg,msg);
}

/*-------------------------------------------------------------------
|   FUNCTION: print_xy_msg()
|    PURPOSE: Generate Falcon 'Print XY' message
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901111 V0.1
---------------------------------------------------------------------*/
static void print_xy_msg()
{
  char tmp[80];
  int  signal = 45;
  double residual = 0.3333;
  double ecr = 3.9;
  double ssr = 2.0;
  int  i;
  char msg[256];

  if (!output)
  {
    sys_falcon.out_msg[0] = '\0';
    return;
  }

  /*---------------------------
  | STX and time (HH:MM:SS.s)
  ----------------------------*/
  sprintf(msg,"%c%02d:%02d:%02d.0 ",
    STX,tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);

  /*----------------------------------------
  | Station codes, ranges and signal levels
  ------------------------------------------*/
  for (i=0 ; i<4 ; i++)       /* For channels A,B,C,D */
  {
    sprintf(tmp,"%2d %8.1lf %2d%9.1lf ",
      station[i].code,station[i].range,signal+i,residual*i);
    strcat(msg,tmp);
  }

  /*-----------------------
  | X/Y coordinate, speed
  ------------------------*/
  sprintf(tmp,"%8.0lf %8.0lf %5.1lf ",ship.x,ship.y,ship.speed);
  strcat(msg,tmp);

  /*----------------------------
    Channel and Site selection
  -----------------------------*/
  site_selection(tmp);
  strcat(msg,tmp);

  /*----------------------------
  | Filter status (0 = locked)
  -----------------------------*/
  strcat(msg,"0 ");

  /*----------------------------------
  | Low signal strength status (hex)
  -----------------------------------*/
  strcat(msg,"0 ");

  /*-------------------
  | ECR and SSR (???)
  -------------------*/
  sprintf(tmp,"%8.1lf %8.1lf ",ecr,ssr);
  strcat(msg,tmp);

  /*---------------------------
  | Event number + CR/LF + ETX
  ----------------------------*/
  sprintf(tmp,"%4d%c%c%c",nav_system->eventnr,CR,LF,ETX);
  strcat(msg,tmp);

  strcat(sys_falcon.out_msg,msg);
}

/*-------------------------------------------------------------------
|   FUNCTION: print_ranges_msg()
|    PURPOSE: Generate Falcon 'Print Ranges-only' message
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901111 V0.1
---------------------------------------------------------------------*/
static void print_ranges_msg()
{
  char tmp[80];
  int  signal = 45;
  int  i;
  char msg[246];

  if (!output)
  {
    msg[0] = '\0';
    return;
  }

  /*---------------------------
  | STX and time (HH:MM:SS.s)
  ----------------------------*/
  sprintf(msg,"%c%02d:%02d:%02d.0 ",
    STX,tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);

  /*----------------------------------------
  | Station codes, ranges and signal levels
  ------------------------------------------*/
  for (i=0 ; i<4 ; i++)       /* For channels A,B,C,D */
  {
    sprintf(tmp,"%2d %8.1lf %2d ",station[i].code,station[i].range,signal+i);
    strcat(msg,tmp);
  }

  /*----------------------------
    Channel and Site selection
  -----------------------------*/
  site_selection(tmp);
  strcat(msg,tmp);

  /*----------------------------------
  | Low signal strength status (hex)
  -----------------------------------*/
  strcat(msg,"0 ");

  /*---------------------------
  | Event number + CR/LF + ETX
  ----------------------------*/
  sprintf(tmp,"%4d%c%c%c",nav_system->eventnr,CR,LF,ETX);
  strcat(msg,tmp);

  strcat(sys_falcon.out_msg,msg);
}


#endif
