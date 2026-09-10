TARGET = psp-ice
OBJS = src/main.o src/network.o src/input.o

CFLAGS = -O2 -G0 -Wall -Iinclude
LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspnet -lpspnet_inet -lpspnet_apctl
#LIBS = -lpspnet_inet -lpspnet -lpspctrl -lpspdebug

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP ICE (In Car Entertainment)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
