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
// Author: Dong Xiang, EMC Corporation (xiang_dong@emc.com)
//
// Modified By: David Dillard, VERITAS Software Corp.
//                  (david.dillard@veritas.com)
//              Aruran, IBM (ashanmug@in.ibm.com) for Bug# 3604
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Listener_h
#define Pegasus_Listener_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Listener/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

class SSLContext;
class CIMIndicationConsumer;


/** CIMListener provides the interfaces that allows CIMListeners to be
    created and managed. CIMListeners are the receivers for cim/xml indications.
    ATTN: KS needs to be completed.TBD
*/
class PEGASUS_LISTENER_LINKAGE CIMListener
{
public:
    /**
     * Constructs a CIMListener object.
     *
     * @param portNumber the specified socket port the listener will listen to
     * @param sslContext the specifed SSL context
     */
    CIMListener(Uint32 portNumber, SSLContext* sslContext=NULL);

    /**
     * Destructor of a CIMLIstener object.
     */
    ~CIMListener();

    /**
     * Returns the socket port number
     *
     * @return the socket port number.
     */
    Uint32 getPortNumber() const;

    /**
     * Returns the SSL context
     *
     * @return the SSL context.
     */
    SSLContext* getSSLContext() const;

    /**
     * Sets the SSL context
     *
     * @param the SSL context.
     */
    void setSSLContext(SSLContext* sslContext);

    /**
     * Starts for listening.
     */
    void start();

    /**
     * Stops listening
     */
    void stop();

    /**
     * Returns if the listener is active
     *
     * @return true if the listener is active;
     *              false otherwise.
     */
    Boolean isAlive() const;

    /**
     * Adds a CIMIndicationConsumer to the listener
     *
     * @param consumer the CIMIndicationConsumer to add.
     * @return true if the indication consumer has been added successfully
     *              false otherwise.
     */
    Boolean addConsumer(CIMIndicationConsumer* consumer);

    /**
     * Removes a CIMIndicationConsumer from the listener.
     *
     * @param the CIMIndicationConsumer to remove.
     * @return true if the indication consumer has been removed successfully
     *              false otherwise.
     */
    Boolean removeConsumer(CIMIndicationConsumer* consumer);

private:
    /*
     * Copy constructor - not implemented
     */
    CIMListener(const CIMListener &);

    /*
     * Assignment operator - not implemented
     */
    CIMListener &operator=(const CIMListener &);

    /*
     * rep object
     */
    void* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Listener_h */
