@echo off
:st
taskkill /F /IM mGBA.exe
docker run --rm -it -v %cd%:/home/gba_armv4t_instruction_verification -w="/home/gba_armv4t_instruction_verification" devkitpro/devkitarm make
START mGBA.exe gba_armv4t_instruction_verification.gba
pause
goto st