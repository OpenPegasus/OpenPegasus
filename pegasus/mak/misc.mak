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
misc:
	@ - bash $(ROOT)/mak/misc.sh

ifeq ($(OPTION),1)
general:
	@$(foreach i, $(SOURCES),  mu srclist -I$(PEGASUS_ROOT)/src  $(i);)
endif

ifeq ($(OPTION),3)
general:
	@$(foreach i, $(SOURCES),  echo pegasus/src/$(DIR)/$(i); grep -n -eATTN -eREVIEW -e" FIX" -eFIXME $(i);)
endif
ifeq ($(OPTION),4)
general:
	@$(foreach i, $(SOURCES),  echo pegasus/src/$(DIR)/$(i); grep $(EXP) $(i); cd .;)
endif

ifeq ($(OPTION),CORE_SEARCH)
CORE_FILES_IN_SRC_DIR = \
     $(wildcard $(PEGASUS_ROOT)/src/$(DIR)/$(PLATFORM_CORE_PATTERN))
general:
  ifneq ($(CORE_FILES_IN_SRC_DIR), )
	@$(ECHO) $(CORE_FILES_IN_SRC_DIR) >> $(CORE_FILE_LIST)
  endif
endif
