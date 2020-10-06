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

################################################################################
##
## Build list of bundle names.
##
###############################################################################

# If we are using ICU resource bundles, then the compiled bundles are .res files
# since the individual makefiles do not specify a root package file, one is 
# appended to the target list
ifeq ($(PEGASUS_HAS_ICU), true)
	TMP_MSG_BUNDLES = $(foreach i,$(MSG_SOURCES),$(MSG_DIR)/$i)
	MSG_BUNDLES = $(TMP_MSG_BUNDLES:.txt=.res)
	MSG_BUNDLES += $(MSG_DIR)/$(MSG_PACKAGE)_root.res
endif

################################################################################
##
## Message compile rules: the normal rule is from package_lang.txt -> package_lang.res
##   the exception to the rule: since package_root.txt does not exist,
##   this defaults to package_en.txt unless the appropriate vars are set.
##   the selected/or default package file is processed and converted into
##   an intermediate package_root.rb file, this file is then compiled into 
##   its corresponding package_root.res file
##
################################################################################

# Rule for ICU resource bundles
ifeq ($(PEGASUS_HAS_ICU), true)
messages: $(MSG_BUNDLES)

$(MSG_DIR)/$(MSG_PACKAGE)_root.res: $(MSG_PACKAGE)_root.rb $(ERROR)
	$(MKDIRHIER) $(MSG_DIR)
	$(MSG_COMPILE) -p $(MSG_PACKAGE) -d $(MSG_DIR) $(MSG_FLAGS) $(MSG_PACKAGE)_root.rb
	@ $(ECHO)

$(MSG_PACKAGE)_root.rb: $(MSG_PACKAGE)$(MSG_ROOT_SOURCE).txt $(ERROR)
	$(MAKE) --directory=$(PEGASUS_ROOT)/src/utils/cnv2rootbundle -f Makefile
	$(CNV_ROOT_CMD) -u $(MSG_PACKAGE)$(MSG_ROOT_SOURCE).txt

$(MSG_DIR)/%.res: %.txt $(ERROR)
	$(MKDIRHIER) $(MSG_DIR)
	$(MSG_COMPILE) -p $(MSG_PACKAGE) -d $(MSG_DIR) $(MSG_FLAGS) $*.txt
	@ $(ECHO)

endif

################################################################################
##
## Clean rules:
##
################################################################################

# these files are cleaned when a user runs make clean, the intermediate package_root.rb file is removed
# from the source directory
FILES_TO_CLEAN = $(MSG_BUNDLES)
FILES_TO_CLEAN += $(ROOT)/src/$(DIR)/*.rb

ifeq ($(PEGASUS_HAS_ICU), true)
include $(ROOT)/mak/clean.mak
else
clean:
endif

depend:

general:

################################################################################
##
## Build list of object names:
##
################################################################################


tests: $(ERROR)

poststarttests: $(ERROR)

