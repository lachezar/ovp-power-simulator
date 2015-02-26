make -C "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/nrf6310/timer_example/gcc" release
@rem nrfjprog.exe --reset --program "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/nrf6310/timer_example/gcc/_build/timer_example_gcc_xxaa.hex" 
@rem "d:\MentorGraphics\Sourcery_CodeBench_Lite_for_ARM_EABI\bin\arm-none-eabi-objdump.exe" -D -l --target ihex -Mforce-thumb -marm "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/nrf6310/timer_example/gcc/_build/timer_example_gcc_xxaa.hex" > ./timer.dis.asm 
"d:\MentorGraphics\Sourcery_CodeBench_Lite_for_ARM_EABI\bin\arm-none-eabi-objdump.exe" -D -l --target ihex -Mforce-thumb -marm "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/nrf6310/timer_example/gcc/_build/timer_example_gcc_xxaa.hex" > ./application.asm 
"c:\Program Files\GNU Tools ARM Embedded\4.7 2013q1\arm-none-eabi\bin\objdump.exe"  -D -l --target ihex -Mforce-thumb -marm "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/nrf6310/timer_example/gcc/_build/timer_example_gcc_xxaa.hex" > ./application2.asm 
python generate_cycle_map.py ./application.asm
