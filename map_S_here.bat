:: This script maps the S: drive to wherever the script happens to be.
:: Place the script on the path you want to map and run it.

::Remove any old mappings for S:
SUBST S: /D
NET USE S: /delete

:: first try to map with NET USE
NET USE S: "%~dp0."

:: iff that fails then try to map with SUBST
IF NOT ERRORLEVEL 1 GOTO:EOF
SUBST S: "%~dp0."
