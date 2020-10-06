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

DIR = Pegasus/General

LIBRARIES = \
        pegcommon

include $(ROOT)/mak/config.mak

ifdef PEGASUS_HAS_SSL
    ifdef OPENSSL_HOME
        SYS_INCLUDES += -I$(OPENSSL_HOME)/include
    endif
endif

LOCAL_DEFINES = -DPEGASUS_GENERAL_INTERNAL -DPEGASUS_INTERNALONLY

ifeq ($(OS_TYPE),vms)
    STATIC=1
endif

STATIC=1
LIBRARY = peggeneral

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
    LIBRARIES = ILEWrapperUtils
endif

SOURCES = \
    CIMError.cpp \
    PropertyAccessor.cpp\
    DynamicLibrary.cpp \
    OptionManager.cpp \
    Stopwatch.cpp \
    SSLContextManager.cpp \
    MofWriter.cpp \
    Guid.cpp \
    VersionUtil.cpp \
    SubscriptionKey.cpp
   
ifdef PEGASUS_HAS_SSL
    ifeq ($(OS_TYPE),windows)
        SYS_LIBS += /libpath:$(OPENSSL_HOME)/lib libeay32.lib ssleay32.lib
    else
        ifeq ($(OS_TYPE), vms)
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

ifeq ($(OS_TYPE),vms)
    ifeq ($(PEGASUS_USE_STATIC_LIBRARIES),false)
        SYS_LIBS += tcpip$$library:tcpip$$lib/lib
        LIBRARIES = vms/include=(vms_crtl_init)
        LINK_OPT1 = COLLECT=crtl_init,lib$$initializdz,lib$$initializd_,
        LINK_OPT2 = lib$$initialize,lib$$initialize$$
        LINKER_OPTIONS = $(LINK_OPT1)$(LINK_OPT2)
    endif
endif

ifeq ($(PEGASUS_PLATFORM),AIX_RS_IBMCXX)
    AIX_LIB_PRIORITY=-20
endif

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
    AIX_LIB_PRIORITY=-20
endif

# Note that PEGASUS_WINDOWS_SDK_HOME only needs to be specified if using VC6.
ifeq ($(OS_TYPE),windows)
    SYS_LIBS += ws2_32.lib advapi32.lib netapi32.lib ole32.lib
    ifdef PEGASUS_WINDOWS_SDK_HOME
        FLAGS += -DPEGASUS_WINDOWS_SDK_HOME
        SYS_LIBS += /libpath:$(PEGASUS_WINDOWS_SDK_HOME)/lib secur32.lib
    else
        ifneq ($(CL_MAJOR_VERSION), 12)
            SYS_LIBS += secur32.lib
        endif
    endif
endif

include $(ROOT)/mak/dynamic-library.mak
# DO NOT DELETE
