TARGET = psp-ice
OBJS = src/main.o src/network.o src/input.o

CFLAGS = -O2 -G0 -Wall -Iinclude
# Keep PSP networking libraries after the SDK defaults so import stubs
# are emitted in the order expected by psp-fixup-imports.
LIBS = -lpspnet -lpspnet_apctl -lpspnet_inet

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP-ICE

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
