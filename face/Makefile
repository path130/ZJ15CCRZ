SRC := face.c dpvideo.c preview.c sensor.c
SRC += frame.c ccic2uni.c utf2uni.c
QUAN_ZHI_LIB_DIR= ../../quanzhi_lib
QUAN_ZHI_OSD_LIB_DIR= ../../quan_zhi_osd_lib
C_FLAGS += -I $(QUAN_ZHI_LIB_DIR)/include
LD_FLAGS += -L $(QUAN_ZHI_LIB_DIR)/lib 


C_FLAGS += -I./code/include -I./code/dpgraphic -I./code/ui/cctrl -I./code/phoneapp -I./code/dpsession -I./code -I./

all: $(SRC) libface
	arm-none-linux-gnueabi-gcc -o face.o -c $(SRC) libfacedetector.a libscanfacedetector.a $(C_FLAGS) -lpthread -lstdc++

debug: a.out

a.out: $(SRC) libface debug/main.c
	arm-none-linux-gnueabi-gcc debug/main.c $(SRC) libfacedetector.a libscanfacedetector.a $(C_FLAGS) $(LD_FLAGS) -lpthread -lstdc++ -lfreetype
	cp a.out /var/lib/tftpboot

libface: libfacedetector.a libscanfacedetector.a

clean:
	make -C face clean
