CSTOOL_PREFIX = /opt/arm_toolchain/external-toolchain/bin/arm-none-linux-gnueabi-
TARGET = $(notdir $(CURDIR))



QUAN_ZHI_LIB_DIR= /opt/quan_zhi_x3/quanzhi_lib
QUAN_ZHI_OSD_LIB_DIR= /opt/quan_zhi_x3/quan_zhi_osd_lib



#JPEGLIB_INCLUDE=-I $(QUAN_ZHI_OSD_LIB_DIR)/jpeg
#JPEGLIB_LIB=$(QUAN_ZHI_OSD_LIB_DIR)/jpeg



JRTPLIB_INCLUDE=./jrtp #temporately use C lib
JRTPLIB_LIB=./jrtp #temporately use C lib





APP_SRCDIR = .
UI_SRCDIR 	    = ./ui
OSD_SRCDIR 	    = ./osd
KEY_SRCDIR	=./key
NET_SRCDIR  	= ./net
UART_SRCDIR  	= ./uart
TEXT_SRCDIR 	= ./ui/text
VIDEO_SRCDIR 	= ./video
AUDIO_SRCDIR 	= ./audio
UTILS_SRCDIR    = ./utils
SPEECH_SRCDIR   = ./speech
PUBLIC_SRCDIR   = ./public
DATABASE_SRCDIR = ./database
DEVICE_SRCDIR = ./device
SQL_UILIB = sqlui
AES_SRCDIR = ./aes
VIDEO_NEW=./video_new
FINGER_SRCDIR = ./finger
JSON_SRCDIR		= ./json
LIBAPP_SRCDIR  = ./libapp


SUB_SRCDIR := $(UART_SRCDIR) $(UI_SRCDIR) $(KEY_SRCDIR) $(NET_SRCDIR) $(TEXT_SRCDIR) $(VIDEO_SRCDIR) $(AUDIO_SRCDIR) $(SPEECH_SRCDIR) $(UTILS_SRCDIR) $(PUBLIC_SRCDIR)  $(DATABASE_SRCDIR) $(OSD_SRCDIR) $(VIDEO_NEW) $(DEVICE_SRCDIR) $(FINGER_SRCDIR) $(JSON_SRCDIR)
SRC_DIR +=$(APP_SRCDIR) $(SUB_SRCDIR) $(AES_SRCDIR) $(LIBAPP_SRCDIR)


C_FLAGS += -Wall -O2 -D_LINUX

C_FLAGS += -I$(APP_SRCDIR) 
C_FLAGS += -I$(UI_SRCDIR) -I$(KEY_SRCDIR) -I$(NET_SRCDIR) -I$(UART_SRCDIR) -I$(TEXT_SRCDIR) -I$(VIDEO_SRCDIR) -I$(AUDIO_SRCDIR) -I$(SPEECH_SRCDIR)
C_FLAGS += -I$(UTILS_SRCDIR) -I$(PUBLIC_SRCDIR) -I$(DATABASE_SRCDIR) -I$(OSD_SRCDIR) -I$(AES_SRCDIR) -I$(VIDEO_NEW)  -I$(DEVICE_SRCDIR) -I$(FINGER_SRCDIR)
C_FLAGS += -I$(OSD_SRCDIR) -I$(JSON_SRCDIR)
# -I$(JPEGLIB_INCLUDE)
C_FLAGS += -I $(QUAN_ZHI_LIB_DIR)/include
C_FLAGS += -I ./code/include -I ./code/dpgraphic -I ./code/ui/cctrl -I ./code/phoneapp -I ./code/dpsession -I ./code -I /opt/quan_zhi_x3/quan_zhi_speech


#LD_FLAGS += -L$(JPEGLIB_LIB)
LD_FLAGS += -lpthread -lasound -ljthread -ljrtp -lfreetype -lz -lpng -lsqlite3
LD_FLAGS += -L $(QUAN_ZHI_LIB_DIR)/lib -lasound -lAudioCore -lAudioOut -lMP3dec -lpng16 -lz -lnetcfg -lSilkCodec -lOnVif -lDPCall

include ./face/face.mk

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
EXEC_BIN_DIR = /mnt/hgfs/tftpboot/targetfs/Z15CCRZ


.PHONY: clean install 

all: $(TARGET)

install:	$(if $(wildcard $(TARGET)), install_$(TARGET))

install_$(TARGET):
	@echo
	./makeappbin APP $(TARGET) P006 15CH WRM
	$(VERBOSE) cp $(TARGET).bin $(EXEC_BIN_DIR)/$(TARGET).bin
	@echo Installed $(TARGET) binaries to $(EXEC_BIN_DIR)..

package:
	cd ./upgrade_package && ./makepack.sh $(TARGET)

#$(TARGET):	$(OBJFILES) ./libapp/libapp.a
$(TARGET):	$(OBJFILES)
	@echo Linking $@ from $^..
	@$(LINK.c) -o $@ $^ -L${DATABASE_SRCDIR} -l$(SQL_UILIB) ajb_jrtpc_lib.a libquan_zhi_speech.a face/libfacedetector.a face/libscanfacedetector.a  ./lib_video_net_lib.a /opt/quan_zhi_x3/quanzhi_lib/lib/libogg.a /opt/quan_zhi_x3/quanzhi_lib/lib/libspeex.a /opt/quan_zhi_x3/quanzhi_lib/lib/libspeexdsp.a

cp:
	cp $(TARGET) /mnt/hgfs/tftpboot/1target/$(TARGET)

$(OBJFILES):	%.o: %.c $(HEADERS)
	@echo Compiling $@ from $<..
	@$(COMPILE.c) -o $@ $<	


clean:
	@echo Removing generated files..
	$(VERBOSE) -$(RM) -rf $(XDC_CFG) $(OBJFILES) $(TARGET) *~ *.d .dep

