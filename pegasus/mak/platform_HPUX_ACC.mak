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
MAJOR_VERSION_NUMBER = 1

OS = HPUX

ifdef ACC_COMPILER_COMMAND
   CXX = $(ACC_COMPILER_COMMAND)
else
   CXX = aCC
endif

CC = $(CXX) -Ae

COMPILER = acc

PLATFORM_VERSION_SUPPORTED = yes

SYS_INCLUDES =

ifdef PEGASUS_CCOVER
 SYS_INCLUDES += -I/opt/ccover11/include
endif

ifdef PEGASUS_PURIFY
 SYS_INCLUDES += -I$(PURIFY_HOME)
endif

#########################################################################
##
## Platform specific compile options controlled by environment variables
## are set here.
##
#########################################################################

# Enable OOP by default if preference not already set in the environment
#
ifndef PEGASUS_DEFAULT_ENABLE_OOP
PEGASUS_DEFAULT_ENABLE_OOP = true
endif



#########################################################################

DEFINES = -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM) -DPEGASUS_PLATFORM_HPUX_ACC

DEFINES += -DPEGASUS_USE_SYSLOGS

ifeq ($(PEGASUS_USE_EMANATE), true)
 DEFINES += -DHPUX_EMANATE
endif

ifdef PEGASUS_CCOVER
 DEFINES += -DPEGASUS_CCOVER
endif

ifdef PEGASUS_PURIFY
 DEFINES += -DPEGASUS_PURIFY
endif

DEPEND_INCLUDES =


## Flags:
##     +Z - produces position independent code (PIC).
##     +DAportable generates code for any HP9000 architecture
##     -Wl, passes the following option to the linker
##       +s causes the linked image or shared lib to be able to
##          search for any referenced shared libs dynamically in
##          SHLIB_PATH (LD_LIBRARY_PATH on 64-bit HP9000)
##       +b enables dynamic search in the specified directory(ies)
##

FLAGS =

PEGASUS_SUPPORTS_DYNLIB=yes

ifeq ($(HAS_ICU_DEPENDENCY),true)
    ifdef ICU_INSTALL
        P1 = $(ICU_INSTALL)/lib:
     endif
endif
ifdef PEGASUS_USE_RELEASE_DIRS
    P2 = $(PEGASUS_DEST_LIB_DIR):
else
    P2 = $(LIB_DIR):
endif
ACC_LINK_SEARCH_PATH = -Wl,+b$(P1)$(P2)/usr/lib

ifdef PEGASUS_USE_RELEASE_DIRS
    ifeq ($(PEGASUS_PLATFORM), HPUX_PARISC_ACC)
        ACC_LINK_SEARCH_PATH += \
            -Wl,+cdp,$(PEGASUS_PLATFORM_SDKROOT)/usr/lib:/usr/lib
        ACC_LINK_SEARCH_PATH += \
            -Wl,+cdp,$(PEGASUS_HOME)/lib:$(PEGASUS_DEST_LIB_DIR)
        ifdef OPENSSL_HOME
          ACC_LINK_SEARCH_PATH += -Wl,+cdp,$(OPENSSL_HOME)/lib:/usr/lib
        endif
        ifdef ICU_INSTALL
          ACC_LINK_SEARCH_PATH += \
              -Wl,+cdp,$(ICU_INSTALL)/lib:$(PEGASUS_DEST_LIB_DIR)
        endif
    endif
endif
ACC_LINK_SEARCH_PATH += -Wl,+s

FLAGS += $(ACC_LINK_SEARCH_PATH)

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
  FLAGS += -g +noobjdebug
else
  FLAGS += +O2 -s
endif

ifdef PEGASUS_USE_RELEASE_DIRS
    PEGASUS_DEST_LIB_DIR=/opt/wbem/lib
endif

#
#  For future use on HP-UX
#
ifdef HPUX_LARGE_INTERFACES
        FLAGS += -D_HPUX_API_LEVEL=20040821
endif

SYS_LIBS = -lpthread -lrt

SH = sh

YACC = bison

RM = rm -f

DIFF = diff

SORT = sort

COPY = cp

MOVE = mv

LIB_SUFFIX = .$(MAJOR_VERSION_NUMBER)
