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
INCLUDES = -I$(ROOT)/src $(EXTRA_INCLUDES)

ifndef LINK_RPATH
    LINK_RPATH = -Xlinker -rpath
endif

LINK_DEST_LIB = -Xlinker $(PEGASUS_DEST_LIB_DIR)

LINK_LIB_DIR = -Xlinker $(LIB_DIR)

ifndef LINK_RPATH_LINK
    LINK_RPATH_LINK = -Xlinker -rpath-link
endif


TMP_OBJECTS = $(foreach i,$(SOURCES),$(OBJ_DIR)/$i)

S_OBJECTS = $(TMP_OBJECTS:.s=.o)
CPP_OBJECTS = $(S_OBJECTS:.cpp=.o)
OBJECTS = $(CPP_OBJECTS:.c=.o)

FULL_PROGRAM=$(BIN_DIR)/$(PROGRAM)$(EXE)

EXE_OUTPUT = $(EXE_OUT) $(FULL_PROGRAM)

ifdef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
    ifdef PEGASUS_USE_RELEASE_DIRS
        EXTRA_LINK_FLAGS += $(LINK_RPATH) $(LINK_DEST_LIB) $(LINK_RPATH_LINK) $(LINK_LIB_DIR)
    else
        EXTRA_LINK_FLAGS += $(LINK_RPATH) $(LINK_LIB_DIR)
    endif

    ifeq ($(HAS_ICU_DEPENDENCY),true)
        ifdef ICU_INSTALL
            EXTRA_LINK_FLAGS += $(LINK_RPATH) -Xlinker ${ICU_INSTALL}/lib
        endif
    endif
endif

ifdef PEGASUS_PURIFY
    PUREOPTIONS = -follow-child-processes=yes -locking=no \
        -always-use-cache-dir -cache-dir=$(PURIFY_TMP)/cache \
        -view-file=$(PURIFY_TMP)/$(PROGRAM).pv \
        -log-file=$(PURIFY_TMP)/$(PROGRAM).log
    LINK_WRAPPER = $(PURIFY_HOME)/purify $(PUREOPTIONS)
    SYS_LIBS += $(PURIFY_HOME)/libpurify_stubs.a
endif

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
  FLAGS += -Wl,-brtl -Wl,-bhalt:$(AIX_LD_HALT)
endif

$(FULL_PROGRAM): $(OBJ_DIR)/target $(BIN_DIR)/target $(OBJECTS) $(FULL_LIBRARIES) $(ERROR)
ifeq ($(PEGASUS_SUPPORTS_DYNLIB),yes)

##
## build images with -l<name> syntax for needed shared libraries
## DYNAMIC_LIBRARIES is defined appropriately in libraries.mak and Makefile
## files
##
## ICU_INSTALL - Specifies the directory path where the ICU lib directory is located.
##               This will set runtime library search path for ICU libraries to ${ICU_INSTALL}/lib
##
     ifeq ($(OS),zos)
	$(LINK_WRAPPER) $(CXX) $(PR_FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES) > $(PROGRAM).llst
	@ $(ZIP) -a -m $(FULL_PROGRAM).llst.zip $(PROGRAM).llst
     else
      ifdef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
	$(LINK_WRAPPER) $(CXX) $(FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
      else
       ifeq ($(PEGASUS_PLATFORM),AIX_RS_IBMCXX)
         ifdef  PEGASUS_USE_RELEASE_DIRS
           ifdef PEGASUS_HAS_MESSAGES
             ifdef ICU_INSTALL
	       $(LINK_WRAPPER) $(CXX) -Wl,-brtl -blibpath:/usr/lib:/lib:$(ICU_INSTALL)/lib -Wl,-bhalt:$(AIX_LD_HALT) $(FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
             endif
           else
	     $(LINK_WRAPPER) $(CXX) -Wl,-brtl -blibpath:/usr/lib:/lib -Wl,-bhalt:$(AIX_LD_HALT) $(FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
           endif
         else
	   $(LINK_WRAPPER) $(CXX) -Wl,-brtl -Wl,-bhalt:$(AIX_LD_HALT) $(FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
         endif
       else
	$(LINK_WRAPPER) $(CXX) $(FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
       endif
      endif
     endif
else
	$(LINK_WRAPPER) $(CXX) $(FLAGS) $(EXTRA_LINK_FLAGS) $(EXE_OUTPUT) $(OBJECTS) $(FULL_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
endif
	$(TOUCH) $(FULL_PROGRAM)
ifdef PEGASUS_TEST_VALGRIND_LOG_DIR
	echo "#!/bin/bash" > $(VALGRIND_SCRIPT_BIN_DIR)/$(PROGRAM)$(EXE)
	echo -e "valgrind --log-file=$(PEGASUS_TEST_VALGRIND_LOG_DIR)/$(PROGRAM) --num-callers=25 --tool=memcheck --leak-check=full --error-limit=no $(FULL_PROGRAM) \"\x24@\"" >> $(VALGRIND_SCRIPT_BIN_DIR)/$(PROGRAM)$(EXE)
	chmod 755 $(VALGRIND_SCRIPT_BIN_DIR)/$(PROGRAM)$(EXE)
endif
	@ $(ECHO)

include $(ROOT)/mak/objects.mak

FILES_TO_CLEAN = $(OBJECTS) $(FULL_PROGRAM)

include $(ROOT)/mak/clean.mak

-include $(ROOT)/mak/depend.mak

include $(ROOT)/mak/build.mak

include $(ROOT)/mak/sub.mak

-include $(OBJ_DIR)/depend.mak

include $(ROOT)/mak/misc.mak
