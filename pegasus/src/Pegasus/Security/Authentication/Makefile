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
ROOT = ../../../..

DIR = Pegasus/Security/Authentication

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DPEGASUS_SECURITY_INTERNAL -DPEGASUS_INTERNALONLY

STATIC=1
LIBRARY = pegauthentication

LIBRARIES = \
    pegcommon \
    pegconfig

ifndef PEGASUS_PAM_AUTHENTICATION
  LIBRARIES += peguser
endif

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
  LIBRARIES += ILEWrapperUtils
endif

PRE_DEPEND_INCLUDES = -I./depends

ifeq ($(OS),HPUX)
    EXTRA_LIBRARIES += -lsec 
endif

SOURCES = \
    AuthenticationStatus.cpp \
    LocalAuthFile.cpp \
    SecureLocalAuthenticator.cpp \
    LocalAuthenticationHandler.cpp \
    SecureBasicAuthenticator.cpp \
    PAMBasicAuthenticator.cpp \
    BasicAuthenticationHandler.cpp \
    AuthenticationManager.cpp

ifeq ($(PEGASUS_ENABLE_SESSION_COOKIES),true)
    SOURCES += Cookies.cpp
endif

ifeq ($(PEGASUS_NEGOTIATE_AUTHENTICATION),true)
    SOURCES += NegotiateAuthenticationHandler.cpp
endif

# Special code calling pam_end in class AuthHandle only required
# with session based authentication mechanism
ifeq ($(PEGASUS_PAM_SESSION_SECURITY),true)
    SOURCES += PAMSessionBasicAuthenticator.cpp
endif
    

ifeq ($(OS_TYPE),windows)
SYS_LIBS = ws2_32.lib advapi32.lib
endif

include $(ROOT)/mak/library.mak
