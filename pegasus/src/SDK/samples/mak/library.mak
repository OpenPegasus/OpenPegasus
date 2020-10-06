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
include $(ROOT)/mak/common.mak

PATHED_LIBRARY = $(PEGASUS_SAMPLE_LIB_DIR)/lib$(LIBRARY).$(PLATFORM_LIB_SUFFIX)
TMP_LIST_STEP1 = $(foreach i,$(SOURCES),$(PEGASUS_SAMPLE_OBJ_DIR)/$i)
TMP_LIST_STEP2 = $(TMP_LIST_STEP1:.cpp=$(OBJ_SUFFIX))
PATHED_OBJECTS = $(TMP_LIST_STEP2:.c=$(OBJ_SUFFIX))

$(PEGASUS_SAMPLE_OBJ_DIR)/%$(OBJ_SUFFIX): %.c
	$(COMPILE_C_COMMAND) -c -o $@ $(LIBRARY_COMPILE_OPTIONS) \
             -I $(PEGASUS_INCLUDE_DIR) $(DEFINES) $*.c

$(PEGASUS_SAMPLE_OBJ_DIR)/%$(OBJ_SUFFIX): %.cpp
	$(COMPILE_CXX_COMMAND) -c -o $@  $(LIBRARY_COMPILE_OPTIONS) \
             -I $(PEGASUS_INCLUDE_DIR) $(DEFINES) $*.cpp

$(PATHED_LIBRARY): $(PEGASUS_SAMPLE_OBJ_DIR)/target $(PEGASUS_SAMPLE_LIB_DIR)/target $(PATHED_OBJECTS) Makefile $(ROOT)/mak/library.mak
	$(LIBRARY_LINK_COMMAND) $(LIBRARY_LINK_OPTIONS) $(DEFINES) \
             $(LINK_OUT)$@ $(PATHED_OBJECTS) $(SYS_LIBS) \
             $(DYNAMIC_LIBRARIES) $(EXTRA_LINK_ARGUMENTS)
ifneq ($(PEGASUS_PROVIDER_LIBRARY),false)
	$(MAKE) -i unlink
	ln -f -s $(PATHED_LIBRARY) $(SYM_LINK_LIB).$(PLATFORM_LIB_SUFFIX)
endif

unlink:
	$(RM) $(SYM_LINK_LIB).$(PLATFORM_LIB_SUFFIX)

clean:
	$(foreach i, $(PATHED_OBJECTS), $(RM) $(i);)
ifneq ($(PEGASUS_PROVIDER_LIBRARY),false)
	$(MAKE) -i unlink
endif
	$(RM) $(PATHED_LIBRARY)
	$(foreach i, $(XMLRESPONSES), $(RM) $(i);)
	$(foreach i, $(ADDITIONAL_CLEAN_FILES), $(RM) $(i);)

depend:

tests:

poststarttests:

include $(ROOT)/mak/build.mak
	
