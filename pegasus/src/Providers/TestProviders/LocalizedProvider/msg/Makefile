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
# This is an example of the 'standard' Makefile to compile ICU resource
# bundles.

ROOT = ../../../../..

DIR = Providers/TestProviders/LocalizedProvider/msg

include $(ROOT)/mak/config.mak

# ICU package name for genrb
MSG_PACKAGE = localizedProvider

# Destination directory for the compiled bundles
# Note: MSG_ROOT is set to PEGASUS_HOME/msg by default.
MSG_DIR = $(MSG_ROOT)/provider/localizedProvider

# The list of resource bundles to compile.
# NOTE - your .txt files must be named with the package name before the '_'
MSG_SOURCES = \
    localizedProvider_en.txt \
    localizedProvider_fr.txt \
    localizedProvider_de.txt\
    localizedProvider_es.txt 

# Keep the following in the order listed, and at the end of the file

include $(ROOT)/mak/message.mak

messages: $(MSG_BUNDLES)

# DO NOT DELETE
