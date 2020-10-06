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

#ifndef Pegasus_SSLContextManager_h
#define Pegasus_SSLContextManager_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/ReadWriteSem.h>

#include <Pegasus/General/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class provides the functionality necessary to manage SSLContext
    objects.
 */
class PEGASUS_GENERAL_LINKAGE SSLContextManager
{
public:
    /** Constructor. */

    SSLContextManager();

    ~SSLContextManager();

    void createSSLContext(
        const String& trustStore,
        const String& certPath,
        const String& keyPath,
        const String& crlStore,
        Boolean callback,
        const String& randFile,
        const String& cipherSuite,
        const Boolean& sslCompatibility);

    /**
        Reload the trust store used by either the CIM Server or
        Indication Server based on the context type.
     */
    void reloadTrustStore();

    /**
        Reload the CRL store.
     */
    void reloadCRLStore();

    /**
        Get a pointer to the sslContext object.
     */
    SSLContext*  getSSLContext() const;

    /**
        Get a pointer to the sslContextObjectLock.
     */
    ReadWriteSem*  getSSLContextObjectLock();

private:
    /**
        A lock to control access to the _sslContext object.
        Before read accessing the _sslContext object, one must first
        lock this for read access.  Before write accessing the _sslContext
        object, one must first lock this for write access.
     */
    ReadWriteSem _sslContextObjectLock;
    SSLContext* _sslContext;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SSLContextManager_h */
