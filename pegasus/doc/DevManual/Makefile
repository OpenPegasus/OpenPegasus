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

DIR = DevManual
include $(ROOT)/mak/config.mak

DOC_ROOT = $(ROOT)/doc
DOC_DIR = $(DOC_ROOT)/$(DIR)
DOC_OUTPUT = $(HOME_DIR)/$(DIR)html
DOC_HEAD = PegasusDoc.dxx


all:
	$(RM) $(DOC_OUTPUT)/*
	gawk --file=PegasusDocTOC.awk $(DOC_HEAD) >pegdocsidebarleft.shtml
	docxx --dir $(DOC_OUTPUT)  --verbose -H $(DOC_HEAD)
	$(COPY) $(DOC_DIR)/*.gif $(DOC_OUTPUT)
	$(COPY) $(DOC_DIR)/*.htm $(DOC_OUTPUT)
	$(COPY) $(DOC_DIR)/*.html $(DOC_OUTPUT)
	$(COPY) $(DOC_DIR)/*.shtml $(DOC_OUTPUT)	
	$(RM) pegdocsidebarleft.shtml
clean:
	$(RM) $(DOC_OUTPUT)/*
depend:

tests:
