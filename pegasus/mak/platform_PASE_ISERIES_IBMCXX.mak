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
include $(ROOT)/mak/config-unix.mak

OS = PASE

ARCHITECTURE = iSeries

COMPILER = xlc

PLATFORM_VERSION_SUPPORTED = yes

SYS_INCLUDES = -I/QOpenSys/vac70/vacpp/include

DEFINES = -DPEGASUS_PLATFORM_$(PEGASUS_PLATFORM)

DEPEND_INCLUDES =

DEPEND_DEFINES = -D__IBMCPP__=500

ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
FLAGS = -g
else
FLAGS = -O3 -qstrict -s
endif

#FLAGS += -qrtti=dyna -qcpluscmt -qlanglvl=newexcp
FLAGS += -qldbl128 -qlonglong -qalign=natural -H16 -qrtti=dyna -qcpluscmt -qlanglvl=newexcp -lc128

SYS_LIBS = -ldl

# SSL support
ifdef PEGASUS_HAS_SSL
ifndef OPENSSL_HOME
OPENSSL_HOME = /QOpenSys/QIBM/ProdData/SC1/OpenSSL/openssl-0.9.7d
endif
endif

CXX = xlC_r

SH = sh

YACC = bison

RM = rm -f

DIFF = diff

SORT = sort

COPY = cp

MOVE = mv

LIB_SUFFIX = .so

#share xlc options with AIX
AIX_LIB_PRIORITY = 0
AIX_LD_HALT = 8

ifndef PEGASUS_USE_MU_DEPEND
PEGASUS_HAS_MAKEDEPEND = yes
endif

PEGASUS_SUPPORTS_DYNLIB = yes

DEFINES += -DPEGASUS_HAS_SIGNALS

ifdef PEGASUS_USE_RELEASE_DIRS
  PEGASUS_DEST_LIB_DIR =/QOpenSys/QIBM/ProdData/UME/Pegasus/lib
endif

