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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_CertificateProvider_h
#define Pegasus_CertificateProvider_h

///////////////////////////////////////////////////////////////////////////////
//  Certificate Provider
///////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>

#include <cctype>
#include <iostream>

#include <Pegasus/ControlProviders/CertificateProvider/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/CIMStatusCode.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/OperationContext.h>

#include <Pegasus/Repository/CIMRepository.h>

#include <Pegasus/Provider/CIMInstanceProvider.h>
#include <Pegasus/Provider/CIMMethodProvider.h>

#include <Pegasus/General/SSLContextManager.h>

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

/** This is the control provider outlined in PEP#187,
 * SSL Certificate Mgmt Enhancements.
 * It models certificate mgmt operations as standard CIM operations.
 * Client certificates are modeled by the PG_SSLCertificate class.
 * Certificate revocation lists (CRLs) are modeled using the
 * PG_SSLCertificateRevocationClass.  These classes are defined in
 * the pg_internal schema.
 * The ssltrustmgr CLI in Clients/ssltrustmgr can be used to invoke
 * these provider methods.
*/

class PEGASUS_CERTIFICATEPROVIDER_LINKAGE CertificateProvider
         : public CIMInstanceProvider, public CIMMethodProvider
{
public:

    enum TruststoreType
    {
        OTHER_TRUSTSTORE, UNKNOWN_TRUSTSTORE, SERVER_TRUSTSTORE,
            EXPORT_TRUSTSTORE, CLIENT_TRUSTSTORE
    };

    CertificateProvider( CIMRepository* repository,
                         SSLContextManager* sslContextMgr);
    virtual ~CertificateProvider(void);

    // CIMProvider interface
    // Note:  The initialize() and terminate() methods are not called for
    // Control Providers.
    virtual void initialize(CIMOMHandle & cimom) { }
    virtual void terminate() { }

    // CIMInstanceProvider interface
    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler);

    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        ObjectPathResponseHandler & handler);

    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        ObjectPathResponseHandler & handler);

    virtual void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler);

    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        ResponseHandler & handler);

    // CIMMethodProvider interface
    virtual void invokeMethod(
        const OperationContext & context,
        const CIMObjectPath & cimObjectPath,
        const CIMName & methodName,
        const Array<CIMParamValue> & inParams,
        MethodResultResponseHandler & handler);

private:

    CIMRepository* _repository;
    SSLContextManager* _sslContextMgr;

    Boolean _enableAuthentication;
    Boolean _sslClientVerificationNotDisabled;
    String _sslTrustStore;
    String _crlStore;

    String _getNewCertificateFileName(String trustStore,
                                      unsigned long hashVal);

    String _getCRLFileName(String crlStore, unsigned long hashVal);

    Boolean _verifyAuthorization(const String& userName);

    void _removeCert(Array<CIMInstance> cimInstances);

};

PEGASUS_NAMESPACE_END

#endif // Pegasus_CertificateProvider_h

