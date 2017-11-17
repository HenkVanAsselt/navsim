/****************************************************************************
*                                                                           *
*        FILE: STATIONS.C                                                   *
*                                                                           *
*     PURPOSE: Navsim station editing routines                              *
*                                                                           *
* DESCRIPTION: -                                                            *
*                                                                           *
*     HISTORY: 910910 V0.1                                                  *
*              920303 V0.2 - Menu's stay on screen now                      *
*                                                                           *
****************************************************************************/

/*---------------------------------------------------------------------------
                        HEADER FILES
---------------------------------------------------------------------------*/
#include "navsim.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <window.h>

/*---------------------------------------------------------------------------
                        PROTOTYPES
---------------------------------------------------------------------------*/
static void popup_station(void);

/*---------------------------------------------------------------------------
                        LOCAL VARIABLES
---------------------------------------------------------------------------*/
static int station_nr = 0;

/*===========================================================================
                        START OF FUNCTIONS
===========================================================================*/

/*-------------------------------------------------------------------
|   FUNCTION: popup_station()
|    PURPOSE: Popup station data for editing
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 910909 V0.1 - Initial version
---------------------------------------------------------------------*/
static void popup_station()
{
  #define NO_ITEMS 7

  int i;
  int choise = ESC;
  MENU menu[NO_ITEMS];

  i = station_nr;

  do
  {
    /*-----------------
      Initialize menu
    ------------------*/
    setup_menu(menu, 0, "Name", -1, NULL, NULL, station[i].name,  "%s",      0);
    setup_menu(menu, 1, "Code", -1, NULL, NULL, &station[i].code, "%d",      0);
    setup_menu(menu, 2, "   X", -1, NULL, NULL, &station[i].x,    "%10.2lf", 0);
    setup_menu(menu, 3, "   Y", -1, NULL, NULL, &station[i].y,    "%10.2lf", 0);
    setup_menu(menu, 4, "   H", -1, NULL, NULL, &station[i].h,    "%10.2lf", 0);
    setup_menu(menu, 5, " C-O", -1, NULL, NULL, &station[i].C_O,  "%10.2lf", 0);
    setup_menu(menu, 6, "Exit",  0, NULL, NULL, NULL,             NULL,      0);

    /*---------------------------
      Show and edit station data
    -----------------------------*/
    choise = popup(NO_ITEMS,menu,10,10);

    /*--------------------
      Previous station ?
    ---------------------*/
    if (choise == ARROW_LEFT)
    {
      i--;
      i = max(0,i);
    }

    /*----------------
      Next station
    ----------------*/
    if (choise == ARROW_RIGHT)
    {
      i++;
      i = min(i,MAX_STATIONS);
    }

  }
  while (choise != ESC && choise != 'E');

  #undef NO_ITEMS
}

/*-------------------------------------------------------------------
|   FUNCTION: edit_stn_data()
|    PURPOSE: Edit station data
| DESRIPTION: Ask for station number to edit and makes it
|             possible to edit name,code,X,Y,H.
|    RETURNS: Nothing
|    VERSION: 901104 V0.2
---------------------------------------------------------------------*/
void edit_stn_data(void)
{
  int i,j;
  int choise = ESC;
  char s[MAX_STATIONS+1][80];
  MENU menu[MAX_STATIONS+1];

  sprintf(s[0],"%-15.15s   %-4s  %9.9s  %10.10s  %6.6s  %6.6s",
    "Name","CODE","X","Y","Z","C-O");
  setup_menu(menu,0,s[0],-1,NULL,"stations",NULL,NULL,COMMENT);

  do
  {
    for (i=0 ; i<MAX_STATIONS ; i++) {
      sprintf(s[i+1],"%-15.15s   %2d  %9.2f  %10.2f  %6.2f  %6.2f",
              station[i].name,station[i].code,
              station[i].x,station[i].y,station[i].h,station[i].C_O);
      setup_menu(menu, i+1, s[i+1], -1, NULL, "stations", NULL, NULL, 0);
    }

    choise = popup(MAX_STATIONS+1,menu,10,10);   /* +1 for comment line */

    if (choise >= 1 && choise < MAX_STATIONS+1)
    {
      station_nr = choise-1;
      popup_station();
    }
  }
  while (choise != ESC);

}

/*-------------------------------------------------------------------
|    FUNCTION: load_chain()
|     PURPOSE: load chain data
| DESCRIPTION: -
|     RETURNS: TRUE if successfull, FALSE if no
|     HISTORY: 910822 V0.1
---------------------------------------------------------------------*/
int load_chain(char *filename)
{
  int i;
  FILE *infile;
  char s[40];

  /*---------------
  | Open datafile
  ----------------*/
  strcpy(s,filename);
  base_name(s);
  strcat(s,".CHN");

  if (!filename[0])
    return(FALSE);

  infile = fopen(s,"r");
  if (!infile)
  {
    wn_error("ERROR: could't load chain file %s",s);
    return(FALSE);
  }

  disp_status("Loading chain from file '%s'",filename);

  /*--------------------
    Read file header
  -------------------*/
  fgets(p_date,80,infile);
  trimstr(p_date);
  fgets(p_area,80,infile);
  trimstr(p_area);
  fgets(p_remarks,80,infile);
  trimstr(p_remarks);

  fscanf(infile,"%s %d\n",s,&no_stations);
  i = 0;

  while (!feof(infile))
  {
#define S station[i]
    if (!fgets(S.name,80,infile))
      continue;
    trimstr(S.name);
    if (S.name[0] == '-')
      strcpy(S.name,"");
    fscanf(infile,"%d %lf %lf %lf %lf\n",&S.code,&S.x,&S.y,&S.h,&S.C_O);
    i++;
    if (i >= MAX_STATIONS)
      break;
#undef S
  }
  fclose(infile);
  return(TRUE);

}

/*-------------------------------------------------------------------
|    FUNCTION: save_chain()
|     PURPOSE: save chain data
| DESCRIPTION: -
|     RETURNS: TRUE if successfull, FALSE if no
|     HISTORY: 910822 V0.1
---------------------------------------------------------------------*/
int save_chain(char *filename)
{
  int  i;
  char s[40];
  FILE *outfile;

  /*------------------
  | Open output file
  -------------------*/
  strcpy(s,filename);
  base_name(s);
  strcat(s,".CHN");
  outfile = fopen(s,"w");
  if (outfile == NULL)
  {
    wn_error("ERROR: couldn't save chainfile %s",s);
    return(FALSE);
  }

  disp_status("Saving chain in file '%s'",s);

  /*--------------------
    Write file header
  -------------------*/
  fprintf(outfile,"%s\n",p_date);
  fprintf(outfile,"%s\n",p_area);
  fprintf(outfile,"%s\n",p_remarks);

  fprintf(outfile,"%15s %2d\n","STATIONS",MAX_STATIONS);

  for (i=0 ; i<MAX_STATIONS ; i++)
  {
    #define S station[i]
    trimstr(S.name);
    if (S.name[0] == '\0')
      fprintf(outfile, "%s\n%2d %lf %lf %lf %lf\n",
        "-", 0, 0.0, 0.0, 0.0, 0.0);
    else
      fprintf(outfile, "%s\n%2d %lf %lf %lf %lf\n",
        S.name,S.code,S.x,S.y,S.h,S.C_O);
    #undef S
  }
  fclose(outfile);
  return(TRUE);
}

/*-------------------------------------------------------------------
|   FUNCTION: save_stations()
|    PURPOSE: Save station data on disk
| DESRIPTION: Save stations on disk with current chain name
|    RETURNS: Nothing
|    VERSION: 901124 V0.1 - Save only current chain name
---------------------------------------------------------------------*/
void save_stations(void)
{
  /*
  editstr(10,10,"date   ",p_date,_DRAW);
  editstr(10,10,"area   ",p_area,_DRAW);
  editstr(10,10,"remarks",p_remarks,_DRAW);
  */

  save_chain(chain_name);
}

/*-------------------------------------------------------------------
|   FUNCTION: save_stations_as()
|    PURPOSE: Save station data on disk with user defined file name
| DESRIPTION: Will give change to change descriptive items,
|             asks for a file name and save station data on disk
|    RETURNS: Nothing
|    VERSION: 901104 V0.2
|             900501 V0.3 - Ask for filename
---------------------------------------------------------------------*/
void save_stations_as(void)
{
  int ret = ESC;

  ret = editstr(-1,-1,20,"chain name", chain_name, _DRAW);
  if (ret == RETURN)
  {
    editstr(-1,-1,20,"date   ",p_date,_DRAW);
    editstr(-1,-1,20,"area   ",p_area,_DRAW);
    editstr(-1,-1,20,"remarks",p_remarks,_DRAW);
    save_chain(chain_name);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: load_stations()
|    PURPOSE: Load system data from disk
| DESRIPTION: Determines filename of system data and tries to load
|             the file from disk.
|    RETURNS: Nothing
|    VERSION: 901104 V0.2
|             900501 V0.3 - Will read all system data from disk
---------------------------------------------------------------------*/
void load_stations(void)
{
  int success = FALSE;
  char filename[15];

  if (get_dir("*.chn",filename) != NULL)
  {
    strcpy(chain_name,filename);
    success = load_chain(chain_name);
    if (success)
    {
      disp_param_info();
    }
  }
}
