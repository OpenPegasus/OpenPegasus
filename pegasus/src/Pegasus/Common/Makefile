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
ROOT = ../../..

DIR = Pegasus/Common

HAS_PAM_DEPENDENCY=true
HAS_ICU_DEPENDENCY=true

include $(ROOT)/mak/config.mak

ifdef PEGASUS_HAS_SSL
    ifdef OPENSSL_HOME
        SYS_INCLUDES += -I$(OPENSSL_HOME)/include
    endif
endif

LOCAL_DEFINES = -DPEGASUS_COMMON_INTERNAL -DPEGASUS_INTERNALONLY

ifeq ($(OS_TYPE),vms)
    STATIC=1
endif

LIBRARY = pegcommon

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
    LIBRARIES = ILEWrapperUtils
endif

##
## CAUTION: the source files are divided evenly (more or less) between SOURCES1
## and SOURCES2. This avoids a command line buffer overflow on Windows 2000
## (see bug #2754 for details). As a precaution, limit SOURCES1 and SOURCES2
## to 75 source files each.
##

SOURCES1 = \
    SCMO.cpp \
    SCMOXmlWriter.cpp \
    SCMOClassCache.cpp \
    SCMOStreamer.cpp \
    Print.cpp \
    Executor.cpp \
    Once.cpp \
    Time.cpp \
    Threads.cpp \
    TSDKey.cpp \
    Semaphore.cpp \
    ReadWriteSem.cpp \
    Condition.cpp \
    AnonymousPipe.cpp \
    Array.cpp \
    AsyncOpNode.cpp \
    AtomicInt.cpp \
    AuthenticationInfoRep.cpp \
    Base64.cpp \
    Buffer.cpp \
    CharSet.cpp \
    LanguageParser.cpp \
    AcceptLanguageList.cpp \
    ContentLanguageList.cpp \
    LanguageTag.cpp \
    MessageLoader.cpp \
    Char16Inline.cpp \
    CIMClass.cpp \
    CIMClassRep.cpp \
    CIMDateTime.cpp \
    CIMFlavor.cpp \
    CIMInstance.cpp \
    CIMInstanceRep.cpp \
    CIMMessage.cpp \
    CIMResponseData.cpp \
    CIMMethod.cpp \
    CIMMethodRep.cpp \
    CIMName.cpp \
    CIMNameInline.cpp \
    CIMObject.cpp \
    CIMObjectRep.cpp \
    CIMObjectPath.cpp \
    Cimom.cpp \
    CimomMessage.cpp \
    CIMParameter.cpp \
    CIMParameterRep.cpp \
    CIMParamValue.cpp \
    CIMParamValueRep.cpp \
    CIMProperty.cpp \
    CIMPropertyList.cpp \
    CIMPropertyRep.cpp \
    CIMQualifier.cpp \
    CIMQualifierDecl.cpp \
    CIMQualifierDeclRep.cpp \
    CIMQualifierList.cpp \
    CIMQualifierNames.cpp \
    CIMQualifierRep.cpp \
    CIMScope.cpp \
    CIMStatusCode.cpp \
    CIMType.cpp \
    CIMValue.cpp \
    CIMValueInline.cpp \
    Config.cpp \
    DeclContext.cpp \
    Dir.cpp \
    List.cpp \
    AsyncQueue.cpp \
    IDFactory.cpp \
    AuditLogger.cpp \
    Magic.cpp \
    UintArgs.cpp

SOURCES2 = \
    Exception.cpp \
    InternalException.cpp \
    FileSystem.cpp \
    Formatter.cpp \
    HashTable.cpp \
    HTTPAcceptor.cpp \
    HTTPConnection.cpp \
    HTTPConnector.cpp  \
    HTTPMessage.cpp \
    Logger.cpp \
    Memory.cpp \
    Message.cpp \
    MessageQueue.cpp \
    MessageQueueService.cpp \
    ModuleController.cpp \
    Monitor.cpp \
    Mutex.cpp \
    ObjectNormalizer.cpp \
    OperationContext.cpp \
    OperationContextInternal.cpp \
    Pair.cpp \
    QueryExpressionRep.cpp \
    Resolver.cpp \
    ResponseHandler.cpp \
    ResponseHandlerRep.cpp \
    Sharable.cpp \
    Signal.cpp \
    Socket.cpp \
    SpinLock.cpp \
    Stack.cpp \
    StatisticalData.cpp \
    String.cpp \
    StringConversion.cpp \
    StringInline.cpp \
    System.cpp \
    TimeValue.cpp \
    SSLContext.cpp \
    TLS.cpp \
    TraceFileHandler.cpp \
    TraceLogHandler.cpp \
    TraceMemoryHandler.cpp \
    Tracer.cpp \
    Thread.cpp \
    ThreadPool.cpp \
    Union.cpp \
    XmlGenerator.cpp \
    XmlParser.cpp \
    XmlReader.cpp \
    XmlWriter.cpp \
    CommonUTF.cpp \
    Constants.cpp \
    HostAddress.cpp \
    HostLocator.cpp \
    BinaryCodec.cpp \
    CIMBuffer.cpp \
    CIMInternalXmlEncoder.cpp \
    SCMOInternalXmlEncoder.cpp \
    AsyncRequestExecutor.cpp \
    CIMBinMsgSerializer.cpp \
    CIMBinMsgDeserializer.cpp \
    IndicationRouter.cpp

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
    SOURCES2 += PaseCcsid.cpp
endif

SOURCES_SLP = Attribute.cpp \
    CIMServerDescription.cpp

ifeq ($(PEGASUS_ENABLE_SLP),true)
    SOURCES2 += $(SOURCES_SLP)
endif

ifeq ($(PEGASUS_PLATFORM),HPUX_PARISC_ACC) 
    SOURCES2 += LoadAndClearWord_HPUX_PARISC_ACC.s
endif

# Special code calling pam_end in class AuthHandle only required
# with session based authentication mechanism
ifeq ($(PEGASUS_PAM_SESSION_SECURITY),true)
    SOURCES2 += AuthHandle.cpp
endif

# LoadAndClearWord_HPUX_PARISC_ACC.s will be compiled using aCC
# in order to use the native assembler to create the object
ifeq ($(PEGASUS_PLATFORM),HPUX_PARISC_GNU)
    SOURCES2 += LoadAndClearWord_HPUX_PARISC_ACC.s
endif

SOURCES = $(SOURCES1) $(SOURCES2)

ifdef PEGASUS_HAS_SSL
    ifeq ($(OS_TYPE),windows)
        SYS_LIBS += /libpath:$(OPENSSL_HOME)/lib libeay32.lib ssleay32.lib
    else
        ifeq ($(OS_TYPE), vms)
            EXTRA_LIBRARIES += -L$(OPENSSL_LIB) \
                -lssl$$libssl_shr32 -lssl$$libcrypto_shr32
        else
            ifdef OPENSSL_HOME
                EXTRA_LIBRARIES += -L$(OPENSSL_HOME)/lib
            endif
            EXTRA_LIBRARIES += -lssl -lcrypto
        endif
    endif
endif

ifeq ($(OS_TYPE),vms)
    ifeq ($(PEGASUS_USE_STATIC_LIBRARIES),false)
        SYS_LIBS += tcpip$$library:tcpip$$lib/lib
        LIBRARIES = vms/include=(vms_crtl_init)
        LINK_OPT1 = COLLECT=crtl_init,lib$$initializdz,lib$$initializd_,
        LINK_OPT2 = lib$$initialize,lib$$initialize$$
        LINKER_OPTIONS = $(LINK_OPT1)$(LINK_OPT2)
    endif
endif

ifeq ($(OS),zos)
    SOURCES2 += Audit_zOS_SMF.cpp \
        PegasusAssertZOS.cpp
endif

ifeq ($(OS),HPUX)
    EXTRA_LIBRARIES += -lipv6
endif

ifeq ($(PEGASUS_PLATFORM),AIX_RS_IBMCXX)
    AIX_LIB_PRIORITY=-20
endif

ifeq ($(PEGASUS_PLATFORM),PASE_ISERIES_IBMCXX)
    AIX_LIB_PRIORITY=-20
endif

# Note that PEGASUS_WINDOWS_SDK_HOME only needs to be specified if using VC6.
ifeq ($(OS_TYPE),windows)
    SYS_LIBS += ws2_32.lib advapi32.lib netapi32.lib
    ifdef PEGASUS_WINDOWS_SDK_HOME
        FLAGS += -DPEGASUS_WINDOWS_SDK_HOME
        SYS_LIBS += /libpath:$(PEGASUS_WINDOWS_SDK_HOME)/lib secur32.lib
    else
        ifneq ($(CL_MAJOR_VERSION), 12)
            SYS_LIBS += secur32.lib
        endif
    endif
endif

ifeq ($(PEGASUS_NEGOTIATE_AUTHENTICATION),true)
    SOURCES += Negotiate.cpp
endif


include $(ROOT)/mak/dynamic-library.mak
# DO NOT DELETE
