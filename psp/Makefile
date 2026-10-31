
TARGET = psp-ice

OBJS = \
	src/main.o \
	src/input/input.o \
	src/network/network.o

CFLAGS = -O2 -G0 -Wall -Iinclude

LIBS = \
	-lpspdebug \
	-lpspdisplay \
	-lpspgu \
	-lpspge \
	-lpspctrl \
	-lpspnet \
	-lpspnet_inet \
	-lpspnet_apctl

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP ICE (In Car Entertainment)

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
