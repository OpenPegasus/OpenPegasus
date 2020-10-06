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
#include <Pegasus/Common/Socket.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/SSLContextRep.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/AuditLogger.h>

#include "TLS.h"

#ifdef PEGASUS_HAS_SSL
# define OPENSSL_NO_KRB5 1
# include <openssl/err.h>
# include <openssl/ssl.h>
# include <openssl/rand.h>
#else
# define SSL_CTX void
typedef void SSL_Context;
#endif // end of PEGASUS_HAS_SSL

#ifdef PEGASUS_OS_ZOS
#include "SocketzOS_inline.h"
#endif

//
// use the following definitions only if SSL is available
//
#ifdef PEGASUS_HAS_SSL

PEGASUS_NAMESPACE_BEGIN

//
// Basic SSL socket
//

SSLSocket::SSLSocket(
    SocketHandle socket,
    SSLContext * sslcontext,
    ReadWriteSem * sslContextObjectLock,
    const String& ipAddress)
   :
   _SSLConnection(0),
   _socket(socket),
   _SSLContext(sslcontext),
   _sslContextObjectLock(sslContextObjectLock),
   _ipAddress(ipAddress),
   _certificateVerified(false)
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::SSLSocket()");

    SSL* sslConnection;
    SharedPtr<X509_STORE, FreeX509STOREPtr> tmpCrlStore;

    _sslReadErrno = 0;

    //
    // create the SSLConnection area
    //
    if (!(sslConnection = SSL_new(_SSLContext->_rep->getContext())))
    {
        PEG_METHOD_EXIT();
        MessageLoaderParms parms(
            "Common.TLS.COULD_NOT_GET_SSL_CONNECTION_AREA",
            "Could not get SSL Connection Area.");
        throw SSLException(parms);
    }

    // This try/catch block is necessary so that we can free the SSL Connection
    // Area if any exceptions are thrown.
    try
    {
        //
        // set the verification callback data
        //

        // we are only storing one set of data, so we can just use index 0,
        // this is defined in SSLContext.h
        //int index = SSL_get_ex_new_index(
        //    0, (void*)"pegasus", NULL, NULL, NULL);

        //
        // Create a new callback info for each new connection
        //
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
        tmpCrlStore = _SSLContext->_rep->getCRLStore();
#endif
        _SSLCallbackInfo.reset(new SSLCallbackInfo(
            _SSLContext->getSSLCertificateVerifyFunction(),
            tmpCrlStore.get(),
            _ipAddress ));

        if (SSL_set_ex_data(
                sslConnection,
                SSLCallbackInfo::SSL_CALLBACK_INDEX,
                _SSLCallbackInfo.get()))
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                "--->SSL: Set callback info");
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
                "--->SSL: Error setting callback info");
        }

        //
        // and connect the active socket with the ssl operation
        //
        if (!(SSL_set_fd(sslConnection, _socket) ))
        {
            PEG_METHOD_EXIT();
            MessageLoaderParms parms(
                "Common.TLS.COULD_NOT_LINK_SOCKET",
                "Could not link socket to SSL Connection.");
            throw SSLException(parms);
        }
    }
    catch (...)
    {
        SSL_free(sslConnection);
        throw;
    }

    _SSLConnection = sslConnection;
    _crlStore = new SharedPtr<X509_STORE, FreeX509STOREPtr>(tmpCrlStore);

    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4, "---> SSL: Created SSL socket");

    PEG_METHOD_EXIT();
}

SSLSocket::~SSLSocket()
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::~SSLSocket()");

    close();
    delete static_cast<SharedPtr<X509_STORE, FreeX509STOREPtr>*>(_crlStore);
    SSL_free(static_cast<SSL*>(_SSLConnection));

    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL3, "---> SSL: Deleted SSL socket");

    PEG_METHOD_EXIT();
}


Boolean SSLSocket::incompleteSecureReadOccurred(Sint32 retCode)
{
    Sint32 err = SSL_get_error(static_cast<SSL*>(_SSLConnection), retCode);

    Boolean isIncompleteRead =
        ((err == SSL_ERROR_SYSCALL) &&
        (_sslReadErrno == EAGAIN || _sslReadErrno == EINTR)) ||
        (err == SSL_ERROR_WANT_READ) ||
        (err == SSL_ERROR_WANT_WRITE);

    if (Tracer::isTraceOn())
    {
        unsigned long rc = ERR_get_error ();
        char buff[256];
        ERR_error_string_n (rc, buff, sizeof (buff)); // added in OpenSSL 0.9.6

        PEG_TRACE((TRC_SSL, Tracer::LEVEL4,
            "In SSLSocket::incompleteSecureReadOccurred : err = %d %s",
            err, buff));

        if (!isIncompleteRead && retCode < 0)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL4,
                "In SSLSocket::incompleteSecureReadOccurred : err = %d %s",
                err, buff));
        }
    }

    return isIncompleteRead;
}

Sint32 SSLSocket::read(void* ptr, Uint32 size)
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::read()");
    Sint32 rc;

    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4, "---> SSL: (r) ");
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
        SSL_state_string_long(static_cast<SSL*>(_SSLConnection)));
    rc = SSL_read(static_cast<SSL*>(_SSLConnection), (char *)ptr, size);

    _sslReadErrno = errno;

    PEG_METHOD_EXIT();
    return rc;
}

Sint32 SSLSocket::peek(void* ptr, Uint32 size)
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::peek()");
    Sint32 rc;

    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4, "---> SSL: (r) ");
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                      SSL_state_string_long(static_cast<SSL*>(_SSLConnection)));
    rc = SSL_peek(static_cast<SSL*>(_SSLConnection), (char *)ptr, size);

    _sslReadErrno = errno;

    PEG_METHOD_EXIT();
    return rc;
}

Sint32 SSLSocket::timedWrite( const void* ptr,
                              Uint32 size,
                              Uint32 socketWriteTimeout)
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::timedWrite()");
    Sint32 bytesWritten = 0;
    Sint32 totalBytesWritten = 0;
    Boolean socketTimedOut = false;
    Uint32 selreturn = 0;

  while (1)
  {
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4, "---> SSL: (w) ");
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
        SSL_state_string_long(static_cast<SSL*>(_SSLConnection)));
    bytesWritten =
        SSL_write(static_cast<SSL*>(_SSLConnection), (char *)ptr, size);

    // Some data written this cycle ?
    // Add it to the total amount of written data.
    if (bytesWritten > 0)
    {
        totalBytesWritten += bytesWritten;
        socketTimedOut = false;
    }

    // All data written ? return amount of data written
    if ((Uint32)bytesWritten == size)
    {
        // exit the while loop
        break;
    }
    // If data has been written partially, we resume writing data
    // this also accounts for the case of a signal interrupt
    // (i.e. errno = EINTR)
    if (bytesWritten > 0)
    {
        size -= bytesWritten;
        ptr = (void *)((char *)ptr + bytesWritten);
        continue;
    }
    // Something went wrong, SSL return with everything not > 0 is an error
    if (bytesWritten <= 0)
    {
        // if we already waited for the socket to get ready, bail out
        if (socketTimedOut)
        {
            // bytesWritten contains the error indication
            PEG_METHOD_EXIT();
            return bytesWritten;
        }

        // just interrupted by a signal, try again
#ifdef PEGASUS_OS_TYPE_WINDOWS
        if (WSAGetLastError() == WSAEINTR) continue;
#else
        if (errno == EINTR) continue;
#endif

        // socket not ready ...
#ifdef PEGASUS_OS_TYPE_WINDOWS
        if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
        if (errno == EAGAIN || errno == EWOULDBLOCK)
#endif
        {
            fd_set fdwrite;
            // max. timeout seconds waiting for the socket to get ready
            struct timeval tv = {socketWriteTimeout , 0 };
            FD_ZERO(&fdwrite);
            FD_SET(_socket, &fdwrite);
            selreturn = select(FD_SETSIZE, NULL, &fdwrite, NULL, &tv);
            if (selreturn == 0) socketTimedOut = true; // ran out of time
            continue;
        }
        // bytesWritten contains the error indication
        PEG_METHOD_EXIT();
        return bytesWritten;
    }
  }
    PEG_METHOD_EXIT();
    return totalBytesWritten;
}

void SSLSocket::close()
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::close()");

    SSL_shutdown(static_cast<SSL*>(_SSLConnection));
    Socket::close(_socket);

    PEG_METHOD_EXIT();
}

void SSLSocket::disableBlocking()
{
    Socket::disableBlocking(_socket);
}

void SSLSocket::initializeInterface()
{
    Socket::initializeInterface();
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL3, "---> SSL: initialized SSL");
}

void SSLSocket::uninitializeInterface()
{
    Socket::uninitializeInterface();
}

Sint32 SSLSocket::accept()
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::accept()");

    SSL* sslConnection = static_cast<SSL*>(_SSLConnection);
    Sint32 ssl_rc,ssl_rsn;

    //ATTN: these methods get implicitly done with the SSL_accept call
    //SSL_do_handshake(sslConnection);
    //SSL_set_accept_state(sslConnection);

    // Make sure the SSLContext object is not updated during this operation.
    ReadLock rlock(*_sslContextObjectLock);

    ssl_rc = SSL_accept(sslConnection);

    if (ssl_rc < 0)
    {
        ssl_rsn = SSL_get_error(sslConnection, ssl_rc);

        if ((ssl_rsn == SSL_ERROR_WANT_READ) ||
            (ssl_rsn == SSL_ERROR_WANT_WRITE))
        {
            PEG_METHOD_EXIT();
            return 0;
        }
        else
        {
            if (Tracer::isTraceOn())
            {
                unsigned long rc = ERR_get_error ();
                char buff[256];
                // added in OpenSSL 0.9.6:
                ERR_error_string_n(rc, buff, sizeof(buff));
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "---> SSL: Not accepted %d %s client IP address : %s",
                    ssl_rsn, buff, (const char*)_ipAddress.getCString() ));
            }

            //
            // If there was a verification error, create a audit log entry.
            //
            if (!(ssl_rsn == SSL_ERROR_SYSCALL ||
                  ssl_rsn == SSL_ERROR_ZERO_RETURN) &&
                _SSLContext->isPeerVerificationEnabled())
            {
                Array<SSLCertificateInfo*> certs =
                    getPeerCertificateChain();
                if (certs.size() > 0)
                {
                    SSLCertificateInfo* clientCert = certs[0];
                    PEGASUS_ASSERT(clientCert != NULL);

                    char serialNumberString[32];
                    sprintf(serialNumberString, "%lu",
                        (unsigned long)clientCert->getSerialNumber());

                    PEG_AUDIT_LOG(logCertificateBasedAuthentication(
                        clientCert->getIssuerName(),
                        clientCert->getSubjectName(),
                        serialNumberString,
                        _ipAddress,
                        false));
                }
            }
            PEG_METHOD_EXIT();
            return -1;
        }
    }
    else if (ssl_rc == 0)
    {
       PEG_TRACE((
           TRC_SSL,
           Tracer::LEVEL1,
           "Shutdown SSL_accept(). Error Code:  %d  Error string: %s",
           SSL_get_error(sslConnection, ssl_rc),
           ERR_error_string(ssl_rc, NULL)));

       PEG_METHOD_EXIT();
       return -1;
    }
    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4, "---> SSL: Accepted");

    //
    // If peer certificate verification is enabled or request received on
    // export connection, get the peer certificate and verify the trust
    // store validation result.
    //
    if (_SSLContext->isPeerVerificationEnabled())
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
            "Attempting to certify client");

        //
        // get client's certificate
        //
        Array<SSLCertificateInfo*> certs = getPeerCertificateChain();
        if (certs.size() > 0)
        {
            SSLCertificateInfo* clientCert = certs[0];
            PEGASUS_ASSERT(clientCert != NULL);

            //
            // get certificate verification result and create a audit log entry.
            //
            int verifyResult = SSL_get_verify_result(sslConnection);
            PEG_TRACE((TRC_SSL, Tracer::LEVEL4,
                "Verification Result:  %d", verifyResult ));
            _certificateVerified = (verifyResult == X509_V_OK);

            char serialNumberString[32];
            sprintf(serialNumberString, "%lu",
                (unsigned long)clientCert->getSerialNumber());

            PEG_AUDIT_LOG(logCertificateBasedAuthentication(
                clientCert->getIssuerName(),
                clientCert->getSubjectName(),
                serialNumberString,
                _ipAddress,
                _certificateVerified));
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL3,
                "---> SSL: Client not certified, no certificate received");
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
            "---> SSL: Client certificate verification disabled");
    }

    PEG_METHOD_EXIT();
    return 1;
}

Sint32 SSLSocket::connect(Uint32 timeoutMilliseconds)
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLSocket::connect()");

    PEG_TRACE((TRC_SSL, Tracer::LEVEL4,
        "Connection timeout in milliseconds is : %d", timeoutMilliseconds));

    SSL* sslConnection = static_cast<SSL*>(_SSLConnection);
    SSL_set_connect_state(sslConnection);

    while (1)
    {
        int ssl_rc = SSL_connect(sslConnection);

        if (ssl_rc > 0)
        {
            // Connected!
            break;
        }

        if (ssl_rc == 0)
        {
            PEG_TRACE((
                TRC_SSL,
                Tracer::LEVEL1,
                "---> SSL: Shutdown SSL_connect() failed. Error string: %s",
                ERR_error_string(ssl_rc, NULL)));

            PEG_METHOD_EXIT();
            return -1;
        }

        // Error case:  ssl_rc < 0

        int ssl_rsn = SSL_get_error(sslConnection, ssl_rc);

        if ((ssl_rsn == SSL_ERROR_SYSCALL) &&
            ((errno == EAGAIN) || (errno == EINTR)))
        {
            // Temporary error; retry the SSL_connect()
            continue;
        }

        if ((ssl_rsn != SSL_ERROR_WANT_READ) &&
            (ssl_rsn != SSL_ERROR_WANT_WRITE))
        {
            // Error, connection failed
            if (Tracer::isTraceOn())
            {
                unsigned long rc = ERR_get_error ();
                char buff[256];
                // added in OpenSSL 0.9.6:
                ERR_error_string_n(rc, buff, sizeof(buff));
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                    "---> SSL: Not connected %d %s", ssl_rsn, buff));
            }

            PEG_METHOD_EXIT();
            return -1;
        }

        // Wait until the socket is ready for reading or writing (as
        // appropriate) and then retry the SSL_connect()

        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(_socket, &fd);
        struct timeval timeoutValue =
            { timeoutMilliseconds/1000, timeoutMilliseconds%1000*1000 };
        int selectResult = -1;

        if (ssl_rsn == SSL_ERROR_WANT_READ)
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                "---> SSL: Retry WANT_READ");
            PEGASUS_RETRY_SYSTEM_CALL(
                select(FD_SETSIZE, &fd, NULL, NULL, &timeoutValue),
                selectResult);
        }
        else    // (ssl_rsn == SSL_ERROR_WANT_WRITE)
        {
            PEGASUS_ASSERT(ssl_rsn == SSL_ERROR_WANT_WRITE);
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                "---> SSL: Retry WANT_WRITE");
            PEGASUS_RETRY_SYSTEM_CALL(
                select(FD_SETSIZE, NULL, &fd, NULL, &timeoutValue),
                selectResult);
        }

        // Check the result of select.
        if (selectResult == 0)
        {
            PEG_TRACE_CSTRING(TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "---> SSL: Failed to connect, connection timed out.");
            PEG_METHOD_EXIT();
            return -1;
        }
        else if (selectResult == PEGASUS_SOCKET_ERROR)
        {
            PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL1,
                "---> SSL: Failed to connect, select error, return code = %d",
                selectResult));
            PEG_METHOD_EXIT();
            return -1;
        }
        // else retry the SSL_connect()
    }

    PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL3, "---> SSL: Connected");

    if (_SSLContext->isPeerVerificationEnabled())
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
           "Attempting to verify server certificate.");

        X509* server_cert = SSL_get_peer_certificate(sslConnection);
        if (server_cert != NULL)
        {
            //
            // Do not check the verification result using
            // SSL_get_verify_result here to see whether or not to continue.
            // The prepareForCallback does not reset the verification result,
            // so it will still contain the original error.  If the client
            // chose to override the default error in the callback and
            // return true, we got here and should proceed with the
            // transaction.  Otherwise, the handshake was already terminated.
            //

            if (SSL_get_verify_result(sslConnection) == X509_V_OK)
            {
                 PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                     "--->SSL: Server Certificate verified.");
            }
            else
            {
                PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                     "--->SSL: Server Certificate not verified, but the "
                         "callback overrode the default error.");
            }

            X509_free (server_cert);
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
                "-->SSL: Server not certified, no certificate received.");
            PEG_METHOD_EXIT();
            return -1;
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
            "---> SSL: Server certification disabled");
    }

    PEG_METHOD_EXIT();
    return 1;
}

Boolean SSLSocket::isPeerVerificationEnabled()
{
    return _SSLContext->isPeerVerificationEnabled();
}

Array<SSLCertificateInfo*> SSLSocket::getPeerCertificateChain()
{
    Array<SSLCertificateInfo*> peerCertificate;

    if (_SSLCallbackInfo.get())
    {
        peerCertificate = _SSLCallbackInfo->_rep->peerCertificate;
    }

    return peerCertificate;
}

Boolean SSLSocket::isCertificateVerified()
{
    return _certificateVerified;
}


//
// MP_Socket (Multi-purpose Socket class)
//



MP_Socket::MP_Socket(SocketHandle socket)
 : _socket(socket), _isSecure(false),
   _socketWriteTimeout(PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS) {}

MP_Socket::MP_Socket(
    SocketHandle socket,
    SSLContext * sslcontext,
    ReadWriteSem * sslContextObjectLock,
    const String& ipAddress)
{
    PEG_METHOD_ENTER(TRC_SSL, "MP_Socket::MP_Socket()");
    if (sslcontext != NULL)
    {
        _isSecure = true;
        _sslsock = new SSLSocket(
            socket, sslcontext, sslContextObjectLock, ipAddress);
    }
    else
    {
        _isSecure = false;
        _socket = socket;
    }

    // PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS
    // seconds are the default for client timeouts
    _socketWriteTimeout = PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS;
    PEG_METHOD_EXIT();
}

MP_Socket::~MP_Socket()
{
    PEG_METHOD_ENTER(TRC_SSL, "MP_Socket::~MP_Socket()");
    if (_isSecure)
    {
        delete _sslsock;
    }
    PEG_METHOD_EXIT();
}

Boolean MP_Socket::isSecure() {return _isSecure;}

Boolean MP_Socket::incompleteSecureReadOccurred(Sint32 retCode)
{
    if (_isSecure)
        return _sslsock->incompleteSecureReadOccurred(retCode);
    return false;
}

SocketHandle MP_Socket::getSocket()
{
    if (_isSecure)
        return _sslsock->getSocket();
    else
        return _socket;
}

Sint32 MP_Socket::read(void * ptr, Uint32 size)
{
    if (_isSecure)
        return _sslsock->read(ptr,size);
    else
        return Socket::read(_socket,ptr,size);
}

Sint32 MP_Socket::peek(void * ptr, Uint32 size)
{
    if (_isSecure)
        return _sslsock->peek(ptr,size);
    else
        return Socket::peek(_socket,ptr,size);
}

Sint32 MP_Socket::write(const void * ptr, Uint32 size)
{
    if (_isSecure)
        return _sslsock->timedWrite(ptr,size,_socketWriteTimeout);
    else
        return Socket::timedWrite(_socket,ptr,size,_socketWriteTimeout);
}

void MP_Socket::close()
{
    if (_isSecure)
        _sslsock->close();
    else
        Socket::close(_socket);
}

void MP_Socket::disableBlocking()
{
    if (_isSecure)
        _sslsock->disableBlocking();
    else
       Socket::disableBlocking(_socket);
}

Sint32 MP_Socket::accept()
{
    if (_isSecure)
    {
        return _sslsock->accept();
    }
    return 1;
}

Sint32 MP_Socket::connect(Uint32 timeoutMilliseconds)
{
    if (_isSecure)
        if (_sslsock->connect(timeoutMilliseconds) < 0) return -1;
    return 0;
}

Boolean MP_Socket::isPeerVerificationEnabled()
{
    if (_isSecure)
    {
        return _sslsock->isPeerVerificationEnabled();
    }
    return false;
}

Array<SSLCertificateInfo*> MP_Socket::getPeerCertificateChain()
{
    Array<SSLCertificateInfo*> peerCertificate;

    if (_isSecure)
    {
        peerCertificate = _sslsock->getPeerCertificateChain();
    }
    return peerCertificate;
}

Boolean MP_Socket::isCertificateVerified()
{
    if (_isSecure)
    {
        return _sslsock->isCertificateVerified();
    }
    return false;
}

void MP_Socket::setSocketWriteTimeout(Uint32 socketWriteTimeout)
{
    _socketWriteTimeout = socketWriteTimeout;
}

PEGASUS_NAMESPACE_END

#else

PEGASUS_NAMESPACE_BEGIN

#ifndef PEGASUS_OS_ZOS

MP_Socket::MP_Socket(SocketHandle socket)
 : _socket(socket), _isSecure(false),
   _socketWriteTimeout(PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS) {}

MP_Socket::MP_Socket(
    SocketHandle socket,
    SSLContext *,
    ReadWriteSem *,
    const String&)
 : _socket(socket), _isSecure(false),
   _socketWriteTimeout(PEGASUS_DEFAULT_SOCKETWRITE_TIMEOUT_SECONDS) {}

#endif

MP_Socket::~MP_Socket()
{
    close();
}

Boolean MP_Socket::isSecure() {return _isSecure;}

Boolean MP_Socket::incompleteSecureReadOccurred(Sint32)
{
    return false;
}

SocketHandle MP_Socket::getSocket()
{
    return _socket;
}

Sint32 MP_Socket::read(void * ptr, Uint32 size)
{
    return Socket::read(_socket,ptr,size);
}

Sint32 MP_Socket::peek(void * ptr, Uint32 size)
{
    return Socket::peek(_socket,ptr,size);
}

Sint32 MP_Socket::write(const void * ptr, Uint32 size)
{
    return Socket::timedWrite(_socket,ptr,size,_socketWriteTimeout);
}

void MP_Socket::close()
{
    Socket::close(_socket);
}

void MP_Socket::disableBlocking()
{
    Socket::disableBlocking(_socket);
}

Sint32 MP_Socket::accept()
{
#ifndef PEGASUS_OS_ZOS
    return 1;
#else
    PEG_METHOD_ENTER(TRC_SSL, "MP_Socket::accept()");
    // ************************************************************************
    // This is a z/OS specific section. No other platform can port this.
    // Pegasus on z/OS has no OpenSSL but cat use a transparent layer called
    // AT-TLS ( Applicatin Transparent Transport Layer Security ) to handle
    // HTTPS connections.
    // ************************************************************************

    int rc;

    if (isSecure())
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4, "---> HTTPS processing.");
        rc = ATTLS_zOS_query();
    }
    else
    {
        // ********************************************************************
        // If the socket is a UNIX Domain socket on z/OS, the local security
        // credentials are read form the socket.
        // ********************************************************************
        LocalSocket_zOS_query();
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
            "---> Normal HTTP processing.");
        rc = 1;
    }
    PEG_METHOD_EXIT();
    return rc;
#endif
}

Sint32 MP_Socket::connect(Uint32) { return 0; }

Boolean MP_Socket::isPeerVerificationEnabled() { return false; }

Array<SSLCertificateInfo*> MP_Socket::getPeerCertificateChain()
{
    //
    //  Return empty array
    //
    return Array<SSLCertificateInfo*>();
}

Boolean MP_Socket::isCertificateVerified() { return false; }

void MP_Socket::setSocketWriteTimeout(Uint32 socketWriteTimeout)
{
    _socketWriteTimeout = socketWriteTimeout;
}


PEGASUS_NAMESPACE_END

#endif // end of PEGASUS_HAS_SSL
