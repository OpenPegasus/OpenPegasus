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

CLEAN_DIRS = $(DIRS) $(RECURSE_EXTRA_CLEAN_DIRS)

define NL


endef

##
## PEGASUS_SKIP_MOST_TEST_DIRS -- disables building of tests if "true".
##

ifdef PEGASUS_SKIP_MOST_TEST_DIRS
  ifeq ($(PEGASUS_SKIP_MOST_TEST_DIRS),false)
    DIRS += $(TEST_DIRS)
  else
    ifneq ($(PEGASUS_SKIP_MOST_TEST_DIRS),true)
      $(error PEGASUS_SKIP_MOST_TEST_DIRS ($(PEGASUS_SKIP_MOST_TEST_DIRS)) invalid, must be true or false)
    endif
  endif
else
  DIRS += $(TEST_DIRS)
endif

#
# In VMS, arguments that must be in uppercase,
#  need to be surrounded by double quotes.
#

all: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) $(NL) )

depend: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) depend $(NL) )

tests: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) tests $(NL) )

poststarttests: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) poststarttests $(NL) )

clean: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(CLEAN_DIRS), $(MAKESH) $(MAKE) "-SC" $(i) clean $(NL))

misc: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(CLEAN_DIRS), $(MAKESH) $(MAKE) "-SC" $(i) misc $(NL))

docxx: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(CLEAN_DIRS), $(MAKESH) $(MAKE) "-SC" $(i) docxx $(NL))

sub: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(CLEAN_DIRS), $(MAKESH) $(MAKE) "-SC" $(i) sub $(NL))

general: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) general OPTION=$(OPTION) $(NL))

strip-license: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) strip-license $(NL) )

prepend-license: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) prepend-license $(NL) )

#l10n
messages: $(RECURSE_DEPENDS) $(ERROR)
	@ $(foreach i, $(DIRS), $(MAKESH) $(MAKE) "-SC" $(i) messages $(NL) )

