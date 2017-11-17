#include "navsim.h"

#if TRISPONDER

#include <stdio.h>
#include <string.h>

/*--------------------------------
| Prototype's of local functions
----------------------------------*/
static void trispndr_a4();
static void trispndr_a5();

NAV_SYSTEM sys_trisponder =
{
  "Trisponder",  	          /* name of navigation system	 		*/
  TRISPONDER,		          /* system number for case statements 	*/
  1,                          /* number of frequencies              */
  {9.48e6},                   /* operation frequencies		  		*/
  {299.650e6},                /* propagation velocity		  		*/
  RANGE_RANGE, 		          /* mode            		 			*/
  4,	                      /* maximum number of patterns	 		*/
  0,                          /* Number of patters system is using  */
  {0},                        /* Selected stations or patterns	 	*/
  2,                          /* Number of formats                  */
  {"A4","A5"},                /* list of format names               */
  { trispndr_a4,              /* List of output functions           */
    trispndr_a5
  },
  1,                          /* Actual format                      */
  0,                          /* Number of standard deviations      */
  {0.0},                      /* Standard deviations                */
  NULL,                       /* Initialization function            */
  trispndr_a4,                /* Output function                    */
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
  1.0,                        /* Dt                                 */
};

static char crlf[3] = {CR,LF,'\0'};

/*------------------------------------------------------------------
  Format trisponder A4 message
-------------------------------------------------------------------*/
static void trispndr_a4()
{
  int i,j;
  char tmp[20];
  char msg[256];

  sprintf(msg,"%3d ",nav_system->eventnr);

  for (i=0 ; i<nav_system->max_patterns ; i++)
  {
    j = find_station(nav_system->p[i]);
    if (j>=0)
      sprintf(tmp,"%2d %7.1f",station[j].code,station[j].range);
    else
      sprintf(tmp,"%2d %7.1f",0,0.0);
    strcat(msg,tmp);
    if (i != 3) strcat(msg,"  ");
  }
  strcat(msg,crlf);
  strcpy(sys_trisponder.out_msg,msg);
}

/*------------------------------------------------------------------
  Format trisponder A5 message
-------------------------------------------------------------------*/
static void trispndr_a5()
{
  int i,j;
  char tmp[20];
  char msg[256];

  sprintf(msg,"%3d",nav_system->eventnr);

  for (i=0 ; i<nav_system->max_patterns ; i++)
  {
    j = find_station(nav_system->p[i]);
    if (j>=0)
    {
      sprintf(tmp,",%2d,%7.1f",station[j].code,station[j].range);
      strcat(msg,tmp);
    }
  }
  strcat(msg,crlf);
  strcpy(sys_trisponder.out_msg,msg);
}

#endif
