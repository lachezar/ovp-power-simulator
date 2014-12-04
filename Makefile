IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")
include $(IMPERAS_HOME)/bin/Makefile.include

#
#  Makefile for Cross Compiling an appication for a target processor type
#

# Various Cross compiler setups, Default or1k
CROSS?=OR1K
-include $(IMPERAS_HOME)/lib/$(IMPERAS_ARCH)/CrossCompiler/$(CROSS).makefile.include
ifeq ($($(CROSS)_CC),)
    IMPERAS_ERROR := $(error "Please install the $(CROSS) toolchain")
endif

ASRC = application.c
AEXE = $(patsubst %.c,%.$(CROSS).elf,$(ASRC))

all: $(AEXE)

%.$(CROSS).elf: %.o
	$(V)  echo "Linking $@"
	$(V) $(IMPERAS_LINK) -o $@ $< $(IMPERAS_LDFLAGS) -lm

%.o: %.c
	$(V)  echo "Compiling $<"
	$(V) $(IMPERAS_CC) -c -o $@ $<

clean:
	- rm -f *.elf *.o
