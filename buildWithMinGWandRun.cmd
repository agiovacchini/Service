@echo off
set path=%path%;d:\Listati\Siti\PromoCalculator\PromoCalculator.deploy\;
del PromoCalculator.exe
rem make -f makefile.win clean > buildLog.log 2>&1
make -f makefile.win > buildLog.log 2>&1
type buildLog.log
if not exist PromoCalculator.exe goto errore

goto ok

:errore
echo ====================
echo   Errore di build!
echo ====================
goto fine

:ok
echo ====================
echo   Build ok, eseguo
echo ====================

:Fine
rem PromoCalculator.exe d:\Listati\Siti\PromoCalculator\PromoCalculator.deploy\
