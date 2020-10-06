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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_SSLContext_h
#define Pegasus_SSLContext_h

#include <Pegasus/Common/CIMDateTime.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Linkage.h>

#ifdef PEGASUS_HAS_SSL
typedef struct x509_store_st X509_STORE;
#else
#define X509_STORE void
#endif

PEGASUS_NAMESPACE_BEGIN

class SSLCertificateInfoRep;
class SSLContextRep;
class SSLContext;
class SSLSocket;
class CIMServer;
class CIMxmlIndicationHandler;
class SSLCertificateInfo;
class SSLCallback;
class SSLContextManager;
class SSLCallbackInfoRep;

// Pegasus-defined SSL certificate verification callback
typedef Boolean (SSLCertificateVerifyFunction) (SSLCertificateInfo &certInfo);

/** This class provides information that is used during the SSL verification callback.
    We pass a pointer to this object to the SSL_set_ex_data function.  We can then use SSL_get_ex_data
    from within the callback and cast the void* back to this object.  In this case, we store a pointer
    to the Pegasus-defined callback function set in the SSLContext.  We also store a pointer to a
    certificate object which we construct during the callback.  Some of the certificate information is
    inaccessible outside the callback, so we need to retrieve the data within the function.
    Each SSL connection object will have the same callback function, but each connection will have its
    own certificate.  Therefore, this class is constructed on a per-connection basis in SSLSocket.
*/
class PEGASUS_COMMON_LINKAGE SSLCallbackInfo
{
public:

    // index to the application-specific data in the SSL connection object
    static const int SSL_CALLBACK_INDEX;

    SSLCallbackInfo(SSLCertificateVerifyFunction* verifyCert);

    SSLCallbackInfo(
        SSLCertificateVerifyFunction* verifyCert,
        X509_STORE* crlStore);

    ~SSLCallbackInfo();

private:

    SSLCallbackInfo();
    SSLCallbackInfo(const SSLCallbackInfo& sslCallbackInfo);
    SSLCallbackInfo& operator=(const SSLCallbackInfo& sslCallbackInfo);

    SSLCallbackInfoRep* _rep;

    friend class SSLSocket;

    friend class SSLCallback;
};


/** This class provides the interface that a client gets as argument
    to certificate verification call back function.
*/
class PEGASUS_COMMON_LINKAGE SSLCertificateInfo
{
public:

    //
    // Certificate validation result codes.
    //
    static const int    V_OK;

    static const int    V_ERR_UNABLE_TO_GET_ISSUER_CERT;
    static const int    V_ERR_UNABLE_TO_GET_CRL;
    static const int    V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE;
    static const int    V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE;
    static const int    V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY;
    static const int    V_ERR_CERT_SIGNATURE_FAILURE;
    static const int    V_ERR_CRL_SIGNATURE_FAILURE;
    static const int    V_ERR_CERT_NOT_YET_VALID;
    static const int    V_ERR_CERT_HAS_EXPIRED;
    static const int    V_ERR_CRL_NOT_YET_VALID;
    static const int    V_ERR_CRL_HAS_EXPIRED;
    static const int    V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD;
    static const int    V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD;
    static const int    V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD;
    static const int    V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD;
    static const int    V_ERR_OUT_OF_MEM;
    static const int    V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT;
    static const int    V_ERR_SELF_SIGNED_CERT_IN_CHAIN;
    static const int    V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY;
    static const int    V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE;
    static const int    V_ERR_CERT_CHAIN_TOO_LONG;
    static const int    V_ERR_CERT_REVOKED;
    static const int    V_ERR_INVALID_CA;
    static const int    V_ERR_PATH_LENGTH_EXCEEDED;
    static const int    V_ERR_INVALID_PURPOSE;
    static const int    V_ERR_CERT_UNTRUSTED;
    static const int    V_ERR_CERT_REJECTED;
    static const int    V_ERR_SUBJECT_ISSUER_MISMATCH;
    static const int    V_ERR_AKID_SKID_MISMATCH;
    static const int    V_ERR_AKID_ISSUER_SERIAL_MISMATCH;
    static const int    V_ERR_KEYUSAGE_NO_CERTSIGN;

    static const int    V_ERR_APPLICATION_VERIFICATION;


    /** Constructor for a SSLCertificateInfo object.
        Note: Do not use this constructor, instead use the private constructor.
        The constructor is not for client applications use, it is intended to be
        used only by the CIMServer.
        @param subjectName subject name of the certificate.
        @param issuerName  issuer name of the certificate.
        @param errorDepth  depth of the certificate chain.
        @param errorCode   error code from the default verification of the
        certificate by the OpenSSL library.
        @param respCode   result code from the default verification of the
        certificate by the OpenSSL library.
    */
    SSLCertificateInfo(
        const String subjectName,
        const String issuerName,
        const int errorDepth,
        const int errorCode,
        const int respCode);

    /** Copy constructor for a SSLCertificateInfo object.
        @param certificateInfo SSLCertificateInfo object to copy
    */
    SSLCertificateInfo(const SSLCertificateInfo& certificateInfo);

    ~SSLCertificateInfo();

    /** Gets the subject name of the certificate.
        @return a string containing the subject name.
    */
    String getSubjectName() const;

    /** Gets the issuer name of the certificate.
        @return a string containing the issuer name.
    */
    String getIssuerName() const;

    /** Gets the notAfter date from the validity period of
        the certificate.
        @return a CIMDateTime containing the notAfter date.
    */
    CIMDateTime getNotAfter() const;

    /** Gets the notBefore date from the validity period of
        the certificate.
        @return a CIMDateTime containing the notBefore date.
    */
    CIMDateTime getNotBefore() const;

    /** Gets the version (version number) from the certificate.
        @return a int containing the version.
    */
    Uint32 getVersionNumber() const;

    /** Gets the serialNumber value from the certificate.
        @return a long integer containing the serial number.
    */
    long getSerialNumber() const;

    /** Gets the depth of the certificate chain.
        @return an int containing the depth of the certificate chain
    */
    Uint32 getErrorDepth() const;

    /** Gets the pre-verify error code.
        @return an int containing the pre-verify error code
    */
    Uint32 getErrorCode() const;

    /** Sets the error code.
        @param errorCode error code to be set
    */
    void setErrorCode(const int errorCode);

    /** Gets the pre-verify error string.
        @return a string containing the pre-verify error string
    */
    String getErrorString() const;

    /** Gets the pre-verify response code.
        @return an int containing the pre-verify response code
    */
    Uint32 getResponseCode() const;

    /** Sets the response code.
        Note: Do not use this function, the value set using this function
        is ignored.
        @param respCode response code to be set.
    */
    void setResponseCode(const int respCode);

    /** Returns a string representation of this object
        @return a string containing the certificate fields
    */
    String toString() const;

private:

    /** Constructor for a SSLCertificateInfo object.
        @param subjectName subject name of the certificate.
        @param issuerName  issuer name of the certificate.
        @param version version number value from the certificate.
        @param serailNumber serial number value from the certificate.
        @param notAfter notAfter date from the validity period of the certificate.
        @param notBefore notBefore date from the validity period of the certificate.
        @param depth  depth of the certificate chain.
        @param errorCode   error code from the default verification of the
        certificate by the OpenSSL library.
        @param errorString error message from the default verification of the
        certificate by the Open SSL library.
        @param respCode   result code from the default verification of the
        certificate by the OpenSSL library.
    */
    SSLCertificateInfo(
        const String subjectName,
        const String issuerName,
        const Uint32 versionNumber,
        const long   serialNumber,
        const CIMDateTime notBefore,
        const CIMDateTime notAfter,
        const Uint32 depth,
        const Uint32 errorCode,
        const String errorString,
        const Uint32 respCode);

    SSLCertificateInfo();
    SSLCertificateInfo& operator=(const SSLCertificateInfo& sslCertificateInfo);

    SSLCertificateInfoRep* _rep;

    // SSLSocket needs to use the private constructor to create
    // a certificate object to pass to the AuthenticationInfo and
    // OperationContext classes
    friend class SSLSocket;

    friend class SSLCallback;
};

/** This class provides the interface that a client uses to create
    SSL context.

    For the OSs that don't have /dev/random device file,
    must enable PEGASUS_SSL_RANDOMFILE flag and pass
    random file name to constructor.
*/
class PEGASUS_COMMON_LINKAGE SSLContext
{
public:

    /** Constructor for a SSLContext object.
        @param trustStore file path of the trust store
        @param verifyCert  function pointer to a certificate verification
        call back function.  A null pointer indicates that no callback is
        requested for certificate verification.
        @param randomFile  file path of a random file that is used as a seed
        for random number generation by OpenSSL.

        @exception SSLException indicates failure to create an SSL context.
    */
    SSLContext(
        const String& trustStore,
        SSLCertificateVerifyFunction* verifyCert,
        const String& randomFile = String::EMPTY);

    SSLContext(const SSLContext& sslContext);

    ~SSLContext();

    /** Gets the truststore path of the SSLContext object.  This may be a CA file or a directory.
        @return a string containing the truststore path.
    */
    String getTrustStore() const;

    /** Gets the x509 certificate path of the SSLContext object.
        @return a string containing the certificate path.
    */
    String getCertPath() const;

    /** Gets the private key path of the SSLContext object.
        @return a string containing the key path
    */
    String getKeyPath() const;

    //PEP187
    /** Gets the certificate revocation list path of the SSLContext object.
        @return a string containing the crl path
    */
    String getCRLPath() const;

    //PEP187
    /** Gets the certificate revocation store of the SSLContext object.
        @return a string containing the crl store
    */
    X509_STORE* getCRLStore() const;

    /** Returns whether peer verification is ON of OFF
        Corresponds to what the SSL_CTX_set_verify is set to
        @return true if verification is on; false otherwise
    */
    Boolean isPeerVerificationEnabled() const;

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
    /** In OpenPegasus 2.4 this method returned the username associated
        with the truststore, if applicable. This method is currently deprecated
        beginning in OpenPegasus 2.5, and will always return String::EMPTY.
        @return String::EMPTY
    */
    String getTrustStoreUserName() const;
#endif

    /** Returns the verification callback associated with this context.  This may be NULL.
        @return the verification callback function
    */
    SSLCertificateVerifyFunction* getSSLCertificateVerifyFunction() const;

    /** Constructor for a SSLContext object. This constructor is intended
        to be used by the CIMServer or CIMClient.
        @param trustStore file path of the trust store.
        @param certPath  file path of the server certificate.
        @param KeyPath  file path of the private key.
        @param verifyCert  function pointer to a certificate verification
        call back function.  A null pointer indicates that no callback is
        requested for certificate verification.
        @param randomFile  file path of a random file that is used as a seed
        for random number generation by OpenSSL.

        @exception SSLException indicates failure to create an SSL context.
    */
    SSLContext(
        const String& trustStore,
        const String& certPath,
        const String& keyPath,
        SSLCertificateVerifyFunction* verifyCert,
        const String& randomFile);


    //PEP187
    /** Constructor for a SSLContext object. This constructor is intended
        to be used by the CIMServer or CIMClient.
        @param trustStore file path of the trust store.
        @param certPath  file path of the server certificate.
        @param keyPath  file path of the private key.
        @param crlPath file path of the certificate revocation list.
        @param verifyCert  function pointer to a certificate verification
        call back function.  A null pointer indicates that no callback is
        requested for certificate verification.
        @param randomFile  file path of a random file that is used as a seed
        for random number generation by OpenSSL.

        @exception SSLException indicates failure to create an SSL context.
    */
    SSLContext(
        const String& trustStore,
        const String& certPath,
        const String& keyPath,
        const String& crlPath,
        SSLCertificateVerifyFunction* verifyCert,
        const String& randomFile);

#ifdef PEGASUS_USE_DEPRECATED_INTERFACES
    /** Constructor for a SSLContextRep object.
        @param trustStore  trust store file path
        @param certPath  server certificate file path
        @param keyPath  server key file path
        @param verifyCert  function pointer to a certificate verification
        call back function.
        @param trustStoreUserName In OpenPegasus 2.5 this parameter specified the user to
        associate the truststore with; this was basically a workaround to
        providers that required a username. With the support provided in PEP 187,
        this parameter is ignored beginning in release 2.5.
        @param randomFile  file path of a random file that is used as a seed
        for random number generation by OpenSSL.

        @exception SSLException  exception indicating failure to create a context.
    */
    SSLContext(
        const String& trustStore,
        const String& certPath,
        const String& keyPath,
        SSLCertificateVerifyFunction* verifyCert,
        String trustStoreUserName,
        const String& randomFile);
#endif

private:

    SSLContext();
    SSLContext& operator=(const SSLContext& sslContext);

    SSLContextRep* _rep;

    friend class SSLSocket;

    friend class CIMServer;

    friend class CIMxmlIndicationHandler;

    friend class SSLContextManager;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SSLContext_h */
