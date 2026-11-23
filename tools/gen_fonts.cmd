@echo off
rem ---------------------------------------------------------------------------
rem Regenerate the vic font headers from the TTF fonts in src\font\ttf
rem 10 and 12 pixel fonts with 2 bit, 16 and 24 pixel fonts with 4 bit per pixel
rem Needs Python 3 with freetype-py (pip install freetype-py)
rem ---------------------------------------------------------------------------
setlocal
set "ROOT=%~dp0.."
set "GEN=python "%ROOT%\tools\fontgen.py""

rem Montserrat: ASCII + Latin-1
set "TTF=%ROOT%\src\font\ttf\Montserrat-Regular.ttf"
%GEN% "%TTF%" --size 10 --bpp 2 || exit /b 1
%GEN% "%TTF%" --size 12 --bpp 2 || exit /b 1
%GEN% "%TTF%" --size 16 --bpp 4 || exit /b 1
%GEN% "%TTF%" --size 24 --bpp 4 || exit /b 1

rem Michroma: ASCII + Latin-1
set "TTF=%ROOT%\src\font\ttf\Michroma-Regular.ttf"
%GEN% "%TTF%" --size 10 --bpp 2 || exit /b 1
%GEN% "%TTF%" --size 12 --bpp 2 || exit /b 1
%GEN% "%TTF%" --size 16 --bpp 4 || exit /b 1
%GEN% "%TTF%" --size 24 --bpp 4 || exit /b 1

rem DSEG7 / DSEG14 Classic (LCD segment display look): ASCII + degree sign, 16, 24 and 32 pixel with 4 bit
for %%F in (7 14) do for %%S in (16 24 32) do (
  %GEN% "%ROOT%\src\font\ttf\DSEG%%FClassic-Regular.ttf" --size %%S --bpp 4 --ranges 20-7E,B0 --name DSEG%%F_%%S_aa4 || exit /b 1
)

rem vector (outline) fonts, scalable: one header for all sizes
set "VEC=python "%ROOT%\tools\vecfontgen.py""
%VEC% "%ROOT%\src\font\ttf\Montserrat-Regular.ttf" || exit /b 1
%VEC% "%ROOT%\src\font\ttf\Michroma-Regular.ttf" || exit /b 1
rem Doto: dot matrix (LED display look), instance of the variable font with round dots
%VEC% "%ROOT%\src\font\ttf\Doto-Variable.ttf" --axes ROND=100,wght=700 || exit /b 1
rem DSEG7 / DSEG14 Classic (LCD segment display look): ASCII + degree sign
%VEC% "%ROOT%\src\font\ttf\DSEG7Classic-Regular.ttf" --ranges 20-7E,B0 --name DSEG7_vec || exit /b 1
%VEC% "%ROOT%\src\font\ttf\DSEG14Classic-Regular.ttf" --ranges 20-7E,B0 --name DSEG14_vec || exit /b 1
echo done
