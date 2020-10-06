REM Execute DOC++ on the PegasusManual
REM ECHO OFF
REM The following Did not work for some reason and I don't have time to look further.
REM if %@index[%path%,"DOC++"] != -1 GOTO DONE

REM TODO - test for existance of PEGASUS_ROOT and PEGASUS_HOME

if defined DOCPLUSPLUS GOTO DONE
set DOCPLUSPLUS=f:\"Program Files"\DOC++\DOC++347
set SWIG=f:\packages\swig13aNT
set PEGASUSMANUAL_HOME=%PEGASUS_HOME%/manualhtml

REM echo This is what the index shows after %@index[%path%,"DOC++"]
set path=%path%;e:\cygnus\bin
set path=%path%;%DOCPLUSPLUS%
set path=%path%;%SWIG%
:DONE

IF NOT EXIST %PEGASUSMANUAL_HOME%\pegasus.html GOTO BYPASSDELETE


	ECHO del %PEGASUSMANUAL_HOME%\*.html /Y /Q
	ECHO del %PEGASUSMANUAL_HOME%\*.html /Y /Q
	pause
	del %PEGASUSMANUAL_HOME%\*.html /Y /Q
	del %PEGASUSMANUAL_HOME%\*.html /Y /Q
	DIR

:BYPASSDELETE
REM Execute
CD %PEGASUS_ROOT%\doc\DEVMANUAL
IF NOT Exist pegasusdoc.dxx GOTO ERROR
docxx --dir %PEGASUSMANUAL_HOME%  --verbose -H pegasusdoc.dxx
mu copy *.gif %PEGASUSMANUAL_HOME%
REM -a parameters says do things with tables
CD %PEGASUS_ROOT%
CD ..\
Quit
ERROR:
ECHO You are not in the directory with the Pegasus Documentation source files.
ECHO could not find pegasusdoc.dxx
