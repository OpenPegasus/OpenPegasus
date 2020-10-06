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
# Makes the dynamic load libs for the files named in DIR
ROOT = ../../../..

DIR = Pegasus/ControlProviders/CertificateProvider

include $(ROOT)/mak/config.mak

ifdef PEGASUS_HAS_SSL
    ifdef OPENSSL_HOME
        SYS_INCLUDES += -I$(OPENSSL_HOME)/include
    endif
else
    $(error PEGASUS_HAS_SSL not defined)
endif

LOCAL_DEFINES = -DPEGASUS_CERTIFICATEPROVIDER_INTERNAL -DPEGASUS_INTERNALONLY

ifeq ($(OS_TYPE),vms)
    STATIC=1
endif

LIBRARY = CertificateProvider

LIBRARIES = \
    pegrepository \
    pegprovider \
    pegconfig \
    peggeneral \
    pegcommon

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
    LIBRARIES += ILEWrapperUtils
endif

SOURCES = CertificateProvider.cpp

ifeq ($(PEGASUS_SUPPORTS_DYNLIB),yes)
    NEEDS_LIBRARIES = -L$(LIB_DIR) -lpegcommon -lpegconfig -lpegprovider
endif

ifdef PEGASUS_HAS_SSL
    ifeq ($(OS_TYPE),windows)
        SYS_LIBS += /libpath:$(OPENSSL_HOME)/lib libeay32.lib ssleay32.lib
    else
        ifeq ($(OS_TYPE),vms)
            EXTRA_LIBRARIES += -L$(OPENSSL_LIB) \
                -lssl$$libssl_shr32 -lssl$$libcrypto_shr32
        else
            ifdef OPENSSL_HOME
                EXTRA_LIBRARIES += -L$(OPENSSL_HOME)/lib
            endif
            EXTRA_LIBRARIES += -lssl -lcrypto
        endif
    endif
endif

ifeq ($(PEGASUS_PLATFORM),AIX_RS_IBMCXX)
    AIX_LIB_PRIORITY=-20
endif

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
    AIX_LIB_PRIORITY=-20
endif

ifeq ($(OS_TYPE),windows)
    SYS_LIBS += ws2_32.lib advapi32.lib netapi32.lib
endif

include $(ROOT)/mak/dynamic-library.mak
