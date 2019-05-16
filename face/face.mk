
JSON_INCLUDE=./face/json/include/json-c
JSON_LIB=./face/json/lib

ZBAR_INCLUDE=./face/zbar
ZBAR_LIB=./face/zbar

FACE_SRCDIR = ./face
SRC_DIR+= $(FACE_SRCDIR)

C_FLAGS += -I$(FACE_SRCDIR)
C_FLAGS += -I$(JSON_INCLUDE)
C_FLAGS += -I$(ZBAR_INCLUDE)

LD_FLAGS += -L$(JSON_LIB) -ljson-c -lstdc++
LD_FLAGS += -L$(ZBAR_LIB) -lzbar
