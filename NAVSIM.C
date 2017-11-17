/****************************************************************************
*                                                                           *
*        FILE: NAVSIM.C                                                     *
*                                                                           *
*     PURPOSE: NAVIGATION SIMULATION PROGRAM MAIN SOURCE FILE               *
*                                                                           *
* DESCRIPTION: -                                                            *
*                                                                           *
*     HISTORY: 911214 V3.0 - Added routines to get ship position in         *
*                            UTM or in lat/long                             *
*                                                                           *
*                                                                           *
****************************************************************************/

#define  MAIN_MODULE

/*---------------------------------------------------------------------------
                        HEADER FILES
---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <window.h>
#include <front9.h>
#include <hva_util.h>
#include <bios.h>
#include "navsim.h"

#ifdef __MSC__
#include <stdarg.h>
#endif

/*---------------------------------------------------------------------------
                        PROTOTYPES
---------------------------------------------------------------------------*/
static void process_input(NAV_SYSTEM *nav_system);
static void do_help(void);
static void terminal(void);
static void load_all(void);
static void save_all(void);
static void save_all_as(void);
static void get_ship_x(void);
static void get_ship_y(void);
static void do_exit(void);
static void toggle_profile(void);
static void change_projection_e0(void);
static void change_projection_n0(void);
static void change_projection_k0(void);
static void change_projection_lat0(void);
static void change_projection_lon0(void);
static void change_display_mode(void);

/*---------------------------------------------------------------------------
                        LOCAL VARIABLES
---------------------------------------------------------------------------*/
static time_t tnow;

MENU M_simulate[] =
{
  { "Start       ", 0, simulate,     "Start" ,       NULL, NULL, EXIT},
  { "Print params", 0, print_params, "Print params", NULL, NULL, NO_FLAGS},
  { "OS shell    ", 0, OS_shell,     "OS shell",     NULL, NULL, EXIT},
  { "Mode        ", 0, change_display_mode, "Mode", display_mode_str, "%s", NO_FLAGS},
  { "Quit        ", 0, do_exit,      "Quit" , NULL,   NULL, EXIT},
  { NULL }
};

MENU M_navsystem[] =
{
  { "Current", 0, NULL, "equipment menu", systemfile_name, "%s" ,COMMENT},
  { "Load   ", 0, load_all,        "equipment menu", NULL, NULL, NO_FLAGS},
  { "Save   ", 1, save_all_as,     "equipment menu", NULL, NULL, NO_FLAGS},
  { "Edit   ", 0, change_xsystems, "equipment menu", NULL, NULL, NO_FLAGS},
  { NULL }
};

static MENU M_spheroid[] =
{
  { "Current spheroid",  0, NULL, "spheroid", spheroid.name, "%s", COMMENT },
  { "Load            ",  0, load_spheroid, "spheroid",  NULL, NULL, NO_FLAGS },
  { "Semi major axis ",  0, NULL, "spheroid", &spheroid.a,    "%.3lf", EDIT_STR },
  { "Excentricity^2  ",  0, NULL, "spheroid", &spheroid.e2,   "%.9lf", EDIT_STR },
  { NULL }
};

static MENU M_projection[] =
{
  { "Current grid", 0, NULL,                   "grid", projection.name, "%s", COMMENT  },
  { "Load        ", 2, load_projection,        "grid", NULL,            NULL,     NO_FLAGS },
  { "False East  ", 6, change_projection_e0,   "grid", &projection.e0,   "%.0lf", EDIT_STR },
  { "False North ", 6, change_projection_n0,   "grid", &projection.n0,   "%.0lf", EDIT_STR },
  { "Scale Factor", 0, change_projection_k0,   "grid", &projection.k0,   "%.5lf", EDIT_STR },
  { "Lon Org.    ", 1, change_projection_lon0, "grid", &projection.lon0, "%.0lf", EDIT_STR },
  { "Lat Org.    ", 1, change_projection_lat0, "grid", &projection.lat0, "%.0lf", EDIT_STR },
  { NULL }
};

static MENU M_ship[] =
{
  { "X      ", 0, get_ship_x, "ship", ship.xpos_str, "%s",   NO_FLAGS },
  { "Y      ", 0, get_ship_y, "ship", ship.ypos_str, "%s",   NO_FLAGS },
  { "Z      ", 0, NULL,       "ship", &ship.h,       "%.1lf",EDIT_STR },
  { "Speed  ", 0, NULL,       "ship", &ship.speed,   "%.1lf",EDIT_STR },
  { "Heading", 0, NULL,       "ship", &ship.heading, "%.1lf",EDIT_STR },
  { "Helm   ", 1, NULL,       "ship", &ship.helm,    "%.1lf",EDIT_STR },
  { NULL }
};

static MENU M_stations[] =
{
  { "Chain:", 0, NULL,             "stations", chain_name, "%s", COMMENT },
  { "Load  ", 0, load_stations,    "stations", NULL, NULL, NO_FLAGS },
  { "Save  ", 0, save_stations_as, "stations", NULL, NULL, NO_FLAGS },
  { "Edit  ", 0, edit_stn_data,    "stations", NULL, NULL, NO_FLAGS },
  { NULL },
};

static MENU M_profile[] =
{
  { "Profile", 0, NULL,           "profile", profile_name,    "%s", COMMENT },
  { "Load   ", 0, load_profile,   "profile", NULL,            NULL, NO_FLAGS },
  { "Save   ", 0, save_prof_as,   "profile", NULL,            NULL, NO_FLAGS },
  { "Edit   ", 0, profile_menu,   "profile", NULL,            NULL, NO_FLAGS },
  { "Mode   ", 0, toggle_profile, "profile", &profile_mode,   "%B", NO_FLAGS },
  { NULL }
};

MENU_HEAD heads[] =
{
  { "Simulate",   0, 5, M_simulate   },
  { "Equipment",  0, 4, M_navsystem  },
  { "Spheroid",   1, 3, M_spheroid   },
  { "Projection", 1, 6, M_projection },
  { "Ship",       1, 6, M_ship       },
  { "Stations",   1, 4, M_stations   },
  { "Profile",    3, 4, M_profile    },
  { NULL }
};

/*===========================================================================
                        START OF FUNCTIONS
===========================================================================*/

/*-------------------------------------------------------------------
|    FUNCTION: change_display_mode()
|     PURPOSE: Toggle mode from monochrome to color
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 911125 V0.1 - Initial version
---------------------------------------------------------------------*/
static void change_display_mode()
{
  if (display_mode == COLOR)
    set_display_colors(MONO);
  else
    set_display_colors(COLOR);
}

/*-------------------------------------------------------------------
|    FUNCTION: do_exit()
|     PURPOSE: Perform exit(0) call from pulldown menu
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 911107 V0.1 - Initial version
---------------------------------------------------------------------*/
static void do_exit()
{
  exit(0);
}

/*-------------------------------------------------------------------
|    FUNCTION: change projection_e0()
|     PURPOSE: Change projection false easting from pulldown menu
| DESCRIPTION: Change e0 and recalculate x and y
|     RETURNS: nothing
|     HISTORY: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
static void change_projection_e0()
{
  edit_data(-1,-1,"False Easting","%.0lf",&projection.e0);
  geoutm(ship.lat,ship.lon,&ship.x,&ship.y);
  gen_posstr(posmode);
}
/*-------------------------------------------------------------------
|    FUNCTION: change projection_n0()
|     PURPOSE: Change projection false northing from pulldown menu
| DESCRIPTION: Change n0 and recalculate x and y
|     RETURNS: nothing
|     HISTORY: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
static void change_projection_n0()
{
  edit_data(-1,-1,"False Northing","%.0lf",&projection.n0);
  geoutm(ship.lat,ship.lon,&ship.x,&ship.y);
  gen_posstr(posmode);
}
/*-------------------------------------------------------------------
|    FUNCTION: change projection_k0()
|     PURPOSE: Change projection scale factor on central meridian from pulldown menu
| DESCRIPTION: Change k0 and recalculate x and y
|     RETURNS: nothing
|     HISTORY: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
static void change_projection_k0()
{
  edit_data(-1,-1,"Scale factor","%.6lf",&projection.k0);
  geoutm(ship.lat,ship.lon,&ship.x,&ship.y);
  gen_posstr(posmode);
}
/*-------------------------------------------------------------------
|    FUNCTION: change projection_lat0()
|     PURPOSE: Change projection lattitude of origin from pulldown menu
| DESCRIPTION: Change lat0 and recalculate x and y
|     RETURNS: nothing
|     HISTORY: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
static void change_projection_lat0()
{
  edit_data(-1,-1,"Lattitude of Origin","%.0lf",&projection.lat0);
  geoutm(ship.lat,ship.lon,&ship.x,&ship.y);
  gen_posstr(posmode);
}
/*-------------------------------------------------------------------
|    FUNCTION: change projection_lon0()
|     PURPOSE: Change projection longitude of origin from pulldown menu
| DESCRIPTION: Change lon0 and recalculate x and y
|     RETURNS: nothing
|     HISTORY: 911119 V0.1 - Initial version
---------------------------------------------------------------------*/
static void change_projection_lon0()
{
  edit_data(-1,-1,"Longitude of origin","%.0lf",&projection.lon0);
  geoutm(ship.lat,ship.lon,&ship.x,&ship.y);
  gen_posstr(posmode);
}

/*-------------------------------------------------------------------
|    FUNCTION: toggle_profile()
|     PURPOSE: Toggle profile status from pulldown menu
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 911118 V0.1 - Initial version
---------------------------------------------------------------------*/
static void toggle_profile()
{
  if (profile_mode == ON)
    profile_mode = OFF;
  else
    profile_mode = ON;

  /*-----------------------------------
    Perform initializations for line 0
  ------------------------------------*/
  if (profile_mode == ON)
    calc_profile_line(&profile,0);
}

/*-------------------------------------------------------------------
|    FUNCTION: gen_posstr()
|     PURPOSE: Generate ships postion strings
| DESCRIPTION: -
|     RETURNS: nothing
|     HISTORY: 910819 V0.1 - Initial version
---------------------------------------------------------------------*/
void gen_posstr(int posmode)
{
  switch (posmode)
  {
    case  LATLON_DD:
      sprintf(ship.xpos_str,"%8.4lf %c",
              ship.lon,
              ship.lon > 0.0 ? 'E':'W');
      sprintf(ship.ypos_str,"%08.4lf %c",
              ship.lat,
              ship.lat > 0.0 ? 'N':'S');
      break;

    case LATLON_DDMM:
      sprintf(ship.xpos_str,"%02d %6.3lf %c",
              ship.lon_deg, ship.lon_mm + ship.lon_ss/60.0,
              ship.lon > 0.0 ? 'E':'W');
      sprintf(ship.ypos_str,"%02d %6.3lf %c",
            ship.lat_deg, ship.lat_mm + ship.lat_ss/60.0,
            ship.lat > 0.0 ? 'N':'S');
      break;

    case LATLON_DDMMSS:
      sprintf(ship.xpos_str,"%02d %02d %5.2lf %c",
              ship.lon_deg,ship.lon_mm,ship.lon_ss,
              ship.lon > 0.0 ? 'E':'W');
      sprintf(ship.ypos_str,"%02d %02d %5.2lf %c",
              ship.lat_deg,ship.lat_mm,ship.lat_ss,
              ship.lat > 0.0 ? 'N':'S');
      break;

    default:  /* UTM */
      sprintf(ship.xpos_str,"%10.2lf",ship.x);
      sprintf(ship.ypos_str,"%10.2lf",ship.y);
      break;
  }

}

/*-------------------------------------------------------------------
|    FUNCTION: get_ship_x()
|     PURPOSE: Get ships x coordinate
| DESCRIPTION: Get coordinate in UTM or longitude.
|              If in longitude, then calculate X coordinate
|     RETURNS: nothing
|     HISTORY: 910814 V0.1 - Initial version
---------------------------------------------------------------------*/
static void get_ship_x()
{
  char datastr[40];
  int  mode = UTM_X;

  gen_posstr(posmode);
  strcpy(datastr,ship.xpos_str);
  editstr(-1,-1,20,"SHIP X / LON",datastr,_DRAW);
  posmode = get_pos(&mode,datastr,&ship.lat,&ship.lon,&ship.x,&ship.y);
  gen_posstr(posmode);
  calc_position(1, 0.0);
}

/*-------------------------------------------------------------------
|    FUNCTION: get_ship_y()
|     PURPOSE: Get ships y coordinate
| DESCRIPTION: Get coordinate in UTM or lattitude.
|              If in lattitude, then calculate Y coordinate
|     RETURNS: nothing
|     HISTORY: 910814 V0.1 - Initial version
---------------------------------------------------------------------*/
static void get_ship_y()
{
  char datastr[40];
  int mode = UTM_Y;

  gen_posstr(posmode);
  strcpy(datastr,ship.ypos_str);
  editstr(-1,-1,20,"SHIP Y / LAT",datastr,_DRAW);
  posmode = get_pos(&mode,datastr,&ship.lat,&ship.lon,&ship.x,&ship.y);
  gen_posstr(posmode);
  calc_position(1, 0.0);
}

/*-------------------------------------------------------------------
|    FUNCTION: calc_next_update()
|     PURPOSE: calculate next update time
| DESCRIPTION: -
|     RETURNS: time (long)
|     HISTORY: 910903 V0.1 - Initial version
---------------------------------------------------------------------*/
long calc_next_update(long time, double dt)
{
  long t;

  t = time + (long) (dt*1000.0);
  return(t);
}

/*-------------------------------------------------------------------
|   FUNCTION: process_input()
|    PURPOSE: Process serial input
| DESRIPTION: Do something with incoming data:
|             display an incoming line or word.
|    RETURNS: Nothing
|    VERSION: 901104 V0.3
---------------------------------------------------------------------*/
static void process_input(NAV_SYSTEM *nav_system)
{
  int error = 0;
  int channel;
  int slot;
  char msg[256];
  char ext_msg[256];

  slot = nav_system->slot;
  channel = nav_system->channel;
  get_slst(slot,&error);

  if (!error)
  {
    msg[0] = '\0';
    get_slot_data(slot,msg,120,&error);
    if (!error)
    {
      sprintf(ext_msg,"CH:%1d/SLOT:%1d ",channel,slot);
      strcat(ext_msg,msg);

      disp_input(msg);

      /*-------------------------------------------------------------
        If the navigation system should process the incoming data,
        call the appropiate function here.
      --------------------------------------------------------------*/
      if (nav_system->input_func)
      {
        (*(nav_system->input_func))(msg+12);   /* Point after timestamp */
      }

      if (printer_out)
      {
        fprintf(stdprn,"%s",ext_msg);
        fflush(stdprn);
      }

      msg[0] = '\0';       /* clear message */
      ext_msg[0] = '\0';
    }
  }


}

/*-------------------------------------------------------------------
|   FUNCTION: save_all()
|    PURPOSE: Save all system data
| DESRIPTION: Save system information in given filename.
|    RETURNS: nothing
|    HISTORY: 910501 V0.1 - Initial version
|             911120 V0.2 - Does only update choosen system file.
---------------------------------------------------------------------*/
static void save_all()
{
  save_system(systemfile_name);
}

/*-------------------------------------------------------------------
|   FUNCTION: save_all_as()
|    PURPOSE: Save all system data
| DESRIPTION: Save system information in user defined filename.
|    RETURNS: nothing
|    HISTORY: 910501 V0.1 - Initial version
|             911120 V0.2 - Does only update choosen system file.
---------------------------------------------------------------------*/
static void save_all_as()
{
  int ret;

  ret = editstr(-1,-1,20,"system file name",systemfile_name,_DRAW);
  if (ret == ENTER)
    save_system(systemfile_name);
}

/*-------------------------------------------------------------------
|   FUNCTION: Load_all()
|    PURPOSE: Load system data for specific system
| DESRIPTION: Calls load_system() with actual filename
|    RETURNS: nothing
|    HISTORY: 910501 V0.1 - Initial version
---------------------------------------------------------------------*/
static void load_all()
{
  char filename[15];

  filename[0] = '\0';

  if (get_dir("*.sys",filename) != NULL)
  {
    load_system(filename);
    load_chain(chain_name);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: simulate()
|    PURPOSE: simulate navigation system
| DESRIPTION: start simulation (after editing parameters or loading
|             station data etc);
|    RETURNS: Nothing
|    VERSION: 901104 V0.9
---------------------------------------------------------------------*/
void simulate()
{
  int i;
  long lasttime = 0;
  long disp_updatetime = 0;

  new_start = TRUE;         /* Indicates (re-)start of simulation */
  calc_position(1, 0.0);    /* Initialize static variables in calc.c */
  gen_posstr(posmode);

  /*---------------------------------
  | Initialize systems
  ----------------------------------*/
  realtime = clock();
  for (i=0 ; i<MAX_CHANNELS ; i++)
  {
    if (systeem[i] != NULL)
    {
      nav_system = systeem[i];
      if (nav_system->init_func)
        (*(nav_system->init_func))();
      nav_system->update_time = clock();
      nav_system->eventnr = 0;
      calc_position(0, nav_system->dt);
      check_pattern();
      calc_patterns();
      if (nav_system->output_func)
        (*(nav_system->output_func))();
    }
  }

  /*-----------------
  | Get actual time
  ------------------*/
  time(&tnow);
  tmnow = localtime(&tnow);

  /*---------------------------
    Initialize time variables
  ----------------------------*/
  realtime    = clock();
  lasttime    = realtime;

  calc_geog_const();

  /*-------------------------------
  | Initialize all windows
  ---------------------------------*/
  clear_all_windows();
  display_all();

  /*---------------------
    MAIN SIMULATION LOOP
  ----------------------*/
  while (TRUE)
  {
    show_time();

    /*------------------------
      Calculate new position
    -------------------------*/
    calc_position(1, (double) ((realtime-lasttime)/1000.0) );
    lasttime = realtime;
    gen_posstr(posmode);

    /*-----------------
    | Format messages
    ------------------*/
    for (i=0 ; i<MAX_CHANNELS ; i++)
    {
      /*---------------------------------------
        Process keys which can be hit
      ---------------------------------------*/
      if (ESC_pressed()) return;
      if (ALT_pressed()) return;
      if (kbhit()) process_key();

      if (systeem[i] != NULL)
      {
        nav_system = systeem[i];
        process_input(nav_system);
        if (nav_system->out_msg[0] == '\0')
        {
          /*--------------------------------------------
            Last message was send; generate new message
          ----------------------------------------------*/
          calc_position(0, nav_system->dt);
          calc_patterns();
          if (nav_system->output_func)
            (*(nav_system->output_func))();
        }
      }
    }


    /*----------------------------------------------
      Check if a system has to perform an output.
      If so, do it.
    -----------------------------------------------*/
    for (i=0 ; i<MAX_CHANNELS ; i++)
    {
      if (ESC_pressed())
      {
        getch();
        return;
      }
      if (ALT_pressed())
        return;
      if (kbhit()) process_key();         /* Process keys hit */

      if (systeem[i] != NULL)
      {
        nav_system= systeem[i];
        process_input(nav_system);
        if (realtime > nav_system->update_time)
        {
          nav_system->update_time =
            calc_next_update(nav_system->update_time,nav_system->dt);
          disp_output(nav_system->out_msg);
          disp_status("Output of system %s",nav_system->name);
          msg_output(nav_system->out_msg);
          nav_system->eventnr = (nav_system->eventnr + 1) % 1000;
        }
      }
    }

    /*-----------------
      Get actual time
    ------------------*/
    realtime = clock();                 /* Get new realtime */
    time(&tnow);
    tmnow = localtime(&tnow);

    /*----------------
      Display items
    -----------------*/
    if (realtime > disp_updatetime)
    {
      display_all();
      disp_updatetime = realtime + 500;  /* 500 ms */
    }

    /*-----------------------
      Reset (re-)start flag
    -------------------------*/
    if (new_start)
      new_start = FALSE;

    if (ESC_pressed())
    {
      getch();
      break;
    }
    if (ALT_pressed())
      break;
  }
}


/*-----------------------------------------------------------------------
     main program
------------------------------------------------------------------------*/
main()
{
  extern char *default_subject;
  int c;

  default_subject = "navsim";

  initialize();
  pulldown_bar(heads);

  gen_posstr(posmode);

  open_disp_window(SHIP_WINDOW);
  open_disp_window(POSITION_WINDOW);
  open_disp_window(INPUT_WINDOW);
  open_disp_window(OUTPUT_WINDOW);

  /*------------------------
    Display initial values
  --------------------------*/
  time(&tnow);
  tmnow = localtime(&tnow);
  realtime    = clock();
  display(DISP_TIME);

  calc_patterns();
  clear_all_windows();
  display_all();

  simulate();

  while (TRUE)
  {
    show_time();
    if (ESC_pressed())
    {
      getch();
      exit_prog();
    }
    else if (ALT_pressed())
    {
      if (_bios_keybrd(_KEYBRD_READY))
      {
        c = waitkey();
        pulldown(heads,c,":navsim");
        calc_patterns();
      }
    }
  }

  /*--------------------
    Close the windows
  --------------------*/
  close_disp_window(SHIP_WINDOW);
  close_disp_window(POSITION_WINDOW);
  close_disp_window(OUTPUT_WINDOW);
  close_disp_window(INPUT_WINDOW);

  /*---------------------
    Exit this programm
  ----------------------*/
  exit(0);
  return(0);

}
