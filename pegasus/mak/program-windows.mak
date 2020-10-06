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

TMP_OBJECTS = $(foreach i,$(SOURCES),$(OBJ_DIR)/$i)

S_OBJECTS = $(TMP_OBJECTS:.s=.obj)
CPP_OBJECTS = $(S_OBJECTS:.cpp=.obj)
OBJECTS = $(CPP_OBJECTS:.c=.obj)
VERSION_RC = $(ROOT)/mak/version-windows.RC
VERSION_RES = $(ROOT)/mak/version-windows.RES

FULL_PROGRAM=$(BIN_DIR)/$(PROGRAM)$(EXE)
FULL_PROGPDB=$(BIN_DIR)/$(PROGRAM)$(PDB)
FULL_PROGILK=$(BIN_DIR)/$(PROGRAM)$(ILK)
FULL_PROGLIB=$(BIN_DIR)/$(PROGRAM)$(LIB_SUFFIX)
FULL_PROGEXP=$(BIN_DIR)/$(PROGRAM)$(EXP)

EXE_OUTPUT = $(EXE_OUT)$(FULL_PROGRAM)

$(FULL_PROGRAM): $(OBJ_DIR)/target $(BIN_DIR)/target $(OBJECTS) $(VERSION_RES) $(FULL_LIBRARIES) $(ERROR)
ifeq ($(PEGASUS_SUPPORTS_DYNLIB),yes)

##
## build images with -l<name> syntax for needed shared libraries
## DYNAMIC_LIBRARIES is defined appropriately in libraries.mak and Makefile
## files
##

	$(LINK_WRAPPER) $(CXX) $(FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(VERSION_RES) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
else
	$(LINK_WRAPPER) $(CXX) $(FLAGS) $(EXTRA_LINK_FLAGS) $(EXE_OUTPUT) $(OBJECTS) $(VERSION_RES) $(RES_FILE) $(FULL_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
endif
	$(TOUCH) $(FULL_PROGRAM)
	@ $(ECHO)

$(VERSION_RES): $(VERSION_RC)
	@ $(RC) /fo $(VERSION_RES) $(VERSION_RC)

include $(ROOT)/mak/objects.mak

FILES_TO_CLEAN = $(OBJECTS) $(FULL_PROGRAM) $(FULL_PROGPDB) $(FULL_PROGILK) $(FULL_PROGLIB) $(FULL_PROGEXP) $(OBJ_DIR)/vc60$(PDB) $(OBJ_DIR)/vc70$(PDB)

include $(ROOT)/mak/clean.mak

-include $(ROOT)/mak/depend.mak

include $(ROOT)/mak/build.mak

include $(ROOT)/mak/sub.mak

-include $(OBJ_DIR)/depend.mak

include $(ROOT)/mak/misc.mak
