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
ROOT = ../../..

DIR = WMIMapper/WMIProvider

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DPEGASUS_WMIPROVIDER_INTERNAL -DPEGASUS_INTERNALONLY

INCLUDES = -I..

ifeq ($(PEGASUS_WMIMAPPER_NEED_MSSDK),true)
  EXTRA_INCLUDES = -I"$(MSSdk)/Include" $(SYS_INCLUDES)
endif

LIBRARY = WMIProvider

LIBRARIES = \
	pegcommon
		
SOURCES = \
   WMIInstanceProvider.cpp \
   WMIClassProvider.cpp \
   WMIAssociatorProvider.cpp \
   WMIQualifierProvider.cpp \
   WMIMethodProvider.cpp \
   WMIReferenceProvider.cpp \
   WMIQueryProvider.cpp \
   WMIBaseProvider.cpp \
   WMICollector.cpp \
   WMIType.cpp \
   WMIFlavor.cpp \
   WMIValue.cpp \
   WMIString.cpp \
   WMIProperty.cpp \
   WMIQualifier.cpp \
   WMIQualifierSet.cpp \
   WMIDateTime.cpp \
   WMIObject.cpp \
   WMIObjectPath.cpp \
   WMIMethod.cpp \
   StdAfx.cpp \
   MyString.cpp

SYS_LIBS = uuid.lib wbemuuid.lib Framedyn.lib ws2_32.lib advapi32.lib
ifeq ($(CL_MAJOR_VERSION), 14)
  SYS_LIBS += comsuppw.lib
endif

include $(ROOT)/mak/library.mak
