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

#DEPEND_MAK_TMP = $(OBJ_DIR)/depend.mak
#DEPEND_MAK = $(subst /,\\,$(DEPEND_MAK_TMP))

DEPEND_MAK = $(OBJ_DIR)/depend.mak

##
## Notice that the system includes are excluded. That is because this
## depend utility doesn't require that all the includes be resolved (just
## the ones that matter (the ones the user put on the path).
##

ifdef SOURCES1
depend: $(OBJ_DIR)/target $(ERROR)
	mu depend -O$(OBJ_DIR) $(INCLUDES) $(SOURCES1) > $(DEPEND_MAK)
	mu depend -O$(OBJ_DIR) $(INCLUDES) $(SOURCES2) >> $(DEPEND_MAK)
else
depend: $(OBJ_DIR)/target $(ERROR)
	mu depend -O$(OBJ_DIR) $(INCLUDES) $(SOURCES) > $(DEPEND_MAK)
endif

clean-depend:
	$(RM) $(OBJ_DIR)/depend.mak
	$(TOUCH) $(OBJ_DIR)/depend.mak
