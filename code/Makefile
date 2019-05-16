CC = /allwinner/arm_toolchain/external-toolchain/bin/arm-none-linux-gnueabi-gcc
SRC = codec.c dpvideo.c
CFLAG = -I ./include -I ./dpgraphic -I ./ui/cctrl -I ./phoneapp -I ./dpsession -I .
LFLAG = -lpthread
IPATH = /workspace/X3/nfsroot/X3_PUBLIC/opt/app
all:
	${CC} ${SRC} ${CFLAG} ${LFLAG}
	cp a.out ${IPATH}
