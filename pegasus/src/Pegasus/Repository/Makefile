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

DIR = Pegasus/Repository

include $(ROOT)/mak/config.mak

ifeq ($(PEGASUS_USE_SQLITE_REPOSITORY),true)
    ifdef SQLITE_HOME
        SYS_INCLUDES += -I$(SQLITE_HOME)/include
    endif
endif

EXTRA_INCLUDES = -I..

LOCAL_DEFINES = -DPEGASUS_REPOSITORY_INTERNAL -DPEGASUS_INTERNALONLY

LIBRARIES = pegcommon pegquerycommon pegconfig

ifeq ($(PEGASUS_PLATFORM),SOLARIS_SPARC_CC)
EXTRA_LIBRARIES += -lCstd
endif

ifeq ($(PEGASUS_USE_SQLITE_REPOSITORY),true)
    ifeq ($(OS_TYPE),windows)
        SYS_LIBS += /libpath:$(SQLITE_HOME)/lib libsqlite3.lib
    else
        ifdef SQLITE_HOME
            EXTRA_LIBRARIES += -L$(SQLITE_HOME)/lib
        endif
        EXTRA_LIBRARIES += -lsqlite3
    endif
endif

##
##  PEP214 Compressed repository
##  ----------------------------
##
##    This PEP added the capability to utilize compression algorithms
##    on the repository. The compressed repository is approximatley 70%
##    smaller than a regular xml text based repository. This allows
##    significant disk space savings and also results in shorter file
##    load times.
##
##    The compression logic is installed as a conditionally compiled option.
##    To enable the compressed repository functionality:
##	1. Install zlib
##	2. define PEGASUS_ENABLE_COMPRESSED_REPOSITORY=on in the environment
##      3. build Pegasus.
##
##   Please refer to readme.compression in the main pegasus directory
##   for more information about repository compression.
##

ifdef PEGASUS_ENABLE_COMPRESSED_REPOSITORY
  LOCAL_DEFINES += -DPEGASUS_ENABLE_COMPRESSED_REPOSITORY

  ifeq ($(OS_TYPE),windows)
    EXTRA_LINK_FLAGS += -defaultlib:libz -libpath:/"Program Files"/GnuWin32/lib
    EXTRA_INCLUDES += -I/"Program Files"/GnuWin32/include
  else
    ifeq ($(OS_TYPE),vms)
       EXTRA_INCLUDES += -I/libz
    else
       EXTRA_LINK_FLAGS += -lz
    endif
  endif

endif

ifeq ($(OS_TYPE),windows)
##
## The following .PHONY rule gets around the problem in Windows for
## the "Program Files" directory name. The depends program strips the
## quotes when the depends file is built and upon subsequent compile
## it fails with no rule to make these fragments.
##
##
.PHONY: /Program Files/GnuWin32/include/zlib.h Files/GnuWin32/include/zconf.h
endif

STATIC=1
LIBRARY = pegrepository

SOURCES = \
    NameSpaceManager.cpp \
    ObjectCache.cpp \
    InheritanceTree.cpp \
    RepositoryDeclContext.cpp \
    RepositoryQueryContext.cpp \
    AutoStreamer.cpp \
    BinaryStreamer.cpp \
    Packer.cpp \
    XmlStreamer.cpp  

ifdef PEGASUS_ENABLE_MRR
    SOURCES += MRRSerialization.cpp
    SOURCES += MRRTypes.cpp
    SOURCES += CIMRepositoryMRR.cpp
else
    SOURCES += AssocClassTable.cpp
    SOURCES += AssocClassCache.cpp
    SOURCES += AssocInstTable.cpp
    SOURCES += InstanceIndexFile.cpp
    SOURCES += InstanceDataFile.cpp
    SOURCES += PersistentStore.cpp
    SOURCES += FileBasedStore.cpp
    SOURCES += CIMRepository.cpp
    ifeq ($(PEGASUS_USE_SQLITE_REPOSITORY),true)
        SOURCES += SQLiteStore.cpp
    endif
endif

include $(ROOT)/mak/library.mak

compress:
	make clean
	make depend PEGASUS_ENABLE_COMPRESSED_REPOSITORY=1
	make PEGASUS_ENABLE_COMPRESSED_REPOSITORY=1
