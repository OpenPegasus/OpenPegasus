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
#//
#// Author: Christopher Neufeld <neufeld@linuxcare.com>
#//         David Kennedy       <dkennedy@linuxcare.com>
#//
#// Modified By: David Kennedy       <dkennedy@linuxcare.com>
#//              Christopher Neufeld <neufeld@linuxcare.com>
#//              Sean Keenan Hewlett-Packard Company <sean.keenan@hp.com>
#//              David Dillard, Symantec Corp. (david_dillard@symantec.com)
#//
#//%////////////////////////////////////////////////////////////////////////////

ROOT = ../../../..

DIR = Providers/ManagedSystem/OperatingSystem

include $(ROOT)/mak/config.mak

LIBRARY = OSProvider

SOURCES = \
	OperatingSystemProvider.cpp \
	OperatingSystemProviderMain.cpp \
	OperatingSystem.cpp

LIBRARIES = \
    pegprovider \
    pegclient \
    pegcommon

ifeq ($(OS_TYPE),vms)
    LIBRARIES += vms
endif

DYNAMIC_LIBRARIES = \
	-lpegcommon \
	-lpegprovider

ifeq ($(OS_TYPE),windows)
# Note that PEGASUS_WINDOWS_SDK_HOME only needs to be specified if using VC6.
  ifdef PEGASUS_WINDOWS_SDK_HOME
	FLAGS += -DPEGASUS_WINDOWS_SDK_HOME
    SYS_INCLUDES += -I$(PEGASUS_WINDOWS_SDK_HOME)/include
    EXTRA_LIBRARIES += -LIBPATH:$(PEGASUS_WINDOWS_SDK_HOME)/lib psapi.lib
  else
    ifneq ($(CL_MAJOR_VERSION),12)
      EXTRA_LIBRARIES += psapi.lib
    endif    
  endif

  EXTRA_LIBRARIES += pdh.lib
endif

include $(ROOT)/mak/dynamic-library.mak

tests:

