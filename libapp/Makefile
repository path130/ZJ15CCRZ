CSTOOL_PREFIX = /opt/arm_toolchain/external-toolchain/bin/arm-none-linux-gnueabi-
TARGET = $(notdir $(CURDIR))



QUAN_ZHI_LIB_DIR= /opt/quan_zhi_x3/quanzhi_lib
QUAN_ZHI_OSD_LIB_DIR= /opt/quan_zhi_x3/quan_zhi_osd_lib



JPEGLIB_INCLUDE=-I $(QUAN_ZHI_OSD_LIB_DIR)/jpeg
JPEGLIB_LIB=$(QUAN_ZHI_OSD_LIB_DIR)/jpeg



JRTPLIB_INCLUDE=./../jrtp #temporately use C lib
JRTPLIB_LIB=./../jrtp #temporately use C lib





APP_SRCDIR = ./..
UI_SRCDIR 	    = ./../ui
OSD_SRCDIR 	    = ./../osd
SPI_SRCDIR      = ./../spi
NET_SRCDIR  	= ./../net
UART_SRCDIR  	= ./../uart
TEXT_SRCDIR 	= ./../ui/text
VIDEO_SRCDIR 	= ./../video
AUDIO_SRCDIR 	= ./../audio
UTILS_SRCDIR    = ./../utils
SPEECH_SRCDIR   = ./../speech
PUBLIC_SRCDIR   = ./../public
DATABASE_SRCDIR = ./../database
SQL_UILIB = ../sqlui
AES_SRCDIR = ./../aes



SUB_SRCDIR :=./
SRC_DIR:=./


C_FLAGS += -Wall -O2 -D_LINUX

C_FLAGS += -I$(APP_SRCDIR) 
C_FLAGS += -I$(UI_SRCDIR)  -I$(NET_SRCDIR) -I$(UART_SRCDIR) -I$(TEXT_SRCDIR) -I$(VIDEO_SRCDIR) -I$(AUDIO_SRCDIR) -I$(SPEECH_SRCDIR) -I$(UTILS_SRCDIR) -I$(PUBLIC_SRCDIR) -I$(DATABASE_SRCDIR) -I$(OSD_SRCDIR) -I$(AES_SRCDIR) -I$(SPI_SRCDIR)
C_FLAGS += -I$(JRTPLIB_INCLUDE) -I$(OSD_SRCDIR) -I$(JPEGLIB_INCLUDE) 
C_FLAGS += -I $(QUAN_ZHI_LIB_DIR)/include
C_FLAGS += -I ./../code/include -I ./../code/dpgraphic -I ./../code/ui/cctrl -I ./../code/phoneapp -I ./../code/dpsession -I ./../code -I /opt/quan_zhi_x3/quan_zhi_speech


LD_FLAGS += -L$(JRTPLIB_LIB) -L$(JPEGLIB_LIB)
LD_FLAGS += -lpthread -lasound -ljthread -ljrtp -lfreetype -lz -ljpeg -lpng -lsqlite3 -L ../lib -lasound -lAudioCore -lAudioOut -lMP3dec -ljpeg -lpng16 -lz -lnetcfg -lSilkCodec -lOnVif -lDPCall
LD_FLAGS += -L $(QUAN_ZHI_LIB_DIR)/lib

#ifeq ($(TARGET), ZJ15CCBHIP)  
#COMPILE.c = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(C_FLAGS) -DFRONT_DOOR -c
#else
#ifeq ($(TARGET), ZJ15CCHIP_EMC)
#COMPILE.c = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(C_FLAGS) -DEMC_TEST -c
#else 
#COMPILE.c = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(C_FLAGS) $(CFLAGS)  -c
#endif
#endif  

COMPILE.c = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(C_FLAGS) $(CFLAGS) -c
LINK.c = $(VERBOSE) $(CSTOOL_PREFIX)gcc $(LD_FLAGS)
AR.c = $(VERBOSE) $(CSTOOL_PREFIX)ar

SOURCES = $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
HEADERS = $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.h))

OBJFILES = $(SOURCES:%.c=%.o)
#EXEC_BIN_DIR = /mnt/hgfs/ubuntu_work


.PHONY: clean install 

all: libapp.a

libapp.a:	$(OBJFILES)
	$(CSTOOL_PREFIX)ar rc ./libapp.a $^

install:	$(if $(wildcard $(TARGET)), install_$(TARGET))

install_$(TARGET):
	@echo
	./makeappbin APP $(TARGET) NC01 11BH GYT
	$(VERBOSE) cp $(TARGET).bin $(EXEC_BIN_DIR)/$(TARGET).bin
	@echo Installed $(TARGET) binaries to $(EXEC_BIN_DIR)..

$(TARGET):	$(OBJFILES)
	@echo Linking $@ from $^..
	$(LINK.c) -o $@ $^ 

$(OBJFILES):	%.o: %.c $(HEADERS)
	@echo Compiling $@ from $<..
	$(COMPILE.c) -o $@ $<	

$(XDC_LFILE) $(XDC_CFLAGS):	$(XDC_CFGFILE)
	@echo
	@echo ======== Building $(TARGET) ========
	@echo Configuring application using $<
	@echo
	$(VERBOSE) XDCPATH="$(XDC_PATH)" $(CONFIGURO) -o $(XDC_CFG) -t $(XDC_TARGET) -p $(XDC_PLATFORM) -b $(CONFIG_BLD) $(XDC_CFGFILE)

clean:
	@echo Removing generated files..
	$(VERBOSE) -$(RM) -rf $(XDC_CFG) $(OBJFILES) $(TARGET) *~ *.d .dep libapp.a

