TARGET = psp-ice
OBJS = src/main.o src/network.o src/input.o

CFLAGS = -O2 -G0 -Wall -Iinclude

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP-ICE

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

# build.mak appends the normal SDK libraries first. pspnet_inet is not
# included there, so add it after the SDK defaults to keep all import stubs
# contiguous for psp-fixup-imports.
LIBS += -lpspnet_inet
