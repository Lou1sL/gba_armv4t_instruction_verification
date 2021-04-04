@echo off
docker run --rm -it -v %cd%:/home/gba_armv4t_instruction_verification -w="/home/gba_armv4t_instruction_verification" devkitpro/devkitarm make clean