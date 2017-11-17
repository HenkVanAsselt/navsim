#include "navsim.h"

#if SYLEDIS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <window.h>

static char crlf[3] = {CR,LF,'\0'};

static void syledis_msg();
static void init_syledis(void);

NAV_SYSTEM sys_syledis =
{
  "Syledis",    	          /* name of navigation system	 		*/
  SYLEDIS,  		          /* system number for case statements 	*/
  1,                          /* number of frequencies              */
  {430e6},                    /* operation frequencies		  		*/
  {299.65e6},                 /* propagation velocity		  		*/
  RANGE_RANGE, 		          /* mode            		 			*/
  8,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  0,                          /* Number of formats                  */
  { "", },                    /* list of format names               */
  { syledis_msg },            /* List of output functions           */
  0,                          /* Actual format                      */
  1,                          /* Number of standard deviations      */
  {0.75},                     /* Standard deviations                */
  init_syledis,               /* Initialization function            */
  syledis_msg,                /* Output function                    */
  NULL,                       /* Input function                     */
  NULL,                       /* Special keyboard functions         */
  -1,                         /* Channel                            */
  -1,                         /* Slot1                              */
  4800,                       /* Baudrate                           */
  8,                          /* Databits                           */
  1,                          /* Stopbits                           */
  'N',                        /* Parity                             */
  'N',                        /* Handshake                          */
  LF,                         /* Terminator                         */
  120,                        /* Terminal count                     */
};

char syledis_format[40];

/*-------------------------------------------------------------------
|   FUNCTION: init_syledis()
|    PURPOSE: Initialize special syledis variables
| DESRIPTION: -
|    RETURNS: nothing
|    VERSION: 910411 V0.1
---------------------------------------------------------------------*/
static void init_syledis()
{
  ;
}


/*-------------------------------------------------------------------
|   FUNCTION: syledis_msg(char *msg)
|    PURPOSE: Format a SYLEDIS output message in 'msg'
| DESRIPTION: For detailed explanation of the format,
|             see syledis MANUAL, Chapter M (Digital outputs)
|    RETURNS: Nothing
|    VERSION: 901104 V0.2
---------------------------------------------------------------------*/
static void syledis_msg()
{
  int	 i,len;
  double r;
  int	 line_nr = 1;
  int	 dist_dev = 1;
  int	 delay = 12;
  int	 dB = 12;
  double lr,dtog;
  char   tmp[40];
  char   tmp_format[80];
  char   *token;
  int    item;
  char   msg[132];

  strcpy(msg,"");   /* Clear message */

  strcpy(tmp_format,syledis_format);
  token = strtok(tmp_format," \t,");
  while (token)
  {
    item = atoi(token);
    switch (item)
    {
      case 0:   /* NIL */
	    break;

      case 1:  /* Lattude or Y */
        switch (posmode)
        {
          case LATLON_DD:
          case LATLON_DDMM:
          case LATLON_DDMMSS:
            sprintf(tmp,"L%02d%02d%4.0lf%c ",
                    ship.lat_deg,ship.lat_mm,ship.lat_ss*100.00,
                    (ship.lat > 0.0 ? 'N' : 'S'));
            for (i=0 ; i<strlen(tmp)-1 ; i++)
              if (tmp[i] == ' ')
                tmp[i] = '0';
            break;
          default:  /* UTM */
            sprintf(tmp,"Y+%9.1lf ",ship.y);
            for (i=0 ; i<strlen(tmp)-1 ; i++)
              if (tmp[i] == ' ')
                tmp[i] = '0';
            break;
        }
        break;

      case 2:  /* Longitude or X */
        switch (posmode)
        {
          case LATLON_DD:
          case LATLON_DDMM:
          case LATLON_DDMMSS:
            sprintf(tmp,"G%03d%02d%4.0lf%c ",
                    ship.lon_deg,ship.lon_mm,ship.lon_ss*100.00,
                    (ship.lat > 0.0 ? 'E' : 'W'));
            for (i=0 ; i<strlen(tmp)-1 ; i++)
              if (tmp[i] == ' ')
                tmp[i] = '0';
            break;
          default:  /* UTM */
            sprintf(tmp,"X+%9.1lf ",ship.x);
            for (i=0 ; i<strlen(tmp)-1 ; i++)
              if (tmp[i] == ' ')
                tmp[i] = '0';
            break;
        }
        break;

      case 5: /* L/R parameters */
	    dtog = left_right.togo * 10;
	    lr   = left_right.rtlft;
	    sprintf(tmp,"%02dP %+07.0lf %+06.0lf ",
		line_nr,lr,dtog);
	    break;

      case 7: /* Course and speed outputs */
	    sprintf(tmp,"%03.0lfV%04.0lf ",ship.heading,ship.speed*10);
            for (i=0 ; i<strlen(tmp)-1 ; i++)
              if (tmp[i] == ' ')
                tmp[i] = '0';
	    break;

      case 8: /* Quality output */
	    sprintf(tmp,"Q3061004 ");
	    break;

      case 9: /* Date and time */
	    r = floor(realtime/100 + 0.5);
	    sprintf(tmp,"123H%06.0f ",r);
            for (i=0 ; i<strlen(tmp)-1 ; i++)
              if (tmp[i] == ' ')
                tmp[i] = '0';
            break;


      case 10:  /* 6 blanks */
	    sprintf(tmp,"      ");
	    break;

      case 11:  /* range 1 */
      case 12:  /* range 2 */
      case 13:  /* range 3 */
      case 14:  /* range 4 */
      case 15:  /* range 5 */
      case 16:  /* range 6 */
      case 17:  /* range 7 */
      case 18:  /* range 8 */
        i = item - 11;   /* calc nr between 0 and 7 */
              sprintf(tmp,"%02d%1d%02dR%06ld ",
                  station[i].code,dist_dev,dB,(long)station[i].range);
	    break;

      case 19:  /* Output-measurement delay */
	    sprintf(tmp,"Z%02d ",delay);
	    break;

      case 20: /* Event counter 1 output */
            sprintf(tmp,"1K%04d ",sys_syledis.eventnr);
	    break;

      case 21: /* Event counter 2 output */
            sprintf(tmp,"2K%04d ",sys_syledis.eventnr);
	    break;

      case 29:  /* CR/LF */
	    strcpy(tmp,crlf);
	    break;

      default:
        sprintf(tmp,"Unknown Syledis format %-d",item);
        wn_error(tmp);
        tmp[0] = '\0';
        break;

    } /* switch */

    strcat(msg,tmp);               /* Concatenate 'tmp' to message */
    token = strtok(NULL," \t,");   /* Get next token    */

  }

  /*--------------------------------------------
     Check if last part of message was CR/LF.
     If not,  add this
  ----------------------------------------------*/
  len = strlen(msg);
  if (len>2)
  {
    if ((msg[len-2] != CR) && (msg[len-1] != LF))
      strcat(msg,crlf);
  }

  strcpy(sys_syledis.out_msg,msg);

}

/*-------------------------------------------------------------------
|   FUNCTION: change_syl_format()
|    PURPOSE: Change syledis format
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901104 V0.1
---------------------------------------------------------------------*/
void change_syl_format()
{
  static char s0[]  = "SYLEDIS OUTPUT FORMATS";
  static char s1[]  = "  ";
  static char s2[]  = "0 Nil            10 6 blanks      20 Event counter 1  ";
  static char s3[]  = "1 Lat/Y          11 Distance 1    21 Event counter 2  ";
  static char s4[]  = "2 Long/X         12 Distance 2    22 -                ";
  static char s5[]  = "3 T8 ouput       13 Distance 3    23 -                ";
  static char s6[]  = "4 T56P output    14 Distance 4    24 -                ";
  static char s7[]  = "5 L/R            15 Distance 5    25 -                ";
  static char s8[]  = "6 Faults         16 Distance 6    26 -                ";
  static char s9[]  = "7 Course,speed   17 Distance 7    27 -                ";
  static char s10[] = "8 Quality        18 Distance 8    28 T8 special output";
  static char s11[] = "9 Date,time      19 Delay         29 CR/LF            ";
  static char s12[] = "  ";
  static char s13[] = "\0";

  char status[40];
  int choise;
  char *token;
  int i,j;
  char tmp_str[40];
  MENU menu[2];

  setup_menu(menu, 0, "Ok",   0, NULL, "Syledis", NULL, NULL, EXIT);
  setup_menu(menu, 1, "Edit", 0, NULL, "Syledis", NULL, NULL, EXIT);

  do
  {
    trimstr(syledis_format);
    sprintf(status,"Current format: %s    ", syledis_format);
    choise = wn_dialog(13,40,2,menu,14,s0,s1,s2,s3,
                s4,s5,s6,s7,s8,s9,s10,s11,s12,status,s13);
    if (choise == 'E')
      editstr(10,10," Syledis format ",syledis_format, _DRAW);
  }
  while (choise != 'O' && choise != ESC);

  /*--------------------------------------------------------
  | Get station numbers from format and calculate patterns
  ---------------------------------------------------------*/
  j = 0;
  strcpy(tmp_str,syledis_format);
  token = strtok(tmp_str," \t,");
  while (token)
  {
    i = atoi(token);
    if (i>=11 && i<=18)                /* Check for valid item number */
    {
      i -= 11;                         /* Adjust for base 0 */
      nav_system->p[j] = station[i].code;  /* Set pattern       */
      j++;                             /* Next pattern      */
    }
    token = strtok(NULL," \t");
  }

}

#endif
