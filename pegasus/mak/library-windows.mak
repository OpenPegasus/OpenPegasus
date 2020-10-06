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
FULL_LIB = $(LIB_DIR)/$(LIBRARY)$(LIB_SUFFIX)
FULL_DLL = $(BIN_DIR)/$(LIBRARY)$(DLL)
FULL_EXP = $(BIN_DIR)/$(LIBRARY)$(EXP)
FULL_ILK = $(BIN_DIR)/$(LIBRARY)$(ILK)
FULL_PDB = $(BIN_DIR)/$(LIBRARY)$(PDB)
VERSION_RC = $(ROOT)/mak/version-windows.RC
VERSION_RES = $(ROOT)/mak/version-windows.RES

##
## ws2_32.lib is needed to get the WINSOCK routines!
##

##
## The next four lines define a variable (called NL) that contains a simple
## newline. The two blank lines are needed (please do not remove).
##
define NL


endef

##
## The name of the link file to be used by the Windows link command. This is
## only used when SOURCES1 is defined.
##
LINKFILE=$(LIB_DIR)/linkfile

$(FULL_LIB): $(BIN_DIR)/target $(LIB_DIR)/target $(OBJ_DIR)/target $(OBJECTS) $(VERSION_RES) $(FULL_LIBRARIES) $(ERROR)
ifdef SOURCES1
	@ $(ECHO) "Creating $(LINKFILE)"
	@ $(RM) $(LINKFILE)
	@ $(foreach i, $(OBJECTS), echo $(i) >> $(LINKFILE) $(NL) )
	link -nologo -dll $(LINK_FLAGS) $(EXTRA_LINK_FLAGS) -out:$(FULL_DLL) -implib:$(FULL_LIB) @$(LINKFILE) $(VERSION_RES) $(FULL_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
else
	link -nologo -dll $(LINK_FLAGS) $(EXTRA_LINK_FLAGS) -out:$(FULL_DLL) -implib:$(FULL_LIB) $(OBJECTS) $(VERSION_RES) $(FULL_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES)
endif

FILES_TO_CLEAN = \
    $(OBJECTS) $(FULL_LIB) $(FULL_DLL) $(FULL_EXP) $(FULL_ILK) $(FULL_PDB) $(OBJ_DIR)/vc60$(PDB) $(OBJ_DIR)/vc70$(PDB) depend.mak depend.mak.bak

$(VERSION_RES): $(VERSION_RC)
	@ $(RC) /fo $(VERSION_RES) $(VERSION_RC)

clean-lib: $(ERROR)
	mu rm $(FULL_LIB) $(FULL_DLL) $(FULL_EXP)
