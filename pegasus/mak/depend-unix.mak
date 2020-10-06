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
DEPEND_MAK = $(OBJ_DIR)/depend.mak

ifeq ($(ARCHITECTURE),PARISC)
  SOURCES_NO_ASM = $(patsubst %.s,,$(SOURCES))
else
  SOURCES_NO_ASM = $(SOURCES)
endif

##
## test for compiler definition in CXX (g++, clang++, etc. compilers).
## Test for compiler type  as substring, not exact because there
## are cases where other info exists in CXX (ex. CXX=ccache g++)
##
ifeq ($(findstring g++, $(CXX)), g++)
    PEGASUS_CXX_MAKEDEPEND_OPTION = -M
endif

ifeq ($(CXX), clang++)
    PEGASUS_CXX_MAKEDEPEND_OPTION = -M
    C_SOURCES = $(filter %.c, $(SOURCES))
endif

ifeq ($(CXX), CC)
    PEGASUS_CXX_MAKEDEPEND_OPTION = -xM1
endif
ifeq ($(CXX), aCC)
    PEGASUS_CXX_MAKEDEPEND_OPTION = +Make -E
    acc_sed_filter = -e 's=$(OBJ_DIR).*cpp:==g'
endif

##
## If compiler definition found, use compiler to define depend.mak
## each obj dir
##
ifdef PEGASUS_CXX_MAKEDEPEND_OPTION

ifeq ($(COMPILER), clang)
ifneq ($(strip $(C_SOURCES)),)
depend: $(OBJ_DIR)/target $(ERROR)
	$(CC) $(PEGASUS_CXX_MAKEDEPEND_OPTION) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $(SOURCES_NO_ASM) | sed -e 's=^\(.*:\)='$(OBJ_DIR)'/\1=' > $(DEPEND_MAK)
else
depend: $(OBJ_DIR)/target $(ERROR)
	$(CXX) $(PEGASUS_CXX_MAKEDEPEND_OPTION) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $(SOURCES_NO_ASM) | sed -e 's=^\(.*:\)='$(OBJ_DIR)'/\1=' > $(DEPEND_MAK)
endif
else
  depend: $(OBJ_DIR)/target $(ERROR)
	$(CXX) $(PEGASUS_CXX_MAKEDEPEND_OPTION) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $(SOURCES_NO_ASM) | sed -e 's=^\(.*:\)='$(OBJ_DIR)'/\1=' $(acc_sed_filter) > $(DEPEND_MAK)
endif

##
## Compiler type not found.   If PEGASUS_HAS_MAKEDEPEND set use makedepend
## to define depend.mak files.  Otherwise use OpenPegasus mu to create
## depend.make files
##
else
ifdef PEGASUS_HAS_MAKEDEPEND
DEPEND_INCLUDES += -DPEGASUS_OS_TYPE_UNIX -I/usr/include $(SYS_INCLUDES)

depend: $(OBJ_DIR)/target $(ERROR)
	touch $(DEPEND_MAK)
	makedepend -v $(LOCAL_DEFINES) $(DEFINES) $(DEPEND_DEFINES) $(PRE_DEPEND_INCLUDES) $(DEPEND_INCLUDES) $(INCLUDES) $(SOURCES_NO_ASM) -f $(DEPEND_MAK) -p $(OBJ_DIR)/
else
depend: $(OBJ_DIR)/target $(ERROR)
	mu depend -O$(OBJ_DIR) $(INCLUDES) $(SOURCES_NO_ASM) > $(DEPEND_MAK)
endif
endif
clean-depend:
	$(RM) $(OBJ_DIR)/depend.mak
