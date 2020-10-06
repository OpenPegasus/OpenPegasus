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
## An ugly trick is used here to overcome a bug in g++ v2.9.5. G++ fails to
## cleanup object files that are placed in directories other than the
## current one (using the -o option). To overcome this bug, we use -o.tmp.o
## and then move the file to the object directory.  Note, this is only done for
## v2.9.5 of g++.
##
################################################################################

ifeq ($(findstring _GNU, $(PEGASUS_PLATFORM)), _GNU)
ifneq ($(GCC_VERSION), 2.9.5)
    _NO_TMP_O=yes
endif
else
    _NO_TMP_O=yes
endif

_TMP_O = $(PEGASUS_PLATFORM).o


ifeq ($(PEGASUS_PLATFORM),HPUX_PARISC_GNU)
ifeq "$(wildcard LoadAndClearWord_HPUX_PARISC_ACC.s)" "LoadAndClearWord_HPUX_PARISC_ACC.s"
$(OBJ_DIR)/LoadAndClearWord_HPUX_PARISC_ACC.o: $(OBJ_DIR)/target LoadAndClearWord_HPUX_PARISC_ACC.s $(ERROR)
	aCC -c -o $(OBJ_DIR)/LoadAndClearWord_HPUX_PARISC_ACC.o $(GNU_LINK_SEARCH_PATH) +Z +DAportable -mt -D_PSTAT64 +DD64 \
		$(EXTRA_C_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) LoadAndClearWord_HPUX_PARISC_ACC.s
	@ $(TOUCH) $@
	@ $(ECHO)
endif
endif

ifeq ($(_NO_TMP_O), yes)
$(OBJ_DIR)/%.o: $(OBJ_DIR)/target %.cpp $(ERROR)
	$(CXX) -c -o $@ $(FLAGS) $(EXTRA_CXX_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.cpp
	@ $(TOUCH) $@
	@ $(ECHO)
else
$(OBJ_DIR)/%.o: %.cpp $(ERROR)
	$(CXX) -c -o $(_TMP_O) $(FLAGS) $(EXTRA_CXX_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.cpp
	@ $(COPY) $(_TMP_O) $@
	@ $(RM) $(_TMP_O)
	@ $(TOUCH) $@
	@ $(ECHO)
endif

ifeq ($(_NO_TMP_O), yes)
$(OBJ_DIR)/%.o: $(OBJ_DIR)/target %.c $(ERROR)
	$(CC) -c -o $@ $(FLAGS) $(EXTRA_C_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.c
	@ $(TOUCH) $@
	@ $(ECHO)
else
$(OBJ_DIR)/%.o: %.c $(ERROR)
	$(CC) -c -o $(_TMP_O) $(FLAGS) $(EXTRA_C_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.c
	@ $(COPY) $(_TMP_O) $@
	@ $(RM) $(_TMP_O)
	@ $(TOUCH) $@
	@ $(ECHO)
endif

ifeq ($(_NO_TMP_O), yes)
$(OBJ_DIR)/%.o: $(OBJ_DIR)/target %.s $(ERROR)
	$(CC) -c -o $@ $(FLAGS) $(EXTRA_C_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.s
	@ $(TOUCH) $@
	@ $(ECHO)
else
$(OBJ_DIR)/%.o: %.s $(ERROR)
	$(CC) -c -o $(_TMP_O) $(FLAGS) $(EXTRA_C_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.s
	@ $(COPY) $(_TMP_O) $@
	@ $(RM) $(_TMP_O)
	@ $(TOUCH) $@
	@ $(ECHO)

endif
