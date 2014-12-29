make -f ./Makefile.platform SRC=platform.c NOVLNV=1 %*
make -C pse NOVLNV=1 %*
make -C pse_rtc NOVLNV=1 %*
make -C pse_radio NOVLNV=1 %*
