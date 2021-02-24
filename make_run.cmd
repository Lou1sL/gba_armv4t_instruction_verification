@echo off
:st
taskkill /F /IM mGBA.exe
docker exec -it mystifying_newton make -C /home/gba_armv4t_instruction_verification
START mGBA.exe gba_armv4t_instruction_verification.gba
pause
goto st