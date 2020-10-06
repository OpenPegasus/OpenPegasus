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
# Makefile to create documentation from nroff formatted for manpages
# This makefile is not connected to the Pegasus Make environment. It must be
# executed manually.
#  It creates the ascii and html manpage files from an NROFF source
# (cimcli.nroff).  Note that it requires utilities probably only found in
# Linux/Unix environments including:
# groff, possibly nroff, and possibly other linux tools for the view* targets.
# This Makefile intended to be used only by the author(s) of cimcli
#
ROOT = ../../../..

include $(ROOT)/mak/config.mak

GROFFOPTIONS=man

DOCUMENT=cimcli
SOURCE=$(ROOT)/rpm/manLinux/man1.Z/$(DOCUMENT).1

## generate all targets.
all: manpage html text

## generate the manpage output.
manpage:
	nroff -$(GROFFOPTIONS) $(SOURCE) >$(DOCUMENT)

## generate html output.
html:
	groff -$(GROFFOPTIONS) -Thtml $(SOURCE) > $(DOCUMENT).html

## manual option for creating html. Sometimes the created html is
## formatted better than with groff and the-Thtml option
man2html:
	man2html <$(SOURCE) >$(DOCUMENT).html

## Produce pure ASCII text output.
text:
	groff -$(GROFFOPTIONS) -Tascii $(SOURCE) | col -b > $(DOCUMENT).txt

## Alternate text output generator since the ascii option on groff not
## universal.
## This operation includes sed processing to remove a unicode equivalent
## of - (/xe2/x80/x90) that nroff produces.  See bug 8945 for more details.
## Uses sed hex character definition which may not be available on all versions
## of unix and linux (sed 3.02 on linux works). Alternate from for the
## search is sed 's/'`echo "\342\200\220"`'/-/g' > $(DOCUMENT).txt
textnroff:
	nroff -$(GROFFOPTIONS) $(SOURCE) | col -b \
	| sed 's/\xE2\x80\x90/-/g' > $(DOCUMENT).txt

clean:
	$(RM) $(DOCUMENT) $(DOCUMENT).html $(DOCUMENT).txt

## The following targets are for developer use to test the various outputs
## of the txt, manpage, and html generation.
## view nroff output for text. This is a test function
viewnroff:
	nroff -$(GROFFOPTIONS) $(SOURCE) | col -b | less

viewgrofftext:
	groff -$(GROFFOPTIONS) -Tascii $(SOURCE) | col -b | less

## view groff output.  Requires. linux groff package
viewgroff:
	groff -$(GROFFOPTIONS) -X $(SOURCE)

