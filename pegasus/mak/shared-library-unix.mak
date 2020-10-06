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


##==============================================================================
##
## xlc compiler
##
##==============================================================================

ifeq ($(COMPILER),xlc)
  LINK_COMMAND = xlC_r
  ifdef PEGASUS_USE_RELEASE_DIRS
    ifdef PEGASUS_HAS_MESSAGES
      ifdef ICU_INSTALL
        LINK_ARGUMENTS = -qmkshrobj=$(AIX_LIB_PRIORITY) -blibpath:/usr/lib:/lib:$(ICU_INSTALL)/lib -Wl,-bhalt:$(AIX_LD_HALT)
      endif
    else
      LINK_ARGUMENTS = -qmkshrobj=$(AIX_LIB_PRIORITY) -blibpath:/usr/lib:/lib -Wl,-bhalt:$(AIX_LD_HALT)
    endif
  else
    LINK_ARGUMENTS = -qmkshrobj=$(AIX_LIB_PRIORITY) -Wl,-bhalt:$(AIX_LD_HALT)
  endif
  LINK_OUT = -o
  ifeq ($(PEGASUS_SUPPORTS_DYNLIB), yes)
    LINK_COMMAND += -G
  endif
endif

##==============================================================================
##
## acc compiler
##
##==============================================================================

ifeq ($(COMPILER),acc)
  LINK_COMMAND = $(CXX) -b -Wl,+hlib$(LIBRARY)$(LIB_SUFFIX)
  ifeq ($(PEGASUS_PLATFORM), HPUX_IA64_ACC)
    LINK_COMMAND += +DD64 -mt -Wl,+rpathfirst
  else
    LINK_COMMAND += -Wl,-Bsymbolic
  endif
  ifeq ($(PEGASUS_SUPPORTS_DYNLIB),yes)
    LINK_COMMAND += $(ACC_LINK_SEARCH_PATH)
  endif
  ifdef PEGASUS_USE_DEBUG_BUILD_OPTIONS
    LINK_COMMAND += -g +noobjdebug
  endif
  ifdef PEGASUS_CCOVER
    EXTRA_LIBRARIES += $(CCOVER_LIB)/libcov.a
  endif
  LINK_ARGUMENTS =
  LINK_OUT = -o
endif

##==============================================================================
##
## gnu compiler
##
##==============================================================================

ifeq ($(COMPILER),gnu)
  ifeq ($(HAS_ICU_DEPENDENCY),true)
    ifdef ICU_INSTALL
      ifeq ($(PEGASUS_PLATFORM), HPUX_PARISC_GNU)
        EXTRA_LINK_ARGUMENTS += -Xlinker $(ICU_INSTALL)/lib
      else 
        EXTRA_LINK_ARGUMENTS += -Xlinker -rpath -Xlinker $(ICU_INSTALL)/lib
      endif
    endif
  endif
  ifneq ($(OS),darwin)
   ifdef PEGASUS_USE_RELEASE_DIRS
      LINK_COMMAND = $(CXX) -shared $(LINK_MACHINE_OPTIONS)
      LINK_ARGUMENTS = -Wl,-hlib$(LIBRARY)$(LIB_SUFFIX)  -Xlinker -rpath \
          -Xlinker $(PEGASUS_DEST_LIB_DIR) $(EXTRA_LINK_ARGUMENTS)
   else
      LINK_COMMAND = $(CXX) -shared $(LINK_MACHINE_OPTIONS)
      LINK_ARGUMENTS = -Wl,-hlib$(LIBRARY)$(LIB_SUFFIX)  -Xlinker -rpath -Xlinker $(LIB_DIR) $(EXTRA_LINK_ARGUMENTS)
   endif
  else
    LINK_COMMAND = $(CXX) -dynamiclib
    LINK_ARGUMENTS = --helplib$(LIBRARY)$(LIB_SUFFIX) -ldl
  endif
  ifeq ($(PEGASUS_PLATFORM), SOLARIS_SPARC_GNU)
    LINK_ARGUMENTS = -Wl,-hlib$(LIBRARY)$(LIB_SUFFIX) -Xlinker -L$(LIB_DIR) $(EXTRA_LINK_ARGUMENTS)
  endif
  ifeq ($(OS), HPUX)
    LINK_COMMAND += -pthread -Wl,+rpathfirst
    LINK_ARGUMENTS = -Xlinker -L$(LIB_DIR) $(EXTRA_LINK_ARGUMENTS)
  endif
  ifeq ($(PEGASUS_SUPPORTS_DYNLIB),yes)
      LINK_COMMAND += $(GNU_LINK_SEARCH_PATH)
  endif
  LINK_OUT = -o
endif
##==============================================================================
##
## llvm/clang compiler 
## change for other platform like windows
##
##==============================================================================

ifeq ($(COMPILER),clang)
  ifeq ($(HAS_ICU_DEPENDENCY),true)
    ifdef ICU_INSTALL
      EXTRA_LINK_ARGUMENTS += -Xlinker -rpath -Xlinker $(ICU_INSTALL)/lib
    endif
  endif
  ifdef PEGASUS_USE_RELEASE_DIRS
      LINK_COMMAND = $(CXX) -shared $(LINK_MACHINE_OPTIONS)
      LINK_ARGUMENTS = -Wl,-hlib$(LIBRARY)$(LIB_SUFFIX)  -Xlinker -rpath \
          -Xlinker $(PEGASUS_DEST_LIB_DIR) $(EXTRA_LINK_ARGUMENTS)
  else
      LINK_COMMAND = $(CXX) -shared $(LINK_MACHINE_OPTIONS)
      LINK_ARGUMENTS = -Wl,-hlib$(LIBRARY)$(LIB_SUFFIX)  -Xlinker -rpath -Xlinker $(LIB_DIR) $(EXTRA_LINK_ARGUMENTS)
  endif
  LINK_OUT = -o
endif

##==============================================================================
##
## decxx compiler
##
##==============================================================================

ifeq ($(COMPILER),deccxx)
  LINK_COMMAND = cxx -shared
  LINK_ARGUMENTS =
  LINK_OUT = -o
endif

##==============================================================================
##
## ibm compiler
##
##==============================================================================

ifeq ($(COMPILER),ibm)
  LINK_COMMAND = $(CXX) $(FLAGS)
  LINK_ARGUMENTS = -W "l,XPLINK,dll,EDIT=NO"
  LINK_OUT = -o
  ifdef PEGASUS_GENERATE_LISTINGS
    LINK_ARGUMENTS += -W"l,MAP,LIST"
  endif
endif

##==============================================================================
##
## CC compiler
##
##==============================================================================

ifeq ($(COMPILER),CC)
  LINK_COMMAND = CC
  LINK_ARGUMENTS = -G -KPIC -mt -h lib$(LIBRARY).so $(LINK_MACHINE_OPTIONS)
  LINK_OUT = -o
  EXTRA_LIBRARIES += $(LIBRARY_SYS_LIBS)
endif

##==============================================================================
##
## Link target
##
##==============================================================================

ifdef EXTRA_LINK_FLAGS
  LINK_COMMAND += $(EXTRA_LINK_FLAGS)
endif

FULL_LIB=$(LIB_DIR)/lib$(LIBRARY)$(LIB_SUFFIX)

## Rule for all UNIX library builds
$(FULL_LIB): $(LIB_DIR)/target $(OBJ_DIR)/target $(OBJECTS) $(FULL_LIBRARIES) \
    $(ERROR)
  ifeq ($(PEGASUS_SUPPORTS_DYNLIB),yes)
    ## To generate shared libraries which will cause dynamic
    ## search of other shared libraries which they reference,
    ## must specify the referenced shared libraries as "-l<name>"
    ## DYNAMIC_LIBRARIES must be defined appropriately in the
    ## libraries.mak file that includes this file
    ##


    ifeq ($(PEGASUS_PLATFORM),AIX_RS_IBMCXX)
	rm -f $(FULL_LIB)
    endif

    ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
	rm -f $(FULL_LIB)
    endif

    ifeq ($(OS),zos)
	$(LINK_COMMAND) $(LINK_ARGUMENTS) -L$(LIB_DIR) $(LINK_OUT) $(FULL_LIB) $(OBJECTS) $(EXTRA_LIBRARIES) $(SYS_LIBS) $(DYNAMIC_LIBRARIES) > lib$(LIBRARY).llst
	@ $(ZIP) -a -m $(FULL_LIB).llst.zip lib$(LIBRARY).llst
      ## z/OS needs side definition files to link executables to
      ## dynamic libraries, so we have to copy them into the lib_dir
	touch $(ROOT)/src/$(DIR)/lib$(LIBRARY)$(DYNLIB_SUFFIX)
	mv $(ROOT)/src/$(DIR)/lib$(LIBRARY)$(DYNLIB_SUFFIX) $(LIB_DIR)
    else

	$(LINK_COMMAND) $(LINK_ARGUMENTS) -L$(LIB_DIR) $(LINK_OUT) $(FULL_LIB) $(OBJECTS) $(DYNAMIC_LIBRARIES) $(EXTRA_LIBRARIES) $(SYS_LIBS)

    endif
  else
	$(LINK_COMMAND) $(LINK_ARGUMENTS) $(LINK_OUT) $(FULL_LIB) $(OBJECTS) $(FULL_LIBRARIES) $(EXTRA_LIBRARIES) $(SYS_LIBS)
  endif
    ifeq ($(PEGASUS_PLATFORM),HPUX_PARISC_ACC)
	$(MAKE) --directory=$(LIB_DIR) -f $(PEGASUS_ROOT)/mak/library-unix.mak \
            ln LIBRARY=lib$(LIBRARY) SUFFIX=$(LIB_SUFFIX) PLATFORM_SUFFIX=sl
    endif
    ifeq ($(PEGASUS_PLATFORM),HPUX_PARISC_GNU)
	$(MAKE) --directory=$(LIB_DIR) -f $(PEGASUS_ROOT)/mak/library-unix.mak \
            ln LIBRARY=lib$(LIBRARY) SUFFIX=$(LIB_SUFFIX) PLATFORM_SUFFIX=sl
    endif
    ifeq ($(PEGASUS_PLATFORM),HPUX_IA64_ACC)
	$(MAKE) --directory=$(LIB_DIR) -f $(PEGASUS_ROOT)/mak/library-unix.mak \
            ln LIBRARY=lib$(LIBRARY) SUFFIX=$(LIB_SUFFIX) PLATFORM_SUFFIX=so
    endif
    ifeq ($(PEGASUS_PLATFORM),HPUX_IA64_GNU)
	$(MAKE) --directory=$(LIB_DIR) -f $(PEGASUS_ROOT)/mak/library-unix.mak \
       ln LIBRARY=lib$(LIBRARY) SUFFIX=$(LIB_SUFFIX) PLATFORM_SUFFIX=so
    endif
    ifdef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
	$(MAKE) --directory=$(LIB_DIR) -f $(PEGASUS_ROOT)/mak/library-unix.mak \
            ln LIBRARY=lib$(LIBRARY) SUFFIX=$(LIB_SUFFIX) PLATFORM_SUFFIX=so
    endif
	$(TOUCH) $(FULL_LIB)
	@ $(ECHO)

##==============================================================================
##
## clean-lib
##
##==============================================================================

clean-lib: $(ERROR)
	rm -f $(FULL_LIB)

##==============================================================================
##
## ln (target to create soft link)
##
## This target is used to allow the current working directory to be set in a
## separate "make" execution, to avoid using fully-specified paths in the link
## command.
##
##==============================================================================

ln:
	ln -f -s $(LIBRARY)$(SUFFIX) $(LIBRARY).$(PLATFORM_SUFFIX)

##==============================================================================
##
## FILES_TO_CLEAN (files removed by "make clean")
##
##==============================================================================

FILES_TO_CLEAN = $(OBJECTS) $(FULL_LIB)
ifeq ($(PEGASUS_PLATFORM),HPUX_PARISC_ACC)
    FILES_TO_CLEAN += $(LIB_DIR)/lib$(LIBRARY).sl
endif
ifeq ($(PEGASUS_PLATFORM),HPUX_PARISC_GNU)
    FILES_TO_CLEAN += $(LIB_DIR)/lib$(LIBRARY).sl
endif
ifeq ($(PEGASUS_PLATFORM),HPUX_IA64_ACC)
    FILES_TO_CLEAN += $(LIB_DIR)/lib$(LIBRARY).so
endif
ifeq ($(PEGASUS_PLATFORM),HPUX_IA64_GNU)
    FILES_TO_CLEAN += $(LIB_DIR)/lib$(LIBRARY).so
endif
ifdef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
    FILES_TO_CLEAN += $(LIB_DIR)/lib$(LIBRARY).so
endif
