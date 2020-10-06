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

include $(ROOT)/mak/config.mak

DIRS = \
    cliutils \
    cimconfig \
    cimauth \
    cimprovider \
    osinfo \
    tomof \
    ipinfo \
    cimperf \
    repupgrade  \
    cimsub \
    cimcli \
    repupgrade/SSPModules/SampleSSPModule \
    loadxml

TEST_DIRS = \
    TestClient \
    cimprovider/tests \
    g11ntest \
    benchmarkTest \
    cimsub/tests \
    repupgrade/tests \
    cimcli/tests

# In WMIMapper, wbemexec functionality for windows is available
# No need to build the wbemexec under Clients directory 

ifneq ($(PEGASUS_BUILD_WMIMAPPER),true)
    DIRS += wbemexec
    TEST_DIRS += wbemexec/tests
endif

ifeq ($(OS),HPUX)
    DIRS += \
        ntpinfo \
        dnsinfo \
        nisinfo
endif

ifeq ($(OS),VMS)
    DIRS += \
        ntpinfo \
        dnsinfo \
        nisinfo
endif

ifdef PEGASUS_HAS_SSL
    DIRS += \
        TestCertClient \
        cimtrust
    TEST_DIRS += cimtrust/tests
    ifeq ($(PEGASUS_ENABLE_SSL_CRL_VERIFICATION),true)
        DIRS += cimcrl
        TEST_DIRS += cimcrl/tests
    endif
endif

ifneq ($(OS_TYPE),windows)
    DIRS += cimreparchive
    TEST_DIRS += cimreparchive/tests
endif

# Have cimuser as a crude replacement of PAM
ifndef PEGASUS_PAM_AUTHENTICATION
    DIRS += cimuser
endif

#The following tests are NOT included as part of the standard
#regession test.  The cimuser tests require PrivilegedUser
#access to execute successfully.  The cimconfig tests modify
#configuration files.

XDIRS = \
    cimconfig/tests \
    cimuser/tests

include $(ROOT)/mak/recurse.mak
