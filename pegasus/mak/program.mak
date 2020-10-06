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
     DEFINES += -DPLATFORM_COMPONENT_NAME=\"$(PROGRAM)\"
  endif
endif

include $(ROOT)/mak/common.mak
ifdef PEGASUS_EXTRA_PROGRAM_LINK_FLAGS
    EXTRA_LINK_FLAGS += $(PEGASUS_EXTRA_PROGRAM_LINK_FLAGS)
endif

ifeq ($(OS_TYPE),windows)
include $(ROOT)/mak/program-windows.mak
endif
ifeq ($(OS_TYPE),unix)
include $(ROOT)/mak/program-unix.mak
# GCC supports the -fPIE switch in version 3.4 or later
 ifeq ($(OS),linux)
   ifeq ($(COMPILER), gnu)
       ifeq ($(shell expr $(GCC_VERSION) '>=' 3.4), 1)
           FLAGS := $(FLAGS:-fPIC=-fPIE)
       endif
   else
       FLAGS:= $(FLAGS:-fpic=-fpie)
   endif
 endif
endif
ifeq ($(OS_TYPE),vms)
 include $(ROOT)/mak/program-vms.mak
endif

#l10n
messages: $(ERROR)
