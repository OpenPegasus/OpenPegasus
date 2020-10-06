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

/////////////////////////////////////////////////////////////////////////////
//  SSLContextManager
/////////////////////////////////////////////////////////////////////////////

#ifdef PEGASUS_HAS_SSL
# include <Pegasus/Common/Network.h>
# define OPENSSL_NO_KRB5 1
# include <openssl/err.h>
# include <openssl/ssl.h>
# include <openssl/rand.h>
#else
# define SSL_CTX void
#endif // end of PEGASUS_HAS_SSL

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/FileSystem.h>

#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/SSLContextRep.h>
#include "SSLContextManager.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

//
// This method along with the relevent code is moved here
// from CIMServer.cpp
//
Boolean verifyClientOptionalCallback(SSLCertificateInfo &certInfo)
{
#if defined PEGASUS_OVERRIDE_SSL_CERT_VERIFICATION_RESULT
    // SSL callback for the "optional" client verification setting
    // By always returning true, we allow the handshake to continue
    // even if the client sent no certificate or sent an untrusted certificate.
    return true;
#else
    //
    // Return the OpenSSL verification result
    //
    return certInfo.getResponseCode();
#endif
}



SSLContextManager::SSLContextManager()
    : _sslContext(0)
{

}

SSLContextManager::~SSLContextManager()
{
    delete _sslContext;
}

//
// Part of this code logic comes from the CIMServer::_getSSLContext()
// and CIMServer::_getExportSSLContext() methods.
//
void SSLContextManager::createSSLContext(
    const String& trustStore,
    const String& certPath,
    const String& keyPath,
    const String& crlStore,
    Boolean callback, 
    const String& randFile,
    const String& cipherSuite,
    const Boolean& sslCompatibility)
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLContextManager::createSSLContext()");

    if ( !_sslContext )
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
            "Creating the Server SSL Context.");

        //
        // Create the SSLContext object
        //
        if ( callback )
        {
            _sslContext = new SSLContext(trustStore, certPath,
                keyPath, crlStore,
                (SSLCertificateVerifyFunction*)verifyClientOptionalCallback,
                randFile, cipherSuite,sslCompatibility);
        }
        else if ( trustStore != String::EMPTY )
        {
            _sslContext = new SSLContext(trustStore, certPath,
                keyPath, crlStore, 0, randFile, cipherSuite,sslCompatibility);
        }
        else
        {
            _sslContext = new SSLContext(String::EMPTY, certPath,
                keyPath, crlStore, 0, randFile, cipherSuite,sslCompatibility);
        }
    }

    PEG_METHOD_EXIT();
}

//
// use the following methods only if SSL is available
//
#ifdef PEGASUS_HAS_SSL

/**
    Create a new store object and load the specified store and return
    a pointer to the new store object.
*/
static X509_STORE* _getNewX509Store(const String& storePath)
{
    PEG_METHOD_ENTER(TRC_SSL, "_getNewX509Store()");

    //
    // reload certificates from the specified store
    //
    PEG_TRACE((TRC_SSL, Tracer::LEVEL3,
        "Reloading certificates from the store: %s",
        (const char*)storePath.getCString()));

    X509_STORE* newStore = X509_STORE_new();
    if (newStore == NULL)
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
            "Memory allocation error reloading the trust or crl store.");
        PEG_METHOD_EXIT();
        throw PEGASUS_STD(bad_alloc)();
    }

    //
    // Check if there is a CA certificate file or directory specified.
    // If specified, load the certificates from the specified store path.
    //
    if (FileSystem::isDirectory(storePath))
    {
        X509_LOOKUP* storeLookup = X509_STORE_add_lookup(newStore,
                                              X509_LOOKUP_hash_dir());
        if (storeLookup == NULL)
        {
            X509_STORE_free(newStore);

            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
                "Could not reload the trust or crl store.");

            MessageLoaderParms parms(
                "Pegasus.Common.SSLContextManager."
                    "COULD_NOT_RELOAD_TRUST_OR_CRL_STORE",
                "Could not reload the trust or certificate revocation list "
                    "store.");
            PEG_METHOD_EXIT();
            throw SSLException(parms);
        }
        X509_LOOKUP_add_dir(storeLookup,
            storePath.getCString(), X509_FILETYPE_PEM);
    }
    else if (FileSystem::exists(storePath))
    {
        X509_LOOKUP* storeLookup = X509_STORE_add_lookup(
            newStore, X509_LOOKUP_file());
        if (storeLookup == NULL)
        {
            X509_STORE_free(newStore);

            PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
                "Could not reload the trust or crl store.");

            MessageLoaderParms parms(
                "Pegasus.Common.SSLContextManager."
                    "COULD_NOT_RELOAD_TRUST_OR_CRL_STORE",
                "Could not reload the trust or certificate revocation list "
                    "store.");
            PEG_METHOD_EXIT();
            throw SSLException(parms);
        }
        X509_LOOKUP_load_file(storeLookup,
            storePath.getCString(), X509_FILETYPE_PEM);
    }
    else
    {
        X509_STORE_free(newStore);

        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
            "Could not reload the trust or crl store, configured store "
                "not found.");

        MessageLoaderParms parms(
            "Pegasus.Common.SSLContextManager."
                "CONFIGURED_TRUST_OR_CRL_STORE_NOT_FOUND",
            "Could not reload the trust or certificate revocation list store. "
                "The configured store is not found.");
        PEG_METHOD_EXIT();
        throw SSLException(parms);
    }

    PEG_METHOD_EXIT();
    return newStore;
}

void SSLContextManager::reloadTrustStore()
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLContextManager::reloadTrustStore()");

    SSL_CTX* sslContext;
    String trustStore;

    if ( _sslContext )
    {
        sslContext = _sslContext->_rep->getContext();
        trustStore = _sslContext->getTrustStore();
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
          "Could not reload the trust store, SSL Context is not initialized.");

        MessageLoaderParms parms(
            "Pegasus.Common.SSLContextManager."
                "COULD_NOT_RELOAD_TRUSTSTORE_SSL_CONTEXT_NOT_INITIALIZED",
            "Could not reload the trust store. The SSL context is not "
                "initialized.");
        PEG_METHOD_EXIT();
        throw SSLException(parms);
    }

    if (trustStore == String::EMPTY)
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
            "Could not reload the trust store, the trust store is not "
                "configured.");

        MessageLoaderParms parms(
            "Pegasus.Common.SSLContextManager.TRUST_STORE_NOT_CONFIGURED",
            "Could not reload the trust store, the trust store is not "
                "configured.");
        PEG_METHOD_EXIT();
        throw SSLException(parms);
    }

    X509_STORE* newStore = _getNewX509Store(trustStore);

    //
    // acquire write lock to Context object and then overwrite the trust
    // store cache
    //
    WriteLock contextLock(_sslContextObjectLock);
    SSL_CTX_set_cert_store(sslContext, newStore);

    PEG_METHOD_EXIT();
}

void SSLContextManager::reloadCRLStore()
{
    PEG_METHOD_ENTER(TRC_SSL, "SSLContextManager::reloadCRLStore()");

    if (!_sslContext)
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
        "Could not reload the crl store, SSL Context is not initialized.");

        MessageLoaderParms parms(
            "Pegasus.Common.SSLContextManager."
                "COULD_NOT_RELOAD_CRL_STORE_SSL_CONTEXT_NOT_INITIALIZED",
            "Could not reload the certificate revocation list store. "
                "The SSL Context is not initialized.");

        PEG_METHOD_EXIT();
        throw SSLException(parms);
    }

    String crlPath = _sslContext->getCRLPath();

    if (crlPath == String::EMPTY)
    {
        PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL1,
            "Could not reload the crl store, the crl store is not configured.");

        MessageLoaderParms parms(
            "Pegasus.Common.SSLContextManager.CRL_STORE_NOT_CONFIGURED",
            "Could not reload the certificate revocation list store. "
                "The certificate revocation list store is not configured.");
        PEG_METHOD_EXIT();
        throw SSLException(parms);
    }

    PEG_TRACE((TRC_SSL, Tracer::LEVEL4,
        "CRL store path is %s",(const char*)crlPath.getCString()));

    // update the CRL store

    {
        WriteLock contextLock(_sslContextObjectLock);
        if (_sslContext)
        {
            _sslContext->_rep->setCRLStore(_getNewX509Store(crlPath));
        }
    }

    PEG_METHOD_EXIT();
}

#else    //#ifdef PEGASUS_HAS_SSL

void SSLContextManager::reloadTrustStore() { }

void SSLContextManager::reloadCRLStore() { }

#endif   //#ifdef PEGASUS_HAS_SSL

/**
    Get a pointer to the sslContext object.
 */
SSLContext*  SSLContextManager::getSSLContext() const
{
    return _sslContext;
}

/**
    Get a pointer to the sslContextObjectLock.
 */
ReadWriteSem* SSLContextManager::getSSLContextObjectLock()
{
    return &_sslContextObjectLock;
}

PEGASUS_NAMESPACE_END
