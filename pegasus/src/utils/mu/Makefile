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
DIR = utils/mu
include $(PEGASUS_ROOT)/mak/config.mak

PROGRAM = mu
MANIFEST = .manifest

OBJ_DIR = .
INCLUDES = -I$(PEGASUS_ROOT)/src $(EXTRA_INCLUDES)

#//When valgrind is used, all the binaries are wrapped around with
#//a shell script.Since mu is built before everything else is build,
#//it wont have the wrapper script. We copy the mu binary to the
#//bin directory to overcome this.

ACTUAL_BIN_DIR = $(PEGASUS_HOME)/bin


ifeq ($(OS_TYPE),windows)
    PLATFORM_SOURCES = FilesWindows.cpp
    # Windows linker does not allow space before pgm name
    EXE_OUTPUT = $(EXE_OUT)$(PROGRAM)$(EXE)
    DEFINES += -DOS_WINDOWS
    RM = del
    BACKWARD_SLASH = 1
    MU_EXE_LOCATION = $(subst /,\,$(BIN_DIR)\$(PROGRAM)$(EXE))
    MU_EXE_MANIFEST_LOCATION = $(subst /,\,$(BIN_DIR)\$(PROGRAM)$(EXE)$(MANIFEST))
else
    PLATFORM_SOURCES = FilesUnix.cpp
    # some platforms (Mac) require space before pgm name
    EXE_OUTPUT = $(EXE_OUT) $(PROGRAM)$(EXE)
    OBJ_OUT = -o
    RM = rm -f
    MU_EXE_LOCATION = $(BIN_DIR)/$(PROGRAM)$(EXE)
endif

ifdef PEGASUS_MU_BIN_DIR
    ifdef BACKWARD_SLASH
       FULL_PROGRAM = $(PEGASUS_MU_BIN_DIR)\$(PROGRAM)$(EXE)
       FULL_PROGRAM_MANIFEST = $(PEGASUS_MU_BIN_DIR)\$(PROGRAM)$(EXE)$(MANIFEST)
    else
       FULL_PROGRAM = $(PEGASUS_MU_BIN_DIR)/$(PROGRAM)$(EXE)
       FULL_PROGRAM_MANIFEST = $(PEGASUS_MU_BIN_DIR)/$(PROGRAM)$(EXE)$(MANIFEST)
    endif
else
    FULL_PROGRAM = $(PROGRAM)$(EXE)
    FULL_PROGRAM_MANIFEST = $(PROGRAM)$(EXE)$(MANIFEST)
endif


SOURCES = \
    $(PLATFORM_SOURCES)     \
    DependCmd.cpp           \
    PwdCmd.cpp              \
    CompareCmd.cpp          \
    MoveCmd.cpp             \
    CopyCmd.cpp             \
    TouchCmd.cpp            \
    EchoCmd.cpp             \
    MkDirHierCmd.cpp        \
    RmDirHierCmd.cpp        \
    RmCmd.cpp               \
    Files.cpp               \
    mu.cpp                  \
    StripCmd.cpp            \
    PrependCmd.cpp          \
    SleepCmd.cpp            \
    SortCmd.cpp             \
    SrcListCmd.cpp          \
    Dependency.cpp          \
    ReplaceCmd.cpp

OBJECTS = $(SOURCES:.cpp=$(OBJ))

%$(OBJ): %.cpp
ifeq ($(OS),zos)
	$(CXX) -c -o $(PEGASUS_PLATFORM).o $(FLAGS) $(EXTRA_CXX_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.cpp > $*.lst
	@ $(ZIP) -a -m $@.lst.zip $*.lst
	@ $(COPY) $(PEGASUS_PLATFORM).o $@
	@ $(RM) $(PEGASUS_PLATFORM).o
	@ $(TOUCH) $@
	@ $(ECHO)
else
	$(CXX) -c $(OBJ_OUT)$@ $(FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.cpp
endif


$(FULL_PROGRAM): $(OBJECTS)
ifeq ($(OS),zos)
	$(LINK_WRAPPER) $(CXX) $(PR_FLAGS) $(EXTRA_LINK_FLAGS) -L$(LIB_DIR) $(EXE_OUTPUT) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(SYS_LIBS) $(EXTRA_LIBRARIES) > $(PROGRAM).llst
	@ $(ZIP) -a -m $(FULL_PROGRAM).llst.zip $(PROGRAM).llst
else
	$(LINK_WRAPPER) $(CXX) $(FLAGS) $(EXE_OUTPUT) $(OBJECTS) $(FULL_LIBRARIES) $(SYS_LIB)
endif
	$(MKDIRHIER) $(BIN_DIR)
	-$(RM) $(MU_EXE_LOCATION)
ifeq ($(OS_TYPE),windows)
	-$(RM) $(MU_EXE_MANIFEST_LOCATION)
	-$(COPY) $(FULL_PROGRAM_MANIFEST) $(MU_EXE_MANIFEST_LOCATION)
endif
	$(COPY) $(FULL_PROGRAM) $(MU_EXE_LOCATION)
ifdef PEGASUS_TEST_VALGRIND_LOG_DIR
	$(MKDIRHIER) $(ACTUAL_BIN_DIR)
	$(COPY) $(FULL_PROGRAM) $(ACTUAL_BIN_DIR)/$(FULL_PROGRAM)
endif

clean:
	$(RM) $(OBJECTS)
	$(RM) $(FULL_PROGRAM)
	-$(RM) $(MU_EXE_LOCATION)
ifeq ($(OS_TYPE),windows)
	-$(RM) $(MU_EXE_MANIFEST_LOCATION)
endif
ifdef PEGASUS_TEST_VALGRIND_LOG_DIR
	$(RM) $(ACTUAL_BIN_DIR)/$(FULL_PROGRAM)
endif

tests:

poststarttests:
