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
echo done
