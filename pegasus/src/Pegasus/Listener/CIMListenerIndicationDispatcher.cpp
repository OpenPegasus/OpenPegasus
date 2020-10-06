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

#include "CIMListenerIndicationDispatcher.h"

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/Tracer.h>

#include <Pegasus/Listener/List.h>
#include <Pegasus/Consumer/CIMIndicationConsumer.h>
#include <Pegasus/Common/ContentLanguageList.h>

PEGASUS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
// CIMListenerIndicationDispatchEvent
///////////////////////////////////////////////////////////////////////////////
class CIMListenerIndicationDispatchEvent
{
public:
    CIMListenerIndicationDispatchEvent(CIMIndicationConsumer* consumer,
                                           String url,
                                           CIMInstance instance,
                                           ContentLanguageList contentLangs);
    ~CIMListenerIndicationDispatchEvent();

    CIMIndicationConsumer* getConsumer() const;

    String getURL() const;
    CIMInstance getIndicationInstance() const;
        ContentLanguageList getContentLanguages() const;

private:
    CIMIndicationConsumer*  _consumer;
    String                  _url;
    CIMInstance             _instance;
    ContentLanguageList     _contentLangs;
};

CIMListenerIndicationDispatchEvent::CIMListenerIndicationDispatchEvent(
    CIMIndicationConsumer* consumer,
    String url,
    CIMInstance instance,
    ContentLanguageList contentLangs)
:_consumer(consumer),_url(url),_instance(instance), _contentLangs(contentLangs)
{
}
CIMListenerIndicationDispatchEvent::~CIMListenerIndicationDispatchEvent()
{
}
CIMIndicationConsumer* CIMListenerIndicationDispatchEvent::getConsumer() const
{
    return _consumer;
}
String CIMListenerIndicationDispatchEvent::getURL() const
{
    return _url;
}
CIMInstance CIMListenerIndicationDispatchEvent::getIndicationInstance() const
{
    return _instance;
}
ContentLanguageList
    CIMListenerIndicationDispatchEvent::getContentLanguages() const
{
    return _contentLangs;
}

///////////////////////////////////////////////////////////////////////////////
// CIMListenerIndicationDispatcherRep
///////////////////////////////////////////////////////////////////////////////
class CIMListenerIndicationDispatcherRep
{
public:
    CIMListenerIndicationDispatcherRep();
    virtual ~CIMListenerIndicationDispatcherRep();

    Boolean addConsumer(CIMIndicationConsumer* consumer);
    Boolean removeConsumer(CIMIndicationConsumer* consumer);

    CIMExportIndicationResponseMessage* handleIndicationRequest(
        CIMExportIndicationRequestMessage* request);


    static ThreadReturnType PEGASUS_THREAD_CDECL deliver_routine(void *param);

private:
    void    deliverIndication(String url,
                              CIMInstance instance,
                              ContentLanguageList contentLangs);

    ThreadPool* _thread_pool;
    PtrList*        _consumers;
};

static struct timeval deallocateWait = {15, 0};


CIMListenerIndicationDispatcherRep::CIMListenerIndicationDispatcherRep()
    : _thread_pool(new ThreadPool(0, "ListenerIndicationDispatcher", 0, 0,
    deallocateWait)), _consumers(new PtrList())
{

}
CIMListenerIndicationDispatcherRep::~CIMListenerIndicationDispatcherRep()
{
    delete _thread_pool;
    delete _consumers;
}

Boolean CIMListenerIndicationDispatcherRep::addConsumer(
    CIMIndicationConsumer* consumer)
{
    _consumers->add(consumer);
    return true;
}
Boolean CIMListenerIndicationDispatcherRep::removeConsumer(
    CIMIndicationConsumer* consumer)
{
    _consumers->remove(consumer);
    return true;
}
CIMExportIndicationResponseMessage*
CIMListenerIndicationDispatcherRep::handleIndicationRequest(
    CIMExportIndicationRequestMessage* request)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "CIMListenerIndicationDispatcherRep::handleIndicationRequest");

    CIMInstance instance = request->indicationInstance;
    String          url = request->destinationPath;
    ContentLanguageList contentLangs =
        ((ContentLanguageListContainer)request->operationContext.
            get(ContentLanguageListContainer::NAME)).getLanguages();

    deliverIndication(url,instance,contentLangs);

    // compose a response message
    CIMException cimException;

    CIMExportIndicationResponseMessage* response =
        dynamic_cast<CIMExportIndicationResponseMessage*>(
            request->buildResponse());
    response->cimException = cimException;
    response->dest = request->queueIds.top();

    PEG_METHOD_EXIT();

    return response;
}

void CIMListenerIndicationDispatcherRep::deliverIndication(String url,
    CIMInstance instance,
    ContentLanguageList contentLangs)
{
    // go thru all consumers and broadcast the result;
    // should be run in seperate thread
        AutoPtr<Iterator> it( _consumers->iterator() );

    while(it->hasNext()==true)
    {
        CIMIndicationConsumer* consumer =
            static_cast<CIMIndicationConsumer*>(it->next());
        CIMListenerIndicationDispatchEvent* event =
            new CIMListenerIndicationDispatchEvent(consumer,
                                                   url,
                                                   instance,
                                                   contentLangs);
        ThreadStatus rtn = _thread_pool->allocate_and_awaken(
                                event,deliver_routine);

            if (rtn != PEGASUS_THREAD_OK)
            {
                PEG_TRACE_CSTRING(
                    TRC_SERVER,
                    Tracer::LEVEL1,
                    "Could not allocate thread to deliver event."
                        " Instead using current thread.");
            delete event;
            throw Exception(MessageLoaderParms(
                "Listener.CIMListenerIndicationDispatcher."
                    "CANNOT_ALLOCATE_THREAD",
                "Not enough threads to allocate a worker to deliver the"
                    " event."));
            }
    }
}
ThreadReturnType PEGASUS_THREAD_CDECL
CIMListenerIndicationDispatcherRep::deliver_routine(void *param)
{
    CIMListenerIndicationDispatchEvent* event =
        static_cast<CIMListenerIndicationDispatchEvent*>(param);

    if(event!=NULL)
    {
        CIMIndicationConsumer* consumer = event->getConsumer();
        OperationContext context;
            context.insert(ContentLanguageListContainer(
                        event->getContentLanguages()));
        if(consumer)
        {
            consumer->consumeIndication(context,
                event->getURL(),event->getIndicationInstance());
        }

        delete event;
    }

    return (0);
}

///////////////////////////////////////////////////////////////////////////////
// CIMListenerIndicationDispatcher
///////////////////////////////////////////////////////////////////////////////
CIMListenerIndicationDispatcher::CIMListenerIndicationDispatcher()
:Base(PEGASUS_QUEUENAME_LISTENERINDICATIONDISPACTCHER)
,_rep(new CIMListenerIndicationDispatcherRep())
{
}
CIMListenerIndicationDispatcher::~CIMListenerIndicationDispatcher()
{
    if(_rep!=NULL)
        delete static_cast<CIMListenerIndicationDispatcherRep*>(_rep);

    _rep=NULL;
}

void CIMListenerIndicationDispatcher::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "CIMListenerIndicationDispatcher::handleEnqueue");

    Message *message = dequeue();
    if(message)
        handleEnqueue(message);

    PEG_METHOD_EXIT();
}

void CIMListenerIndicationDispatcher::handleEnqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_SERVER,
        "CIMListenerIndicationDispatcher::handleEnqueue");

    if(message!=NULL)
    {
        switch (message->getType())
        {
            case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
                {
                    CIMExportIndicationRequestMessage* request =
                        (CIMExportIndicationRequestMessage*)message;

                    CIMExportIndicationResponseMessage* response =
                        static_cast<CIMListenerIndicationDispatcherRep*>(_rep)->
                            handleIndicationRequest(request);

                    MessageQueue* queue = MessageQueue::lookup(response->dest);
                    PEGASUS_ASSERT(queue != 0);
                    queue->enqueue(response);
                }
                break;
            default:
                break;
        }
    delete message;
    }

    PEG_METHOD_EXIT();
}
Boolean CIMListenerIndicationDispatcher::addConsumer(
    CIMIndicationConsumer* consumer)
{
    return static_cast<CIMListenerIndicationDispatcherRep*>(_rep)->addConsumer(
            consumer);
}
Boolean CIMListenerIndicationDispatcher::removeConsumer(
        CIMIndicationConsumer* consumer)
{
    return static_cast<CIMListenerIndicationDispatcherRep*>
        (_rep)->removeConsumer(consumer);
}

PEGASUS_NAMESPACE_END
