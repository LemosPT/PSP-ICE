TARGET = psp-ice
OBJS = src/main.o src/network.o src/input.o

CFLAGS = -O2 -G0 -Wall -Iinclude
# build.mak supplies the normal user-mode PSP libraries. Only add the
# networking library that is not part of the default list.
LIBS = -lpspnet_inet

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP-ICE

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
