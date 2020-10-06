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

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/ProviderManager2/AutoPThreadSecurity.h>

#include "ClientCIMOMHandleRep.h"

PEGASUS_NAMESPACE_BEGIN

static void deleteContentLanguage(void* data)
{
    ContentLanguageList* cl = static_cast<ContentLanguageList*>(data);
    delete cl;
}

/**
    This class is used to prevent concurrent access to a non-reentrant
    CIMClient object.
 */
class ClientCIMOMHandleAccessController
{
public:
    ClientCIMOMHandleAccessController(Mutex& lock)
        : _lock(lock)
    {
        try
        {
            // assume default client timeout
            if (!_lock.timed_lock(PEGASUS_DEFAULT_CLIENT_TIMEOUT_MILLISECONDS))
            {
                throw CIMException(CIM_ERR_ACCESS_DENIED, MessageLoaderParms(
                    "Provider.CIMOMHandle.CIMOMHANDLE_TIMEOUT",
                    "Timeout waiting for CIMOMHandle"));
            }
        }
        catch (Exception& e)
        {
            PEG_TRACE((TRC_CIMOM_HANDLE, Tracer::LEVEL2,
                "Unexpected Exception: %s",
                (const char*)e.getMessage().getCString()));
            throw;
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_CIMOM_HANDLE, Tracer::LEVEL2,
                "Unexpected exception");
            throw;
        }
    }

    ~ClientCIMOMHandleAccessController()
    {
        // Must not throw an exception from a destructor
        try
        {
            _lock.unlock();
        }
        catch (Exception& e)
        {
            PEG_TRACE((TRC_CIMOM_HANDLE, Tracer::LEVEL2,
                "Ignoring Exception: %s",
                (const char*)e.getMessage().getCString()));
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_CIMOM_HANDLE, Tracer::LEVEL2,
                "Ignoring unknown exception");
        }
    }

private:
    // Unimplemented constructors and assignment operator
    ClientCIMOMHandleAccessController();
    ClientCIMOMHandleAccessController(const ClientCIMOMHandleAccessController&);
    ClientCIMOMHandleAccessController& operator=(
        const ClientCIMOMHandleAccessController&);

    Mutex& _lock;
};


/**
    The ClientCIMOMHandleSetup class encapsulates the logic to set up the
    CIMClient object state based on a specified OperationContext.  The
    original CIMClient state is restored by the destructor.  Only one
    ClientCIMOMHandleSetup object may operate on a given CIMClient object
    at a time.  Use of the ClientCIMOMHandleAccessController class is
    recommend to control access to CIMClient objects.
 */
class ClientCIMOMHandleSetup
{
public:
    ClientCIMOMHandleSetup(
        CIMClientRep*& client,
        const OperationContext& context)
    {
        //
        // Initialize the CIMClient object if necessary
        //
        if (client == 0)
        {
            PEG_TRACE_CSTRING(TRC_CIMOM_HANDLE, Tracer::LEVEL3,
                "Creating CIMClient connection");
            client = new CIMClientRep();

            //
            // If connection fails, we need to make sure that subsequent
            // calls will try to connect again.
            //
            try
            {
                client->connectLocalBinary();
            }
            catch(...)
            {
                delete client;
                client = 0;
                throw;
            }
        }
        _client = client;

        //
        // If the caller specified a timeout value in the OperationContext,
        // set it in the CIMClient object.
        //
        _origTimeout = client->getTimeout();
        if (context.contains(TimeoutContainer::NAME))
        {
            TimeoutContainer t_cntr = (TimeoutContainer)
                context.get(TimeoutContainer::NAME);
            client->setTimeout(t_cntr.getTimeOut());
        }

        //
        // If the caller specified an Accept-Language in the
        // OperationContext, set it in the CIMClient object.
        //
        _origAcceptLanguages = client->getRequestAcceptLanguages();
        if (context.contains(AcceptLanguageListContainer::NAME))
        {
            AcceptLanguageListContainer al_cntr = (AcceptLanguageListContainer)
                context.get(AcceptLanguageListContainer::NAME);
            _client->setRequestAcceptLanguages(al_cntr.getLanguages());
        }
        else
        {
            // No AcceptLanguageListContainer in OperationContext; try
            // getting the AcceptLanguageList from the current thread
            AcceptLanguageList* al = Thread::getLanguages();
            if (al != NULL)
            {
                _client->setRequestAcceptLanguages(*al);
            }
        }

        //
        // If the caller specified a Content-Language in the
        // OperationContext, set it in the CIMClient object.
        //
        _origContentLanguages = client->getRequestContentLanguages();
        if (context.contains(ContentLanguageListContainer::NAME))
        {
            ContentLanguageListContainer cl_cntr =
                (ContentLanguageListContainer)context.get(
                    ContentLanguageListContainer::NAME);
            _client->setRequestContentLanguages(cl_cntr.getLanguages());
        }
    }

    ~ClientCIMOMHandleSetup()
    {
        // Must not throw an exception from a destructor
        try
        {
            //
            // If the response has a Content-Language then save it into
            // thread-specific storage
            //
            if (_client->getResponseContentLanguages().size() > 0)
            {
                 Thread* curThrd = Thread::getCurrent();
                 if (curThrd != NULL)
                 {
                     // deletes the old tsd and creates a new one
                     curThrd->put_tsd(TSD_CIMOM_HANDLE_CONTENT_LANGUAGES,
                         deleteContentLanguage,
                         sizeof(ContentLanguageList*),
                         new ContentLanguageList(
                             _client->getResponseContentLanguages()));
                 }
            }

            //
            // Reset CIMClient timeout value and languages to original values
            //
            _client->setTimeout(_origTimeout);
            _client->setRequestAcceptLanguages(_origAcceptLanguages);
            _client->setRequestContentLanguages(_origContentLanguages);
        }
        catch (Exception& e)
        {
            PEG_TRACE((TRC_CIMOM_HANDLE, Tracer::LEVEL1,
                "Ignoring Exception: %s",
                (const char*)e.getMessage().getCString()));
        }
        catch (...)
        {
            PEG_TRACE_CSTRING(TRC_CIMOM_HANDLE, Tracer::LEVEL1,
                "Ignoring unknown exception");
        }
    }

private:
    // Unimplemented constructors and assignment operator
    ClientCIMOMHandleSetup();
    ClientCIMOMHandleSetup(const ClientCIMOMHandleSetup&);
    ClientCIMOMHandleSetup& operator=(const ClientCIMOMHandleSetup&);

    CIMClientRep* _client;
    Uint32 _origTimeout;
    AcceptLanguageList _origAcceptLanguages;
    ContentLanguageList _origContentLanguages;
};


//
// ClientCIMOMHandleRep
//

ClientCIMOMHandleRep::ClientCIMOMHandleRep()
    : _client(0)
{
}

ClientCIMOMHandleRep::~ClientCIMOMHandleRep()
{
    if (_client != 0)
    {
        // Must not throw an exception from a destructor
        try
        {
            _client->disconnect();
        }
        catch (...)
        {
            // Ignore disconnect exceptions
        }

        delete _client;
    }
}


//
// CIM Operations
//

CIMClass ClientCIMOMHandleRep::getClass(
    const OperationContext & context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::getClass");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->getClass(
        nameSpace,
        className,
        localOnly,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

Array<CIMClass> ClientCIMOMHandleRep::enumerateClasses(
    const OperationContext & context,
    const CIMNamespaceName& nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean localOnly,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::enumerateClasses");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->enumerateClasses(
        nameSpace,
        className,
        deepInheritance,
        localOnly,
        includeQualifiers,
        includeClassOrigin);
}

Array<CIMName> ClientCIMOMHandleRep::enumerateClassNames(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMName& className,
    Boolean deepInheritance)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::enumerateClassNames");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->enumerateClassNames(
        nameSpace,
        className,
        deepInheritance);
}

void ClientCIMOMHandleRep::createClass(
    const OperationContext & context,
    const CIMNamespaceName& nameSpace,
    const CIMClass& newClass)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::createClass");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    _client->createClass(
        nameSpace,
        newClass);

    PEG_METHOD_EXIT();
}

void ClientCIMOMHandleRep::modifyClass(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMClass& modifiedClass)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::modifyClass");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    _client->modifyClass(
        nameSpace,
        modifiedClass);

    PEG_METHOD_EXIT();
}

void ClientCIMOMHandleRep::deleteClass(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMName& className)
{

    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::deleteClass");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    _client->deleteClass(
        nameSpace,
        className);

    PEG_METHOD_EXIT();
}

CIMResponseData ClientCIMOMHandleRep::getInstance(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& instanceName,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::getInstance");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->getInstance(
        nameSpace,
        instanceName,
        false,    // localOnly
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

CIMResponseData ClientCIMOMHandleRep::enumerateInstances(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMName& className,
    Boolean deepInheritance,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::enumerateInstances");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->enumerateInstances(
        nameSpace,
        className,
        deepInheritance,
        false,    // localOnly
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

CIMResponseData ClientCIMOMHandleRep::enumerateInstanceNames(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMName& className)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::enumerateInstanceNames");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->enumerateInstanceNames(
        nameSpace,
        className);
}

CIMObjectPath ClientCIMOMHandleRep::createInstance(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMInstance& newInstance)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::createInstance");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->createInstance(
        nameSpace,
        newInstance);
}

void ClientCIMOMHandleRep::modifyInstance(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMInstance& modifiedInstance,
    Boolean includeQualifiers,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::modifyInstance");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    _client->modifyInstance(
        nameSpace,
        modifiedInstance,
        includeQualifiers,
        propertyList);

    PEG_METHOD_EXIT();
}

void ClientCIMOMHandleRep::deleteInstance(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& instanceName)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::deleteInstance");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    _client->deleteInstance(
        nameSpace,
        instanceName);

    PEG_METHOD_EXIT();
}

CIMResponseData ClientCIMOMHandleRep::execQuery(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const String& queryLanguage,
    const String& query)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::execQuery");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->execQuery(
        nameSpace,
        queryLanguage,
        query);
}

CIMResponseData ClientCIMOMHandleRep::associators(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::associators");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->associators(
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

CIMResponseData ClientCIMOMHandleRep::associatorNames(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& assocClass,
    const CIMName& resultClass,
    const String& role,
    const String& resultRole)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::associatorNames");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->associatorNames(
        nameSpace,
        objectName,
        assocClass,
        resultClass,
        role,
        resultRole);
}

CIMResponseData ClientCIMOMHandleRep::references(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMPropertyList& propertyList)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::references");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->references(
        nameSpace,
        objectName,
        resultClass,
        role,
        includeQualifiers,
        includeClassOrigin,
        propertyList);
}

CIMResponseData ClientCIMOMHandleRep::referenceNames(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& objectName,
    const CIMName& resultClass,
    const String& role)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE,
        "ClientCIMOMHandleRep::referenceNames");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->referenceNames(
        nameSpace,
        objectName,
        resultClass,
        role);
}

CIMValue ClientCIMOMHandleRep::getProperty(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::getProperty");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->getProperty(
        nameSpace,
        instanceName,
        propertyName);
}

void ClientCIMOMHandleRep::setProperty(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& propertyName,
    const CIMValue& newValue)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::setProperty");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    _client->setProperty(
        nameSpace,
        instanceName,
        propertyName,
        newValue);

    PEG_METHOD_EXIT();
}

CIMValue ClientCIMOMHandleRep::invokeMethod(
    const OperationContext & context,
    const CIMNamespaceName &nameSpace,
    const CIMObjectPath& instanceName,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParameters,
    Array<CIMParamValue>& outParameters)
{
    PEG_METHOD_ENTER(TRC_CIMOM_HANDLE, "ClientCIMOMHandleRep::invokeMethod");

    AutoPThreadSecurity revPthreadSec(context, true);
    ClientCIMOMHandleAccessController access(_clientMutex);
    ClientCIMOMHandleSetup setup(_client, context);

    PEG_METHOD_EXIT();
    return _client->invokeMethod(
        nameSpace,
        instanceName,
        methodName,
        inParameters,
        outParameters);
}


//
// Other public methods
//

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
OperationContext ClientCIMOMHandleRep::getResponseContext()
{
    OperationContext ctx;

    Thread* curThrd = Thread::getCurrent();
    if (curThrd == NULL)
    {
        ctx.insert(ContentLanguageListContainer(ContentLanguageList()));
    }
    else
    {
        ContentLanguageList* contentLangs = (ContentLanguageList*)
            curThrd->reference_tsd(TSD_CIMOM_HANDLE_CONTENT_LANGUAGES);
        curThrd->dereference_tsd();

        if (contentLangs == NULL)
        {
            ctx.insert(ContentLanguageListContainer(ContentLanguageList()));
        }
        else
        {
            ctx.insert(ContentLanguageListContainer(*contentLangs));
            // delete the old tsd to free the memory
            curThrd->delete_tsd(TSD_CIMOM_HANDLE_CONTENT_LANGUAGES);
        }
    }

    return ctx;
}
#endif

PEGASUS_NAMESPACE_END
