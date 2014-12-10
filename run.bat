@rem platform.Windows32.exe "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/pca10001/blinky_example/gcc/_build/blinky_gcc_xxaa.out"

set postfix=%1
IF "%1" == "" set postfix=hex

@rem platform.Windows32.exe "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/pca10001/blinky_example/gcc/_build/blinky_gcc_xxaa.%postfix%"

platform.Windows32.exe "../../../Documents/master thesis/hi/hi2/Src/nrf51822/Board/pca10001/s110/ble_app_beacon/gcc/b.%postfix%"