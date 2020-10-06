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
#//              Paulo Fernando Borges da Silva <pfborges@wowmail.com>
#//              Sean Keenan Hewlett-Packard Company <sean.keenan@hp.com>
#//
#//%////////////////////////////////////////////////////////////////////////////

ROOT = ../../../..

PROVIDERROOT = $(ROOT)/src/Providers/linux
PROVIDERINCLUDE_ROOT = $(PROVIDERROOT)/Include
PROVIDERSUPPORT_ROOT = $(PROVIDERROOT)/ProviderSupport
PROVIDERDATA_ROOT    = $(PROVIDERROOT)/ProviderData

DIR = Providers/ManagedSystem/DNSService

include $(ROOT)/mak/config.mak

LIBRARY = DNSProvider

EXTRA_INCLUDES = -I$(ROOT)/src/Providers/ManagedSystem/NTPProviderSecurity

SOURCES = \
	DNSServiceProvider.cpp \
	DNSServiceProviderMain.cpp \
        DNSService.cpp

ifeq ($(OS),VMS)
LIBRARIES = \
    NTPProviderSecurity \
    pegprovider \
    pegclient \
    pegcommon
else
LIBRARIES = \
    pegprovider \
    pegclient \
    pegcommon
 ifeq ($(OS),HPUX)
LIBRARIES += NTPProviderSecurity
 endif
endif

DYNAMIC_LIBRARIES = \
	-lpegcommon \
	-lpegprovider

include $(ROOT)/mak/dynamic-library.mak

tests:

