@echo OFF
echo "git version"

set gitclean=0
set versionfile=../Source/Version.h
for /F %%i in ('git rev-parse --short HEAD') do ( set commitid=%%i)
for /F %%j in ( 'git status ^|findstr clean' ) do (set gitclean=1)   

echo //this is auto generate by the build process, don't modify >%versionfile%
echo #define BUILD_NUMBER (0x%commitid%) >>%versionfile%
echo #define GIT_CLEAN (%gitclean%) >>%versionfile%


echo "generate index.h"

cd /d %cd%\..\out
set index_h=../Source/Thread/Net/Web/index.h
set index_html=../Source/Thread/Net/Web/index.html
binsrc.exe %index_html% %index_h% index