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
# Name for the Pegasus compiler.

CIMMOFCLI = cimmofl

# The following variables determine the version of the Schema to
# be loaded into Namespaces.
# Update the following environment variables to change the version.

PG_INTEROP_SCHEMA_VER=20
PG_INTEROP_SCHEMA_DIR=VER$(PG_INTEROP_SCHEMA_VER)
PG_INTEROP_MOF_PATH=$(ROOT)/Schemas/Pegasus/InterOp/$(PG_INTEROP_SCHEMA_DIR)


ifeq ($(PEGASUS_INTEROP_NAMESPACE),root/interop)
INTEROPNS=root/interop
else
ifeq ($(PEGASUS_INTEROP_NAMESPACE),interop)
INTEROPNS=interop
else
INTEROPNS=root/PG_InterOp
endif
endif


# The INTERNALNS variable defines the absolute name of the
# Pegasus INTERNALNS Namespace.

INTERNALNS = root/PG_Internal

# The MANAGEDSYSTEMNS variable defines the absolute name of the
# namespace for the managed system running the CIMOM.  The content and
# the name of this namespace is expected to be platform specific.

MANAGEDSYSTEMNS = NOTDEFINED

ifeq ($(OS), HPUX)
#   MANAGEDSYSTEMNS = root/HPV2
endif

ifeq ($(MANAGEDSYSTEMNS),NOTDEFINED)
   MANAGEDSYSTEMNS = root/cimv2
endif
