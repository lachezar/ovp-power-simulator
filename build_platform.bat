make -f ./Makefile.platform SRC=platform.c NOVLNV=1 %*
make -C pse_timer NOVLNV=1 %*
make -C pse_rtc NOVLNV=1 %*
make -C pse_radio NOVLNV=1 %*
make -C pse_spi NOVLNV=1 %*
