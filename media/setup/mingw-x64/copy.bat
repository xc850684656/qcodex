md "%cd%\packages"
xcopy "%cd%\..\..\build-mTranscode-Desktop_Qt_6_5_1_MinGW_64_bit-Release\release\mTranscode.dll" "%cd%\packages" /Y
:: 依赖的dll文件
set STD_DIR=C:\Qt\6.5.1\mingw_64\bin
xcopy "%STD_DIR%\libstdc++-6.dll" "%cd%\packages" /Y
xcopy "%STD_DIR%\libgcc_s_seh-1.dll" "%cd%\packages" /Y

pause