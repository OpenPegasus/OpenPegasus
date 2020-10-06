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
ROOT = ../..

include $(ROOT)/mak/config.mak
include $(ROOT)/mak/commands.mak


ifndef PEGASUS_HTML_DIR
   PEGASUS_HTML_DIR = $(PEGASUS_HOME)/html
endif

PEGASUS_DEFAULTCXX_COMMON_FILES = \
    ../../src/Pegasus/Common/AcceptLanguageList.h \
    ../../src/Pegasus/Common/Array.h \
    ../../src/Pegasus/Common/ArrayInter.h \
    ../../src/Pegasus/Common/Char16.h \
    ../../src/Pegasus/Common/CIMClass.h \
    ../../src/Pegasus/Common/CIMDateTime.h \
    ../../src/Pegasus/Common/CIMFlavor.h \
    ../../src/Pegasus/Common/CIMIndication.h \
    ../../src/Pegasus/Common/CIMInstance.h \
    ../../src/Pegasus/Common/CIMMethod.h \
    ../../src/Pegasus/Common/CIMName.h \
    ../../src/Pegasus/Common/CIMObject.h \
    ../../src/Pegasus/Common/CIMObjectPath.h \
    ../../src/Pegasus/Common/CIMOperationType.h \
    ../../src/Pegasus/Common/CIMParameter.h \
    ../../src/Pegasus/Common/CIMParamValue.h \
    ../../src/Pegasus/Common/CIMProperty.h \
    ../../src/Pegasus/Common/CIMPropertyList.h \
    ../../src/Pegasus/Common/CIMQualifierDecl.h \
    ../../src/Pegasus/Common/CIMQualifier.h \
    ../../src/Pegasus/Common/CIMScope.h \
    ../../src/Pegasus/Common/CIMStatusCode.h \
    ../../src/Pegasus/Common/CIMType.h \
    ../../src/Pegasus/Common/CIMValue.h \
    ../../src/Pegasus/Common/Config.h \
    ../../src/Pegasus/Common/ContentLanguageList.h \
    ../../src/Pegasus/Common/Exception.h \
    ../../src/Pegasus/Common/LanguageTag.h \
    ../../src/Pegasus/Common/OperationContext.h \
    ../../src/Pegasus/Common/ResponseHandler.h \
    ../../src/Pegasus/Common/SSLContext.h \
    ../../src/Pegasus/Common/String.h

PEGASUS_DEFAULTCXX_CLIENT_FILES = \
    ../../src/Pegasus/Client/CIMClient.h \
    ../../src/Pegasus/Client/CIMClientException.h \
    ../../src/Pegasus/Client/ClientOpPerformanceDataHandler.h

PEGASUS_DEFAULTCXX_CONSUMER_FILES = \
    ../../src/Pegasus/Consumer/CIMIndicationConsumer.h

PEGASUS_DEFAULTCXX_PROVIDER_FILES = \
    ../../src/Pegasus/Provider/CIMAssociationProvider.h \
    ../../src/Pegasus/Provider/CIMIndicationConsumerProvider.h \
    ../../src/Pegasus/Provider/CIMIndicationProvider.h \
    ../../src/Pegasus/Provider/CIMInstanceProvider.h \
    ../../src/Pegasus/Provider/CIMMethodProvider.h \
    ../../src/Pegasus/Provider/CIMOMHandle.h \
    ../../src/Pegasus/Provider/CIMProvider.h \
    ../../src/Pegasus/Provider/ProviderException.h

PEGASUS_CMPI_PROVIDER_FILES = \
    ../../src/Pegasus/Provider/CMPI/cmpidt.h \
    ../../src/Pegasus/Provider/CMPI/cmpift.h \
    ../../src/Pegasus/Provider/CMPI/docs/cmpimacs.h

PEGASUS_CMPICXX_PROVIDER_FILES = \
    ../../src/Pegasus/Provider/CMPI/CmpiArgs.h \
    ../../src/Pegasus/Provider/CMPI/CmpiArray.h \
    ../../src/Pegasus/Provider/CMPI/CmpiAssociationMI.h \
    ../../src/Pegasus/Provider/CMPI/CmpiBaseMI.h \
    ../../src/Pegasus/Provider/CMPI/CmpiBooleanData.h \
    ../../src/Pegasus/Provider/CMPI/CmpiBroker.h \
    ../../src/Pegasus/Provider/CMPI/CmpiCharData.h \
    ../../src/Pegasus/Provider/CMPI/CmpiContext.h \
    ../../src/Pegasus/Provider/CMPI/CmpiData.h \
    ../../src/Pegasus/Provider/CMPI/CmpiDateTime.h \
    ../../src/Pegasus/Provider/CMPI/CmpiEnumeration.h \
    ../../src/Pegasus/Provider/CMPI/CmpiIndicationMI.h \
    ../../src/Pegasus/Provider/CMPI/CmpiInstance.h \
    ../../src/Pegasus/Provider/CMPI/CmpiInstanceMI.h \
    ../../src/Pegasus/Provider/CMPI/CmpiMethodMI.h \
    ../../src/Pegasus/Provider/CMPI/CmpiObject.h \
    ../../src/Pegasus/Provider/CMPI/CmpiObjectPath.h \
    ../../src/Pegasus/Provider/CMPI/CmpiPropertyMI.h \
    ../../src/Pegasus/Provider/CMPI/CmpiProviderBase.h \
    ../../src/Pegasus/Provider/CMPI/CmpiResult.h \
    ../../src/Pegasus/Provider/CMPI/CmpiSelectExp.h \
    ../../src/Pegasus/Provider/CMPI/CmpiStatus.h \
    ../../src/Pegasus/Provider/CMPI/CmpiString.h

FORCE:

buildDoc_ContinueOnError:
	@if $(DOXYGEN) --version > /dev/null 2>&1; \
then $(MAKE) --directory=$(ROOT)/doc/SDK -f Makefile PEGASUS_HTML_DIR=$(PEGASUS_HTML_DIR) buildDoc;\
else $(ECHO) "Warning: API documentation not generated.  $(DOXYGEN) command not found."; fi

buildDoc:
	@$(RMDIRHIER) $(PEGASUS_HTML_DIR)
	@$(MKDIRHIER) $(PEGASUS_HTML_DIR)
	@$(COPY) index.html $(PEGASUS_HTML_DIR)/index.html
	@$(MAKE) --directory=$(ROOT)/doc/SDK -f Makefile PEGASUS_HTML_DIR=$(PEGASUS_HTML_DIR) buildDefaultC++Doc
	@$(MAKE) --directory=$(ROOT)/doc/SDK -f Makefile PEGASUS_HTML_DIR=$(PEGASUS_HTML_DIR) buildCMPIDoc

2.6Deferred:
	@$(MAKE) --directory=$(ROOT)/doc/SDK -f Makefile PEGASUS_HTML_DIR=$(PEGASUS_HTML_DIR) buildCMPIC++Doc

buildDefaultC++Doc:
	@$(RMDIRHIER) $(PEGASUS_HTML_DIR)/DefaultC++
	@$(MKDIRHIER) $(PEGASUS_HTML_DIR)/DefaultC++
	@PEGASUS_PROJECT_NAME="OpenPegasus Default C++ Interface" PEGASUS_HTML_DIR="$(PEGASUS_HTML_DIR)/DefaultC++" PEGASUS_DOXYGEN_FILES="$(PEGASUS_DEFAULTCXX_CLIENT_FILES) $(PEGASUS_DEFAULTCXX_PROVIDER_FILES) $(PEGASUS_DEFAULTCXX_CONSUMER_FILES) $(PEGASUS_DEFAULTCXX_COMMON_FILES)" $(DOXYGEN) Doxyfile

buildCMPIDoc:
	@$(RMDIRHIER) $(PEGASUS_HTML_DIR)/CMPI
	@$(MKDIRHIER) $(PEGASUS_HTML_DIR)/CMPI
	@$(MAKE) --directory=$(PEGASUS_ROOT)/src/Pegasus/Provider/CMPI/docs -f Makefile clean doc
	@PEGASUS_PROJECT_NAME="OpenPegasus CMPI" PEGASUS_HTML_DIR="$(PEGASUS_HTML_DIR)/CMPI" PEGASUS_DOXYGEN_FILES="$(PEGASUS_CMPI_PROVIDER_FILES)" $(DOXYGEN) Doxyfile

buildCMPIC++Doc:
	@$(RMDIRHIER) $(PEGASUS_HTML_DIR)/CMPIC++
	@$(MKDIRHIER) $(PEGASUS_HTML_DIR)/CMPIC++
	@$(MAKE) --directory=$(PEGASUS_ROOT)/src/Pegasus/Provider/CMPI/docs -f Makefile clean doc
	@PEGASUS_PROJECT_NAME="OpenPegasus CMPI C++" PEGASUS_HTML_DIR="$(PEGASUS_HTML_DIR)/CMPIC++" PEGASUS_DOXYGEN_FILES="$(PEGASUS_CMPICXX_PROVIDER_FILES)" $(DOXYGEN) Doxyfile

clean:
	@$(RMDIRHIER) $(PEGASUS_HTML_DIR)
	@$(MAKE) --directory=$(PEGASUS_ROOT)/src/Pegasus/Provider/CMPI/docs -f Makefile clean

depend:

tests:
