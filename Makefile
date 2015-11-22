PC_CC=g++
PC_CFLAGS=-I/usr/include
PC_LFLAGS=-L/usr/lib

BFIN_CC=bfin-uclinux-g++
BFIN_CFLAGS=-I/home/allard/buildroot/output/staging/usr/include
BFIN_LFLAGS=-L/home/allard/buildroot/output/staging/usr/lib

eth002-ctrl-pc: eth002-ctrl.cpp
		$(PC_CC) $(PC_CFLAGS) -o bin/eth002-ctrl-pc eth002-ctrl.cpp $(PC_LFLAGS)

eth002-ctrl-bfin: eth002-ctrl.cpp
		$(BFIN_CC) $(BFIN_CFLAGS) -o bin/eth002-ctrl-bfin eth002-ctrl.cpp $(BFIN_LFLAGS)
