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
ROOT = ../..

DIR = Service

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DPEGASUS_INTERNALONLY -DPEGASUS_SERVICE_INTERNAL

STATIC=1
LIBRARY = pegservice

LIBRARIES = \
    pegclient \
    pegcommon

SOURCES = \
    PidFile.cpp \
    ServerProcess.cpp \
    ServerRunStatus.cpp \
    ServerShutdownClient.cpp


SOURCES_ZOS = ARM_zOS.cpp

ifeq ($(PEGASUS_PLATFORM),ZOS_ZSERIES_IBM)
  SOURCES_ZOS += AutoRestartMgr_ZOS_ZSERIES_IBM.s
endif

#ifeq ($(PEGASUS_PLATFORM),ZOS_ZSERIES64_IBM)
#  SOURCES_ZOS += AutoRestartMgr_ZOS_ZSERIES64_IBM.s
#endif

ifeq ($(OS),zos)
   SOURCES += $(SOURCES_ZOS)
endif

EXTRA_INCLUDES = $(SYS_INCLUDES)

include $(ROOT)/mak/library.mak

ifeq ($(OS_TYPE),windows)
  SYS_LIBS = ws2_32.lib advapi32.lib	
endif


