TARGET = psp-ice
OBJS = src/main.o src/network.o src/input.o

CFLAGS = -O2 -G0 -Wall -Iinclude
LIBS = -lpspnet_inet -lpspnet -lpspctrl -lpspdebug

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP-ICE

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
