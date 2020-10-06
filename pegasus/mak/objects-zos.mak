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
## An ugly trick is used here to overcome a bug in g++ 2.9.5. G++ failes to
## cleanup object files that are placed in directories other than the
## current one (using the -o option). To overcome this bug, we use -o.tmp.o
## and then move the file to the object directory.
##
################################################################################

_TMP_O = $(PEGASUS_PLATFORM).o

# prelinker support

ifdef PEGASUS_ZOS_PROGRAM_OBJECT
FLAGS = $(PR_FLAGS)
endif

$(OBJ_DIR)/%.o: %.cpp $(ERROR)
	$(CXX) -c -o $(_TMP_O) $(FLAGS) $(EXTRA_CXX_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.cpp > $*.lst
	@ $(ZIP) -a -m $@.lst.zip $*.lst
	@ $(COPY) $(_TMP_O) $@
	@ $(RM) $(_TMP_O)
	@ $(TOUCH) $@
	@ $(ECHO)

$(OBJ_DIR)/%.o: %.c $(ERROR)
	$(CC) -c -o $(_TMP_O) $(FLAGS) $(EXTRA_CXX_FLAGS) $(LOCAL_DEFINES) $(DEFINES) $(SYS_INCLUDES) $(INCLUDES) $*.c > $*.lst
	@ $(ZIP) -a -m $@.lst.zip $*.lst
	@ $(COPY) $(_TMP_O) $@
	@ $(RM) $(_TMP_O)
	@ $(TOUCH) $@
	@ $(ECHO)
