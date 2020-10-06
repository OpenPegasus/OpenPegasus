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
## Common definitions used by both program.mak and libraries.mak
##
################################################################################

ifeq ($(OS_TYPE),vms)
    # Allow option lib/option syntax
    _P1 = $(foreach lib,$(LIBRARIES),$(word 1,$(subst /, ,$(lib))))
    FULL_LIBRARIES = $(addprefix $(LIB_DIR)/$(LIB_PREFIX), \
        $(addsuffix $(LIB_SUFFIX), $(_P1)))
else
    ifeq ($(PEGASUS_USE_STATIC_LIBRARIES),true)
        _P1 = $(addprefix $(LIB_DIR)/$(LIB_PREFIX), $(LIBRARIES))
        _P2 = $(addsuffix ".*[1a]", $(_P1))
        FULL_LIBRARIES=$(shell echo $(_P2))
    else
        FULL_LIBRARIES = $(addprefix $(LIB_DIR)/$(LIB_PREFIX), \
            $(addsuffix $(LIB_SUFFIX), $(LIBRARIES)))
    endif
endif

ifeq ($(OS),zos)
    ifeq ($(PEGASUS_USE_STATIC_LIBRARIES),true)
        _P1 = $(addprefix $(LIB_DIR)/$(LIB_PREFIX), $(LIBRARIES))
        _P2 = $(addsuffix $(STATLIB_SUFFIX), $(_P1))
        _P3 = $(addsuffix $(DYNLIB_SUFFIX), $(_P1))
        _P4 = $(shell echo $(_P2) $(_P3))
        DYNAMIC_LIBRARIES = $(foreach linkfile, $(_P4), $(wildcard $(linkfile)))
        FULL_LIBRARIES = $(foreach linkfile, $(_P4), $(wildcard $(linkfile)))
    else	
        DYNAMIC_LIBRARIES = $(addprefix $(LIB_DIR)/$(LIB_PREFIX), \
            $(addsuffix $(DYNLIB_SUFFIX), $(LIBRARIES)))
    endif
else
    DYNAMIC_LIBRARIES = $(addprefix -l, $(LIBRARIES))
endif
