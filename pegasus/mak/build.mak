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

build: $(OBJ_DIR)/target $(BIN_DIR)/target $(LIB_DIR)/target $(ERROR)

define BUILD_NL


endef

ifeq ($(OS_TYPE),vms)

$(OBJ_DIR)/target: $(ERROR)
ifndef VMSPROGRAM
	library/create/object $(FULL_VMSLIB)
endif
	@ $(MKDIRHIER) $(REP_DIR)
	@ $(MKDIRHIER) $(OBJ_DIR)
	@ $(TOUCH) $(OBJ_VMSDIR)]target

$(BIN_DIR)/target: $(ERROR)
	@ $(MKDIRHIER) $(BIN_DIR)
	@ $(TOUCH) $(BIN_VMSDIR)]target

$(LIB_DIR)/target: $(ERROR)
	@ $(MKDIRHIER) $(LIB_DIR)
	@ $(TOUCH) $(LIB_VMSDIR)]target

$(OPT_DIR)/target: $(ERROR)
	@ $(MKDIRHIER) $(OPT_DIR)
	@ $(TOUCH) $(OPT_VMSDIR)]target
else
$(OBJ_DIR)/target: $(ERROR)
	@ $(MKDIRHIER) $(OBJ_DIR)
	$(foreach i, $(OBJECT_DIRS), $(MKDIRHIER) $(OBJ_DIR)/$(i) $(BUILD_NL) )
	@ $(TOUCH) $(OBJ_DIR)/target

$(BIN_DIR)/target: $(ERROR)
ifdef PEGASUS_TEST_VALGRIND_LOG_DIR
	@ $(MKDIRHIER) $(VALGRIND_SCRIPT_BIN_DIR)
endif
	@ $(MKDIRHIER) $(BIN_DIR)
	@ $(TOUCH) $(BIN_DIR)/target

$(LIB_DIR)/target: $(ERROR)
	@ $(MKDIRHIER) $(LIB_DIR)
	@ $(TOUCH) $(LIB_DIR)/target
endif
