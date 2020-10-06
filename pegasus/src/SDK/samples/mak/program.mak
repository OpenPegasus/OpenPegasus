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

PATHED_PROGRAM = $(PEGASUS_SAMPLE_BIN_DIR)/$(PROGRAM)
TMP_LIST_STEP1 = $(foreach i,$(SOURCES),$(PEGASUS_SAMPLE_OBJ_DIR)/$i)
TMP_LIST_STEP2 = $(TMP_LIST_STEP1:.cpp=$(OBJ_SUFFIX))
PATHED_OBJECTS = $(TMP_LIST_STEP2:.c=$(OBJ_SUFFIX))

$(PEGASUS_SAMPLE_OBJ_DIR)/%.o: %.cpp
	$(COMPILE_CXX_COMMAND) $(PROGRAM_COMPILE_OPTIONS) -c -o $@ -I $(PEGASUS_INCLUDE_DIR) $(DEFINES) $*.cpp

$(PATHED_PROGRAM): $(PEGASUS_SAMPLE_OBJ_DIR)/target $(PEGASUS_SAMPLE_BIN_DIR)/target $(PATHED_OBJECTS)
	$(PROGRAM_LINK_COMMAND) $(PROGRAM_LINK_OPTIONS) $(LINK_OUT)$@ $(PATHED_OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS)

clean:
	@$(foreach i, $(PATHED_OBJECTS), $(RM) $(i);)
	@$(foreach i, $(PATHED_PROGRAM), $(RM) $(i);)
	@$(foreach i, $(ADDITIONAL_CLEAN_FILES), $(RM) $(i);)

depend:

tests:

poststarttests:

include $(ROOT)/mak/build.mak

