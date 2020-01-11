cd /d %cd%\..\out
copy %cd%\..\Debug\fs_station.out %cd%\fs_station.out
out2rprc.exe fs_station.out fs_station.bin
HexAIS_OMAP-L138.exe -ini NandFlash.ini -o fs_station.ais fs_station.out