#//%LICENSE////////////////////////////////////////////////////////////////
#//
#// Licensed to The Open Group (TOG) under one or more contributor license
#// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
#// this work for additional information regarding copyright ownership.
#// Each contributor licenses this file to you under the OpenPegasus Open
#// Source License; you may not use this file except in compliance with the
#// License.
#//
#// Permission is hereby granted, free of charge, to any person obtaining a
#// copy of this software and associated documentation files (the "Software"),
#// to deal in the Software without restriction, including without limitation
#// the rights to use, copy, modify, merge, publish, distribute, sublicense,
#// and/or sell copies of the Software, and to permit persons to whom the
#// Software is furnished to do so, subject to the following conditions:
#//
#// The above copyright notice and this permission notice shall be included
#// in all copies or substantial portions of the Software.
#//
#// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#//
#//////////////////////////////////////////////////////////////////////////
ifdef VMS_HAS_CC
    CPPVMSOBJECTS = $(SOURCES:.cpp=.obj,)
    TMPVMSOBJECTS += $(CPPVMSOBJECTS:.c=.obj,)

    TMP_OBJECTS = $(foreach i,$(SOURCES),$(OBJ_DIR)/$i)
    CPP_OBJECTS = $(TMP_OBJECTS:.cpp=.obj)
    OBJECTS = $(CPP_OBJECTS:.c=.obj)
else
    TMPVMSOBJECTS = $(SOURCES:.cpp=.obj,)

    TMP_OBJECTS = $(foreach i,$(SOURCES),$(OBJ_DIR)/$i)
    OBJECTS = $(TMP_OBJECTS:.cpp=.obj)
endif

VMSOBJECTS = $(OBJ_VMSDIRA)]$(TMPVMSOBJECTS)

FULL_LIB=$(LIB_DIR)/lib$(LIBRARY)$(LIB_SUFFIX)
FULL_VMSLIB=$(LIB_VMSDIRA)]lib$(LIBRARY)$(LIB_SUFFIX)

FULL_PROGRAM=$(BIN_DIR)/$(PROGRAM)$(EXE)
FULL_VMSPROGRAM=$(BIN_VMSDIRA)]$(PROGRAM)$(EXE)

EXE_OUTPUT =$(FULL_PROGRAM)

ifeq ($(PEGASUS_USE_STATIC_LIBRARIES),true)
    EXTRA_LIBRARIES += \
        -L$(OPENSSL_LIB) -lssl$$libssl_shr32 -lssl$$libcrypto_shr32
    LIBRARIES += vms/include=vms_crtl_init
endif

ifeq (,$(findstring libpegrepository, $(LIBRARIES)))
    EXTRA_LIBRARIES += -Llibz -lz
endif

ifdef VMSSHARE
    EXE_VMSOUTPUT =/sysexe/share=$(FULL_VMSPROGRAM)
else
    EXE_VMSOUTPUT =/exe=$(FULL_VMSPROGRAM)
endif

#OPT = $(VMSROOT)[src$(VMSDIRA)]$(PROGRAM)/opt
OPT = $(OPT_VMSDIRA)]$(PROGRAM)/opt
VMSPROGRAM = YES

# GSMATCH and IDENTIFICATION

LINKER_OPTIONS += \
    GSMATCH=$(PEGASUS_VMS_LINKER_GSMATCH) \
    IDENTIFICATION=$(PEGASUS_VMS_LINKER_IDENT)

$(FULL_PROGRAM): $(OBJ_DIR)/target $(BIN_DIR)/target $(OPT_DIR)/target \
    $(OBJECTS) $(FULL_LIBRARIES) $(ERROR)

ifdef OBJECTS_IN_OPTIONFILE
	@ take $(PLATFORM_VMSDIRA)]vms_create_optfile.com \
            "$(PROGRAM)" "$(strip $(LIBRARIES))" "$(strip $(EXTRA_LIBRARIES))" \
            "$(strip $(VMS_VECTOR))" "$(strip $(LINKER_OPTIONS))" \
            "$(SOURCES)" "$(OBJ_VMSDIRA)]"
	cxxlink$(LFLAGS)$(EXE_VMSOUTPUT)/reposit=$(CXXREPOSITORY_VMSROOT) \
            $(foreach lib,$(SYS_LIBS),$(lib),) $(OPT) $(DLLOPT)
else
	@ take $(PLATFORM_VMSDIRA)]vms_create_optfile.com \
            "$(PROGRAM)" "$(strip $(LIBRARIES))" "$(strip $(EXTRA_LIBRARIES))" \
            "$(strip $(VMS_VECTOR))" "$(strip $(LINKER_OPTIONS))"
	cxxlink$(LFLAGS)$(EXE_VMSOUTPUT)/reposit=$(CXXREPOSITORY_VMSROOT) \
            $(VMSOBJECTS) $(foreach lib,$(SYS_LIBS),$(lib),) $(OPT) $(DLLOPT)
endif
	@ $(TOUCH) $(FULL_VMSPROGRAM)
ifdef SHARE_COPY
    ifdef PEGASUS_RELEASE_BUILD
#	$(COPY) $(FULL_VMSPROGRAM) sys$$share:$(PROGRAM)$(EXE)
	$(COPY) "$(FULL_VMSPROGRAM)" "$(PEGASUS_SYSSHARE)$(PROGRAM)$(EXE)"
    endif
endif
ifdef VMSSHARE
	library/create/share $(FULL_VMSLIB) $(FULL_VMSPROGRAM)
	define/job/log $(PROGRAM) $(FULL_VMSPROGRAM)
endif
	@ $(ECHO)

include $(ROOT)/mak/objects.mak

FILES_TO_CLEAN = $(VMSOBJECTS) $(FULL_VMSPROGRAM);

include $(ROOT)/mak/clean.mak

-include $(ROOT)/mak/depend.mak

include $(ROOT)/mak/build.mak

include $(ROOT)/mak/sub.mak

-include $(OBJ_DIR)/depend.mak

include $(ROOT)/mak/misc.mak
	
