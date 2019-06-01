cd /d %cd%\..\out
copy %cd%\..\Debug\fs_station.out %cd%\fs_station.out
HexAIS_OMAP-L138.exe -ini NandFlash.ini -o fs_station.ais fs_station.out