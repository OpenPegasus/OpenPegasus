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
# Configuration options for Pegasus on all architectures running Linux
# These options are also used by clang( which was designed as dropin
# replacement for gcc). So some of the names are misleading like 
# PEGASUS_PLATFORM_LINUX_GENERIC_GNU due to GNU appended, Better will
# be PEGASUS_PLATFORM_LINUX_GENERIC and GNU and CLANG can add it. 
# Changing this now(for 9236) will involve lot of work, Will take up 
# this work later. 


include $(ROOT)/mak/config-unix.mak

PEGASUS_PLATFORM_LINUX_GENERIC_GNU = 1
DEFINES += -DPEGASUS_PLATFORM_LINUX_GENERIC_GNU
DEFINES += -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)

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


# Enable CMPI by default.
#
ifndef PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER
PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER=true
endif

#########################################################################

OS = linux


ifeq ($(findstring _CLANG, $(PEGASUS_PLATFORM)), _CLANG)
    COMPILER = clang
    CXX = clang++
    CC = clang
else
    COMPILER = gnu
    ifndef CXX
        CXX = g++
    endif
endif

PLATFORM_VERSION_SUPPORTED = yes


SH = sh

YACC = bison

PEGASUS_SUPPORTS_DYNLIB = yes

MAJOR_VERSION_NUMBER = 1

LIB_SUFFIX = .so.$(MAJOR_VERSION_NUMBER)

DEFINES += -DPEGASUS_USE_SYSLOGS

SYS_LIBS = -ldl -lpthread -lcrypt

ifeq ($(COMPILER), clang)
    FLAGS += -W -Wall -Wno-unused-parameter  -Wno-unused-value -D_GNU_SOURCE \
        -DTHREAD_SAFE -D_REENTRANT -Werror=unused-variable -Wno-unused-function \
        -Werror=switch
else
    FLAGS += -W -Wall -Wno-unused -Wunused-variable
  # Starting with gcc 4.3 specific warnings can be reported as error
  # Enabling a specific selection of warnings to turn into errors
  ifeq ($(shell expr $(GCC_VERSION) '>=' 4.3), 1)
    FLAGS += -Werror=unused-variable
    FLAGS += -Werror=switch
   endif
    FLAGS += -D_GNU_SOURCE -DTHREAD_SAFE -D_REENTRANT
endif


##==============================================================================
##
## The DYNAMIC_FLAGS variable defines linker flags that only apply to shared
## libraries.
##
##==============================================================================
DYNAMIC_FLAGS += -fPIC

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
  FLAGS += -g
else
  #
  # The -fno-enforce-eh-specs is not available in 2.9.5 and it probably
  # appeared in the 3.0 series of compilers.
  #
  ifeq ($(COMPILER), gnu)
   # disable the strict aliasing
   ifeq ($(shell expr $(GCC_VERSION) '>=' 3.0), 1)
     PEGASUS_EXTRA_CXX_FLAGS += -fno-enforce-eh-specs -fno-strict-aliasing
   endif
  endif
      
  ifdef PEGASUS_OPTIMIZE_FOR_SIZE
    ifeq ($(COMPILER), gnu)
      FLAGS += -Os
    else
      FLAGS += -Oz
    endif
  else
    FLAGS += -O2
  endif
endif

FLAGS += $(CXX_MACHINE_OPTIONS)

ifdef PEGASUS_ENABLE_GCOV
  FLAGS += -ftest-coverage -fprofile-arcs
  SYS_LIBS += -lgcc -lgcov
  EXTRA_LIBRARIES += -lgcc -lgcov
endif

ifndef PEGASUS_USE_MU_DEPEND
PEGASUS_HAS_MAKEDEPEND = yes
endif

##==============================================================================
##
## Set the default visibility symbol to hidden for shared libraries. This
## feature is only available in GCC 4.0 and later.
##
##==============================================================================

ifeq ($(COMPILER), gnu)
 ifeq ($(shell expr $(GCC_VERSION) '>=' 4.0), 1)
    FLAGS += -fvisibility=hidden
 endif
else
    FLAGS +=-fvisibility=hidden	
endif

ifndef PEGASUS_ARCH_LIB
  ifeq ($(PEGASUS_PLATFORM),LINUX_X86_64_GNU)
        PEGASUS_ARCH_LIB = lib64
  endif
  ifeq ($(PEGASUS_PLATFORM),LINUX_X86_64_CLANG)
        PEGASUS_ARCH_LIB = lib64
  endif
  PEGASUS_ARCH_LIB = lib
endif
DEFINES += -DPEGASUS_ARCH_LIB=\"$(PEGASUS_ARCH_LIB)\"
