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

#ifndef Pegasus_ProviderAgent_h
#define Pegasus_ProviderAgent_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Semaphore.h>
#include <Pegasus/Common/AnonymousPipe.h>
#include <Pegasus/Common/Thread.h>
#include <Pegasus/Common/ThreadPool.h>
#include <Pegasus/Common/Signal.h>
#include <Pegasus/Common/SCMOClassCache.h>
#include <Pegasus/ProviderManagerRouter/BasicProviderManagerRouter.h>

PEGASUS_NAMESPACE_BEGIN

class ProviderAgent
{
public:
    /** Constructor */
    ProviderAgent(
        const String& agentId,
        AnonymousPipe* pipeFromServer,
        AnonymousPipe* pipeToServer);

    /** Destructor */
    ~ProviderAgent();

    /**
        Start the Provider Agent reading and processing requests from the
        CIM Server.  This method returns when the Provider Agent is exiting.
        The Provider Agent exits when: 1) Its provider module is disabled,
        stopped, or idle, 2) It is unable to communicate with the CIM Server,
        3) It receives a TERM or HUP signal, or 4) It encounters an
        unrecoverable error.
     */
    void run();

private:
    //
    // Private methods
    //

    /** Unimplemented */
    ProviderAgent();
    /** Unimplemented */
    ProviderAgent(const ProviderAgent&);
    /** Unimplemented */
    ProviderAgent& operator=(const ProviderAgent&);

    /**
        Read a request from the _pipeFromServer and process it.

        @return true if the Provider Agent successfully received a request
                from the CIM Server; false otherwise.
     */
    Boolean _readAndProcessRequest();

    /**
        Forward a request to the _providerManagerRouter.  Catch exceptions
        and add them to the cimException attribute in the response.

        @param request The CIMRequestMessage to process.
        @return A CIMResponseMessage resulting from the processing of the
                specified CIMRequestMessage.
     */
    Message* _processRequest(CIMRequestMessage* request);

    /**
        Write a message to the _pipeToServer.  On error, set _terminating
        to true.

        @param message The message to write to the CIM Server.
     */
    void _writeResponse(Message* message);

    /**
        Forward a request to the _providerManagerRouter.  Catch exceptions
        and add them to the cimException attribute in the response.
        Write the response message to the _pipeToServer.  On error, set
        _terminating to true.

        This method allows a request message to be processed asynchronously
        in its own thread.

        @param arg A pointer to a ProviderAgentRequest object containing a
                   reference to the ProviderAgent object and the request
                   message to process.
     */
    static ThreadReturnType PEGASUS_THREAD_CDECL
        _processRequestAndWriteResponse(void* arg);

    /**
        Callback function to which all generated indications are sent for
        processing.
     */
    static void _indicationCallback(
        CIMProcessIndicationRequestMessage* request);

    static void _handleIndicationDeliveryResponse(
        CIMProcessIndicationResponseMessage *response);

    /**
        Callback function to which all response chunks are sent for processing.
     */
    static void _responseChunkCallback(
        CIMRequestMessage* request, CIMResponseMessage* response);

    /*
     *Tries to unload idle providers giving a timeout of value shutdown timeout
     * return true if unloaded successfuly
     */
    Boolean _unloadIdleProviders();

    static ThreadReturnType PEGASUS_THREAD_CDECL
        _unloadIdleProvidersHandler(void* arg) throw();

    static void _terminateSignalHandler(
        int s_n, PEGASUS_SIGINFO_T* s_info, void* sig);

#ifdef PEGASUS_OS_PASE
    static void _synchronousSignalHandler(
        int s_n, PEGASUS_SIGINFO_T* s_info, void* sig);
    static void _asynchronousSignalHandler(
        int s_n, PEGASUS_SIGINFO_T* s_info, void* sig);
#endif

    //
    // Private data
    //

    /**
        ProviderAgent reference used by the terminateSignalHandler to
        indicate that the Provider Agent should exit.
     */
    static ProviderAgent* _providerAgent;

    /**
        String used to identify this Provider Agent instance.
     */
    String _agentId;

    /**
        The pipe connection on which the Provider Agent reads requests
        from the CIM Server.
     */
    AnonymousPipe* _pipeFromServer;
    /**
        The pipe connection on which the Provider Agent writes responses
        (and indications) to the CIM Server.
     */
    AnonymousPipe* _pipeToServer;
    /**
        This lock must be held whenever writing on the _pipeToServer
        connection.
     */
    Mutex _pipeToServerMutex;

    /**
        Indicates whether the Provider Agent should exit.
     */
    Boolean _terminating;

    /**
        The ProviderManagerRouter to which all provider operations are
        forwarded for processing.
     */
    BasicProviderManagerRouter _providerManagerRouter;

    /**
        Indicates the number of threads currently attempting to unload idle
        providers.  This value is used to prevent multiple threads from
        unloading idle providers concurrently.
     */
    AtomicInt _unloadIdleProvidersBusy;

    /**
        Holds the last provider module instance received from the CIM Server
        in a ProviderIdContainer.  Since the provider module instance rarely
        changes, an optimization is used to send it only when it differs from
        the last provider module instance sent.
     */
    CIMInstance _providerModuleCache;

    /**
        A thread pool used for asynchronous processing of provider operations.
     */
    ThreadPool _threadPool;


    /**
     * Call back for the SCMOClassCache.
     */
    static SCMOClass _scmoClassCache_GetClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    void _processGetSCMOClassResponse(
        ProvAgtGetScmoClassResponseMessage* response);

    void _processStopAllProvidersRequest(CIMRequestMessage* request);

    static void _indicationDeliveryRoutine(
        CIMProcessIndicationRequestMessage* message);

    /**
     * Condition variable and transger pointer for the provider agend to
     * the SCMOClassCache.
     **/
    static Semaphore  _scmoClassDelivered;
    static SCMOClass* _transferSCMOClass;
    static Mutex       _transferSCMOClassMutex;
    static String      _transferSCMOClassRspMsgID;

    /**
       Indicates if the provider agent has been successful initialised already.
     */
    Boolean _isInitialised;

    /**
        Indicates whether the provider module is disabled successfully or
        CIMStopAllProvidersRequestMessage is processed successfully.
    */
    Boolean _providersStopped;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ProviderAgent_h */
