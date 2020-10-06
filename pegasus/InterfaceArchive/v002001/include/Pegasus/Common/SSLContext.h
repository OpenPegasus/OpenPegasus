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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/Linkage.h>


PEGASUS_NAMESPACE_BEGIN

class SSLCertificateInfoRep;
class SSLContextRep;
class SSLSocket;


/** This class provides the interface that a client gets as argument
    to certificate verification call back function.
*/
class PEGASUS_COMMON_LINKAGE SSLCertificateInfo
{
public:
    /** Constructor for a SSLCertificateInfo object.
    @param subjectName subject name of the certificate.
    @param issuerName  issuer name of the certificate.
    @param errorDepth  depth of the certificate chain.
    @param errorCode   error code from the default verification of the
    certificates by the Open SSL library.
    */
    SSLCertificateInfo(
        const String subjectName,
        const String issuerName,
        const int errorDepth,
        const int errorCode);

    /** Copy constructor for a SSLCertificateInfo object.
    @param certificateInfo SSLCertificateInfo object to copy.
    */
    SSLCertificateInfo(const SSLCertificateInfo& certificateInfo);

    /// Destructor.
    ~SSLCertificateInfo();

    /** Gets the subject name of the certificate.
    @return a string containing the subject name.
    */
    String getSubjectName() const;

    /** Gets the issuer name of the certificate.
    @return a string containing the issuer name.
    */
    String getIssuerName() const;

    /** Gets the depth of the certificate chain.
    @return an int containing the depth of the certificate chain.
    */
    int getErrorDepth() const;

    /** Gets the preverify error code.
    @return an int containing the preverification error code. 
    */
    int getErrorCode() const;

    /** Sets the response code.
    @param respCode response code to be set.
    */
    void setResponseCode(const int respCode);

private:

    SSLCertificateInfo();

    SSLCertificateInfoRep* _rep;
};


typedef Boolean (SSLCertificateVerifyFunction) (SSLCertificateInfo &certInfo);

/** This class provides the interface that a client can use to create
    SSL context.

    For the OSs that don't have /dev/random device file,
    must enable PEGASUS_SSL_RANDOMFILE flag and pass
    random file name to constructor.
*/
class PEGASUS_COMMON_LINKAGE SSLContext
{
public:

    /** Constructor for a SSLContext object.
    @param certPath  certificate file path.
    @param verifyCert  function pointer to a certificate verification
    call back function.  A null pointer indicates that no callback is
    requested for certificate verification.
    @param randomFile  file path of a random file that is used as a seed 
    for random number generation by OpenSSL.

    @exception SSLException indicates failure to create an SSL context.
    */
    SSLContext(
        const String& certPath,
        SSLCertificateVerifyFunction* verifyCert,
        const String& randomFile = String::EMPTY);

    ///
    SSLContext(const SSLContext& sslContext);

    /// Destructor.
    ~SSLContext();

private:

    SSLContext();

    SSLContextRep* _rep;

    friend class SSLSocket;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_SSLContext_h */
