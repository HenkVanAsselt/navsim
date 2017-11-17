REM --- Delete temporary files

call cleanup

del tmp*.*
del *.tmp
del test.*
del *.bak
del b:\navsim\test*.*
del b:\navsim\tmp*.*
del b:\navsim\*.tmp
del b:\navsim\*.bak

REM --- Update diskette

copy /u *.c *.h b:\navsim
copy /u *.bat *.lnk *.mak b:\navsim
copy /u *.ndx *.hlp b:\navsim
copy /u *.ndx *.hlp b:\
copy /u *.doc b:\navsim
copy /u navsim.exe b:

REM --- Update HD

copy /u b:\navsim\*.c .
copy /u b:\navsim\*.h .
copy /u b:\navsim\*.bat .
copy /u b:\navsim\*.lnk .
copy /u b:\navsim\*.mak .
copy /u b:\navsim\*.ndx .
copy /u b:\navsim\*.hlp .
copy /u b:\navsim\*.doc .
