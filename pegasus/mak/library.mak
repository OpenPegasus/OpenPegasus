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
ifeq ($(PLATFORM_VERSION_SUPPORTED), yes)
  ifdef PLATFORM_COMPONENT_NAME
     DEFINES += -DPLATFORM_COMPONENT_NAME=\"$(PLATFORM_COMPONENT_NAME)\"
  else
     DEFINES += -DPLATFORM_COMPONENT_NAME=\"$(LIBRARY)\"
  endif
endif

INCLUDES = -I$(ROOT)/src $(EXTRA_INCLUDES)

include $(ROOT)/mak/common.mak

ifdef PEGASUS_EXTRA_LIBRARY_LINK_FLAGS
    EXTRA_LINK_FLAGS += $(PEGASUS_EXTRA_LIBRARY_LINK_FLAGS)
endif

################################################################################
##
## Build list of object names.
##
################################################################################

TMP_OBJECTS = $(foreach i,$(SOURCES),$(OBJ_DIR)/$i)

ifeq ($(OS_TYPE),windows)
S_OBJECTS = $(TMP_OBJECTS:.s=.obj)
CPP_OBJECTS = $(S_OBJECTS:.cpp=.obj)
OBJECTS = $(CPP_OBJECTS:.c=.obj)
else
S_OBJECTS = $(TMP_OBJECTS:.s=.o)
CPP_OBJECTS = $(S_OBJECTS:.cpp=.o)
OBJECTS = $(CPP_OBJECTS:.c=.o)
endif

################################################################################
##
## Library rule:
##
################################################################################

ifeq ($(OS_TYPE),windows)
include $(ROOT)/mak/library-windows.mak
endif
ifeq ($(OS_TYPE),unix)
include $(ROOT)/mak/library-unix.mak
endif
ifeq ($(OS_TYPE),vms)
 include $(ROOT)/mak/library-vms.mak
endif

################################################################################
##
## Clean rules:
##
################################################################################

include $(ROOT)/mak/clean.mak

################################################################################
##
## Build list of object names:
##
################################################################################

include $(ROOT)/mak/objects.mak

include $(ROOT)/mak/depend.mak

include $(ROOT)/mak/build.mak

include $(ROOT)/mak/docxx.mak

include $(ROOT)/mak/sub.mak

-include $(OBJ_DIR)/depend.mak

include $(ROOT)/mak/misc.mak

tests: $(ERROR)

#l10n

messages : $(ERROR)

poststarttests: $(ERROR)

