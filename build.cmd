@echo off

docker start mystifying_newton
docker exec -it mystifying_newton make clean -C /home/gba_armv4t_instruction_verification
docker exec -it mystifying_newton make -C /home/gba_armv4t_instruction_verification