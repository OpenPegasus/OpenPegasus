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

#ifndef Pegasus_TLS_h
#define Pegasus_TLS_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Socket.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/ReadWriteSem.h>

PEGASUS_NAMESPACE_BEGIN

#ifdef PEGASUS_HAS_SSL
class PEGASUS_COMMON_LINKAGE SSLSocket
{
public:

    SSLSocket(
        SocketHandle socket,
        SSLContext * sslcontext,
        ReadWriteSem * sslContextObjectLock,
        const String& ipAddress);

    ~SSLSocket();

    Boolean incompleteSecureReadOccurred(Sint32 retCode);

    Sint32 read(void* ptr, Uint32 size);

    Sint32 peek(void* ptr, Uint32 size);

    Sint32 timedWrite(const void* ptr,
                      Uint32 size,
                      Uint32 socketWriteTimeout);

    void close();

    void disableBlocking();

    static void initializeInterface();

    static void uninitializeInterface();

    SocketHandle getSocket() {return _socket;}

    /**
        Accepts the connection, performing the necessary SSL handshake.

        @return Returns -1 on failure, 0 if not enough data is available to
        complete the operation (retry needed), and 1 on success.
     */
    Sint32 accept();

    Sint32 connect(Uint32 timeoutMilliseconds);

    Boolean isPeerVerificationEnabled();

    Boolean isCertificateVerified();

    /**
        Gets peer certificate chain.

        @return array of SSLCertificateInfo pointers if there is an
                    SSLCallbackInfo pointer,
                Otherwise an empty array
     */
    Array<SSLCertificateInfo*> getPeerCertificateChain();

private:

    /**
        This member is of type SSL*, but we don't want to expose a dependency
        on the SSL include files in a header file.
    */
    void* _SSLConnection;

    SocketHandle _socket;
    SSLContext * _SSLContext;
    /**
        Keeps a copy of the CRL store from the _SSLContext to prevent using a
        dangling pointer when the CRL store is reloaded in the _SSLContext.
    */
    void* _crlStore;
    ReadWriteSem * _sslContextObjectLock;
    Uint32 _sslReadErrno;

    AutoPtr<SSLCallbackInfo> _SSLCallbackInfo;
    String _ipAddress;
    Boolean _certificateVerified;
};
#else

// offer a non ssl dummy class for use in MP_Socket

class SSLSocket {};

#endif // end of PEGASUS_HAS_SSL

//
// MP_Socket (Multi-purpose Socket class
//

class MP_Socket {

public:
    MP_Socket(SocketHandle socket);                          // "normal" socket

    MP_Socket(
        SocketHandle socket,
        SSLContext * sslcontext,
        ReadWriteSem * sslContextObjectLock,
        const String& ipAddress = String::EMPTY);

    ~MP_Socket();

    Boolean isSecure();

    Boolean incompleteSecureReadOccurred(Sint32 retCode);

    SocketHandle getSocket();

    Sint32 read(void* ptr, Uint32 size);

    Sint32 peek(void* ptr, Uint32 size);

    Sint32 write(const void* ptr, Uint32 size);

    void close();

    void disableBlocking();

    /**
        Accepts the connection, performing an SSL handshake if applicable.

        @return Returns -1 on failure, 0 if not enough data is available to
        complete the operation (retry needed), and 1 on success.
     */
    Sint32 accept();

    Sint32 connect(Uint32 timeoutMilliseconds);

    Boolean isPeerVerificationEnabled();

    Array<SSLCertificateInfo*> getPeerCertificateChain();

    Boolean isCertificateVerified();

    void setSocketWriteTimeout(Uint32 socketWriteTimeout);

#ifdef PEGASUS_OS_ZOS
    // Return the authenicated user name
    String getAuthenticatedUser() { return String(_username); }
    // Is the client authenticated ?
    Boolean isClientAuthenticated() { return _userAuthenticated; }
    // What was type of authentication ?
    String getAuthType() { return _authType; }
#endif

    union {
        SocketHandle _socket;
        SSLSocket *_sslsock;
    };

private:
    Boolean   _isSecure;
    Uint32    _socketWriteTimeout;

#ifdef PEGASUS_OS_ZOS
    // Query a AT-TLS secured socket for the authenticated
    // client userID.
    int ATTLS_zOS_query();

    // Query a UNIX Domain socket (local socket) for
    // the connected client userID.
    int LocalSocket_zOS_query();

    // The user name if authenticated.
    char _username[10];
    // Was a user authenticated ?
    Boolean _userAuthenticated;
    // Was it AT-TLS or UNIX Domain authentication ?
    String _authType;
#endif

};




PEGASUS_NAMESPACE_END

#endif /* Pegasus_TLS_h */
