TARGET = psp-ice
OBJS = src/main.o src/network.o src/input.o

CFLAGS = -O2 -G0 -Wall -Iinclude

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

# build.mak supplies the standard SDK libraries. Keep the networking
# imports in the historical PSPSDK order: pspnet -> pspnet_inet -> apctl.
LIBS := -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspnet -lpspnet_inet -lpspnet_apctl

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP-ICE
