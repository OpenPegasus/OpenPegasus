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

DIR = Executor/tests/lib

HAS_PAM_DEPENDENCY=true

# The source files in the Executor directory are built twice, once
# as part of the Executor executable (cimserver) and once as part
# of the a test library (pegexecutor). The object files for the
# Executor executable are placed in $PEGASUS_HOME/obj/Executor.
# To avoid conflict with these objects, the ALT_OBJ_DIR variable
# is used to define an alternative object directory for the objects
# used to build the test library.
#
# Need two more levels since ../.. will remove two levels.
ALT_OBJ_DIR = Executor/tests/lib/tests/lib

include $(ROOT)/mak/config.mak

LOCAL_DEFINES = -DEXECUTOR_PLEASE_EXPORT

LIBRARY = pegexecutor

SOURCES = \
    ../../Assert.c \
    ../../Child.c \
    ../../Config.c \
    ../../Exit.c \
    ../../Fatal.c \
    ../../FileHandle.c \
    ../../Globals.c \
    ../../LocalAuth.c \
    ../../Log.c \
    ../../Macro.c \
    ../../Match.c \
    ../../Messages.c \
    ../../Options.c \
    ../../Parent.c \
    ../../PasswordFile.c \
    ../../Path.c \
    ../../Policy.c \
    ../../Process.c \
    ../../Random.c \
    ../../Socket.c \
    ../../User.c

include ../common.mak
include $(ROOT)/mak/library.mak
