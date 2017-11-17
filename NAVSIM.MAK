######################################################
# Makefile for NAVSIM with MCS51                     #
######################################################

### Set inference rules
#########################

.c.obj:
    cl /AL /L /W3 /c /Zi $*.c

.asm.obj:
    masm /ML /W2 /Zi /Zd /L $*.asm,,,;

### Set dependencies
######################

NR53.OBJ: $*.C

CALC.OBJ: $*.C

UTMCON.OBJ: $*.C

DISP.OBJ: $*.C

INIT.OBJ: $*.C

FILIO.OBJ: $*.C

KEYS.OBJ: $*.C

TRISPOND.OBJ: $*.C

DECCA.OBJ: $*.C

ECHOTRAC.OBJ: $*.C

NMEA.OBJ: $*.C

FALCON.OBJ: $*.C

MICROFIX.OBJ: $*.C

HYPERFIX.OBJ: $*.C

SYLEDIS.OBJ: $*.C

MENU.OBJ: $*.C

RADARFIX.OBJ: $*.C

ART.OBJ: $*.C

ART4.OBJ: $*.C

POLAR.OBJ: $*.C

UCM40.OBJ: $*.C

DESO25.OBJ: $*.C

PULSE8.OBJ: $*.C

NAVSIM.OBJ: $*.C navsim.h

TRIMBLE.OBJ: $*.C

GETPOS.OBJ: $*.C

UTIL.OBJ: $*.C

STATIONS.OBJ: $*.C

PROFILE.OBJ: $*.C

navsim.exe: navsim.obj menu.obj util.obj getpos.obj calc.obj utmcon.obj \
 init.obj disp.obj keys.obj filio.obj nr53.obj trispond.obj art.obj \
 art4.obj polar.obj ucm40.obj deso25.obj pulse8.obj trimble.obj \
 microfix.obj hyperfix.obj syledis.obj radarfix.obj nmea.obj \
 echotrac.obj decca.obj falcon.obj stations.obj profile.obj \
 \lib\window.lib \lib\serial9.lib
 link @navsim.lnk

navsim.ndx: navsim.hlp
  genindex navsim.hlp
