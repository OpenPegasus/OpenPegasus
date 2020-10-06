//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_JMPIProviderManager_h
#define Pegasus_JMPIProviderManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/ProviderManager2/ProviderManager.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/ProviderManager2/OperationResponseHandler.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/ProviderManager2/JMPI/Linkage.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPILocalProviderManager.h>
#include <Pegasus/Provider/CIMOMHandleQueryContext.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/WQL/WQLParser.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_JMPIPM_LINKAGE JMPIProviderManager : public ProviderManager
{
public:
    JMPIProviderManager();
    virtual ~JMPIProviderManager(void);

    virtual Boolean insertProvider(const ProviderName & providerName,
            const String &ns, const String &cn);

    virtual Message * processMessage(Message * request) throw();

    static String resolveFileName(String name);

    virtual Boolean hasActiveProviders();
    virtual void unloadIdleProviders();

    class indProvRecord
    {
    public:
        indProvRecord ()
        {
           enabled   = false;
           count     = 0;
           ctx       = NULL;
           handler   = NULL;
        }

        mutable Mutex                     mutex;
        Boolean                           enabled;
        int                               count;
        OperationContext                 *ctx;
        EnableIndicationsResponseHandler *handler;
    };

    class indSelectRecord
    {
    public:
        indSelectRecord ()
        {
            qContext = NULL;
        }

        CIMOMHandleQueryContext *qContext;
        String                   query;
        String                   queryLanguage;
        CIMPropertyList          propertyList;
    };

    typedef HashTable<String,indProvRecord*,EqualFunc<String>,
                      HashFunc<String> > IndProvTab;
    typedef HashTable<String,indSelectRecord*,EqualFunc<String>,
                      HashFunc<String> > IndSelectTab;
    typedef HashTable<String,ProviderName,EqualFunc<String>,
                      HashFunc<String> > ProvRegistrar;

    static Mutex         mutexProvTab;
    static IndProvTab    provTab;
    static Mutex         mutexSelxTab;
    static IndSelectTab  selxTab;
    static Mutex         mutexProvReg;
    static ProvRegistrar provReg;

protected:
    JMPILocalProviderManager providerManager;

    Message * handleUnsupportedRequest(const Message * message) throw();

    Message * handleGetInstanceRequest(const Message * message) throw();
    Message * handleEnumerateInstancesRequest(const Message * message) throw();
    Message * handleEnumerateInstanceNamesRequest(
                  const Message * message) throw();
    Message * handleCreateInstanceRequest(const Message * message) throw();
    Message * handleModifyInstanceRequest(const Message * message) throw();
    Message * handleDeleteInstanceRequest(const Message * message) throw();

    Message * handleExecQueryRequest(const Message * message) throw();

    Message * handleAssociatorsRequest(const Message * message) throw();
    Message * handleAssociatorNamesRequest(const Message * message) throw();
    Message * handleReferencesRequest(const Message * message) throw();
    Message * handleReferenceNamesRequest(const Message * message) throw();

    Message * handleGetPropertyRequest(const Message * message) throw();
    Message * handleSetPropertyRequest(const Message * message) throw();

    Message * handleInvokeMethodRequest(const Message * message) throw();

    Message * handleCreateSubscriptionRequest(const Message * message) throw();
////Message * handleModifySubscriptionRequest(const Message * message) throw();
    Message * handleDeleteSubscriptionRequest(const Message * message) throw();

////Not supported by JMPI
////Message * handleExportIndicationRequest(const Message * message) throw();

    Message * handleDisableModuleRequest(const Message * message) throw();
    Message * handleEnableModuleRequest(const Message * message) throw();
    Message * handleStopAllProvidersRequest(const Message * message) throw();
    Message * handleSubscriptionInitCompleteRequest (const Message * message);
    Message * handleIndicationServiceDisabledRequest (Message * message);

    ProviderName _resolveProviderName(const ProviderIdContainer & providerId);

private:
    void debugPrintMethodPointers (JNIEnv *env, jclass jc);
    bool getInterfaceType         (ProviderIdContainer pidc,
                                   String&             interfaceType,
                                   String&             interfaceVersion);
    bool interfaceIsUsed          (JNIEnv             *env,
                                   jobject             jObject,
                                   String              searchInterfaceName);

    static int trace;
};

PEGASUS_NAMESPACE_END

#endif
