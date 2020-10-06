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
#//              Lyle Wilkinson      <lyle_wilkinson@hp.com>
#//              Sean Keenan Hewlett-Packard Company <sean.keenan@hp.com>
#//
#//%////////////////////////////////////////////////////////////////////////////

ROOT = ../../../..

DIR = Providers/ManagedSystem/IP

include $(ROOT)/mak/config.mak

LIBRARY = IPProviderModule

EXTRA_INCLUDES = 

SOURCES = \
	IPPlatform.cpp \
	IPPEpProvider.cpp \
        BIPTLEpProvider.cpp \
	IPRouteProvider.cpp \
	NextHopIPRouteProvider.cpp \
	RSApProvider.cpp \
	RUEpProvider.cpp \
	ANHProvider.cpp \
	IPMain.cpp

LIBRARIES = \
    pegprovider \
    pegclient \
    pegcommon

DYNAMIC_LIBRARIES = \
	-lpegcommon \
	-lpegprovider

ifeq ($(OS),HPUX)
  EXTRA_LIBRARIES += -lnm
endif

include $(ROOT)/mak/dynamic-library.mak

tests:

