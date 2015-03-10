#
# Pse gcc
# Makefile for compiling BHM/PPM pse topse.pse
#

IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")

ifndef IMPERAS_HOME
  IMPERAS_ERROR := $(error "IMPERAS_HOME not defined")
endif

pse_check := $(sort $(wildcard $(IMPERAS_HOME)/lib/$(IMPERAS_ARCH)/CrossCompiler/PSE.makefile.include))
ifeq ($(strip $(pse_check)),) 
   IMPERAS_ERROR := $(error "The PSE compiler is not installed")
endif
include $(IMPERAS_HOME)/lib/$(IMPERAS_ARCH)/CrossCompiler/PSE.makefile.include
include $(IMPERAS_HOME)/ImperasLib/buildutils/Makefile.common

# Opt or Debug ?
OPT?=1
ifeq ($(OPT),1)
    PSE_CFLAGS += -O2
else
    PSE_CFLAGS += -g -gdwarf-2
endif
PSE_CFLAGS += -Wall -Werror -fno-unit-at-a-time -I$(PWD)

all: $(WORKDIR)/pse.pse

# include all the dependancy requirements
ifneq ($(MAKECMDGOALS),clean)
$(foreach dep,$(DEP),$(eval -include $(dep)))
endif

#
# pse.tcl ->
# pse.attrs.igen.c pse.c.igen.stubs pse.igen.c pse.igen.h pse.igen.xml pse.macros.igen.h
#
IGEN?= igen.exe
IGENFLAGS?= --quiet --nobanner --excludem GPT_NH --excludem GPT_UFNR
PSE-TCL  = $(wildcard pse.tcl)
PSE-SRC  = $(patsubst %.tcl, %.attrs.igen.c,           $(PSE-TCL)) \
           $(patsubst %.tcl, %.igen.c,                 $(PSE-TCL)) \
           $(patsubst %.tcl, %.igen.h,                 $(PSE-TCL)) \
           $(patsubst %.tcl, %.igen.stubs,             $(PSE-TCL)) \
           $(patsubst %.tcl, %.igen.xml,               $(PSE-TCL)) \
           $(patsubst %.tcl, %.macros.igen.h,          $(PSE-TCL))
PSE-OBJ  = $(patsubst %.tcl, $(OBJDIR)/%.attrs.igen.o, $(PSE-TCL)) \
           $(patsubst %.tcl, $(OBJDIR)/%.igen.o,       $(PSE-TCL))


$(WORKDIR)/pse.pse: $(OBJ) $(PSE-OBJ)
	$(V) echo "# Cross Linking Peripheral $@"
	$(V) mkdir -p $(@D)
	$(V) $(PSE_LINKXX) -o $@ $^ $(CFLAGS) $(PSE_LDFLAGS) $(LDFLAGS)
ifeq ($(COPYXML),1)
	$(V) echo "# Copying XML $(XML) to $(@D)"
	$(V) cp $(XML) $(@D)
endif

%.attrs.igen.c %.igen.c %.igen.h %.igen.stubs %.igen.xml %.macros.igen.h: %.tcl
	$(V) echo "# PreProcess $<"
	$(V) $(IGEN) $(IGENFLAGS)			     \
		--batch         $(<)			     \
		--writec        $(*)                 \
		--writexml      $(*).igen.xml        \
		$(WRITEHEADER)            			 \
		--overwrite

$(OBJDIR)/%.d: %.c $(PSE-SRC)
	$(V) echo "# Cross Depending Peripheral $@"
	$(V) mkdir -p $(@D)
	$(V) $(PSE_CC) -MM $< $(CFLAGS) -MT $(OBJDIR)/$*.o -MF $@
	$(V) sed -i 's/d:/\/cygdrive\/d/I' $@

$(OBJDIR)/%.o: %.c
	$(V) echo "# Cross Compiling Peripheral $@"
	$(V) mkdir -p $(@D)
	$(V) $(PSE_CC) -g -gdwarf-2 -c -o $@ $< $(CFLAGS)

CXX_SUFFIX=cpp c++ cxx
define CXX_SUFFIX_Template
$(OBJDIR)/%.d: %.$(1) $$(PSE-SRC)
	@    echo "# Host Depending $$@"
	$$(V) mkdir -p $$(@D)
	$$(V) $$(PSE_CC) -MM $$< $$(CFLAGS) -MT $$(OBJDIR)/$$*.o -MF $$@

$(OBJDIR)/%.o: %.$(1)
	$$(V) mkdir -p $$(@D)
	@    echo "# Host Compiling Peripheral $$@"
	$$(V) $$(PSE_CXX) -g -gdwarf-2 -c -o $$@ $$< $$(CFLAGS)
endef
$(foreach X,$(CXX_SUFFIX),$(eval $(call CXX_SUFFIX_Template,$(X))))


clean::
	$(V) - rm -f $(WORKDIR)/pse.pse
	$(V) - rm -f $(PSE-SRC)
	$(V) - rm -rf $(OBJDIR)
