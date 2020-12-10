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

#include <Pegasus/Common/CIMNameCast.h>
#include "CertificateProvider.h"

#define OPENSSL_NO_KRB5 1
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/PegasusVersion.h>

#include <cctype>
#include <iostream>

#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Logger.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/XmlParser.h>

#include <stdlib.h>

#include <Pegasus/Common/Executor.h>

#ifdef PEGASUS_OS_PASE
# include <ILEWrapper/ILEUtilities.h>
#endif

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// PG_SSLCertificate property names
static const CIMName ISSUER_NAME_PROPERTY      =
    CIMNameCast("IssuerName");
static const CIMName SERIAL_NUMBER_PROPERTY    =
    CIMNameCast("SerialNumber");
static const CIMName SUBJECT_NAME_PROPERTY     =
    CIMNameCast("SubjectName");
static const CIMName USER_NAME_PROPERTY        =
    CIMNameCast("RegisteredUserName");
static const CIMName TRUSTSTORE_TYPE_PROPERTY  =
    CIMNameCast("TruststoreType");
static const CIMName FILE_NAME_PROPERTY        =
    CIMNameCast("TruststorePath");
static const CIMName NOT_BEFORE_PROPERTY       =
    CIMNameCast("NotBefore");
static const CIMName NOT_AFTER_PROPERTY        =
    CIMNameCast("NotAfter");
static const CIMName CERTIFICATE_TYPE_PROPERTY =
    CIMNameCast("CertificateType");

// PG_SSLCertificateRevocationList property names also has IssuerName
static const CIMName LAST_UPDATE_PROPERTY            =
    CIMNameCast("LastUpdate");
static const CIMName NEXT_UPDATE_PROPERTY            =
    CIMNameCast("NextUpdate");
static const CIMName REVOKED_SERIAL_NUMBERS_PROPERTY =
    CIMNameCast("RevokedSerialNumbers");
static const CIMName REVOCATION_DATES_PROPERTY       =
    CIMNameCast("RevocationDates");

// Method names for PG_SSLCertificate
static const CIMName METHOD_ADD_CERTIFICATE  =
    CIMNameCast("addCertificate");
static const CIMName PARAMETER_CERT_CONTENTS =
    CIMNameCast("certificateContents");
static const CIMName PARAMETER_USERNAME      =
    CIMNameCast("userName");
static const CIMName PARAMETER_TYPE          =
    CIMNameCast("certificateType");

static const String TYPE_AUTHORITY                  = "a";
static const String TYPE_AUTHORITY_END_ENTITY       = "e";
static const String TYPE_SELF_SIGNED_IDENTITY       = "s";

static const Uint16 CERT_TYPE_UNKNOWN                    = 0;

// Method names for PG_SSLCertificateRevocationList
static const CIMName METHOD_ADD_CRL =
    CIMNameCast("addCertificateRevocationList");
static const CIMName PARAMETER_CRL_CONTENTS = CIMNameCast("CRLContents");

// Truststore and crlstore directory mutexes
static Mutex _trustStoreMutex;
static Mutex _crlStoreMutex;

struct FreeX509Ptr
{
    void operator()(X509* ptr)
    {
        X509_free(ptr);
    }
};

struct FreeX509CRLPtr
{
    void operator()(X509_CRL* ptr)
    {
        X509_CRL_free(ptr);
    }
};

struct FreeX509NAMEPtr
{
    void operator()(X509_NAME* ptr)
    {
        X509_NAME_free(ptr);
    }
};

struct FreeBIOPtr
{
    void operator()(BIO* ptr)
    {
        BIO_free_all(ptr);
    }
};

/** Convert ASN1_UTCTIME to CIMDateTime
 */
inline CIMDateTime getDateTime(const ASN1_UTCTIME* utcTime)
{
    struct tm time;
    int offset;
    char plusOrMinus = '+';
    unsigned char* utcTimeData = utcTime->data;

    memset(&time, '\0', sizeof(time));

#define g2(p) ( ( (p)[0] - '0' ) * 10 + (p)[1] - '0' )

    if (utcTime->type == V_ASN1_GENERALIZEDTIME)
    {
        time.tm_year = g2(utcTimeData) * 100;
        utcTimeData += 2;  // Remaining data is equivalent to ASN1_UTCTIME type
        time.tm_year += g2(utcTimeData);
    }
    else
    {
        time.tm_year = g2(utcTimeData);
        if (time.tm_year < 50)
        {
            time.tm_year += 2000;
        }
        else
        {
            time.tm_year += 1900;
        }
    }

    time.tm_mon = g2(utcTimeData + 2) - 1;
    time.tm_mday = g2(utcTimeData + 4);
    time.tm_hour = g2(utcTimeData + 6);
    time.tm_min = g2(utcTimeData + 8);
    time.tm_sec = g2(utcTimeData + 10);

    if (utcTimeData[12] == 'Z')
    {
        offset = 0;
    }
    else
    {
        offset = g2(utcTimeData + 13) * 60 + g2(utcTimeData + 15);
        if (utcTimeData[12] == '-')
        {
            plusOrMinus = '-';
        }
    }
#undef g2

    if (plusOrMinus == '-')
    {
        offset = -offset;
    }

    CIMDateTime dateTime = CIMDateTime(
            time.tm_year,
            time.tm_mon + 1,
            time.tm_mday,
            time.tm_hour,
            time.tm_min,
            time.tm_sec,
            0,
            6,
            offset);

    return (dateTime);
}

/**
    The issuer name should be in the format /type0=value0/type1=value1/type2=...
    where characters may be escaped by \
 */
inline X509_NAME* getIssuerName(char* issuer, long chtype)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "CertificateProvider::getIssuerName");

    // Allocate buffers for type-value pairs
    size_t buflen = strlen(issuer)+1;
    size_t maxPairs = buflen / 2 + 1;
    AutoArrayPtr<char> buf;
    AutoArrayPtr<char*> types;
    AutoArrayPtr<char*> values;

    buf.reset(new char[buflen]);
    types.reset(new char*[maxPairs]);
    values.reset(new char*[maxPairs]);

    char* sp = issuer;
    char* bp = buf.get();
    int count = 0;

    while (*sp)
    {
        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
            "CertificateProvider::getIssuerName WHILE");

        if (*sp != '/')
        {
            break;
        }
        sp++;

        types.get()[count] = bp;
        while (*sp)
        {
            if (*sp == '\\')
            {
                if (*++sp)
                {
                    *bp++ = *sp++;
                }
            }
            else if (*sp == '=')
            {
                sp++;
                *bp++ = '\0';
                break;
            }
            else
            {
                *bp++ = *sp++;
            }
        }

        values.get()[count] = bp;
        while (*sp)
        {
            if (*sp == '\\')
            {
                if (*++sp)
                {
                    *bp++ = *sp++;
                }
            }
            else if (*sp == '/')
            {
                break;
            }
            else
            {
                *bp++ = *sp++;
            }
        }
        *bp++ = '\0';
        count++;
    }


    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
    "CertificateProvider::getIssuerName  WHILE EXIT");

    // Create the issuername object and add each type/value pair
    AutoPtr<X509_NAME, FreeX509NAMEPtr> issuerNameNew(X509_NAME_new());
    if (issuerNameNew.get() == NULL)
    {
        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
            "Could not get the CRL issuer name.");
        throw PEGASUS_STD(bad_alloc)();
    }

    int nid;
    for (int i = 0; i < count; i++)
    {
        nid = OBJ_txt2nid(types.get()[i]);

        // If we don't recognize the name element or there is no
        // corresponding value, continue to the next one
        if (nid == NID_undef || !*values.get()[i])
        {
            continue;
        }

        if (!X509_NAME_add_entry_by_NID(
                issuerNameNew.get(), nid, chtype,
                (unsigned char*)values.get()[i], -1, -1, 0))
        {
            issuerNameNew.reset();
            break;
        }
    }

    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "Got issuerName successfully");
    PEG_METHOD_EXIT();

    return issuerNameNew.release();
}

/** Determines whether the user has sufficient access to perform a
    certificate operation.
  */
Boolean CertificateProvider::_verifyAuthorization(const String& userName)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
    "CertificateProvider::_verifyAuthorization");

    if (_enableAuthentication)
    {
#ifdef PEGASUS_OS_PASE
        if (!umeVerifyUserSpecialAuthorities("*SECADM   *ALLOBJ   ",
                    2, userName.getCString(), true))
#else
        if (!System::isPrivilegedUser(userName))
#endif
        {
            PEG_METHOD_EXIT();
            return false;
        }
    }

    PEG_METHOD_EXIT();
    return true;
}

/** Constructor
 */
CertificateProvider::CertificateProvider(CIMRepository* repository,
                                         SSLContextManager* sslContextMgr) :
_repository(repository),
_sslContextMgr(sslContextMgr)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
    "CertificateProvider::CertificateProvider");

    ConfigManager* configManager = ConfigManager::getInstance();

    // Get config properties
    _enableAuthentication = ConfigManager::parseBooleanValue(
        configManager->getCurrentValue("enableAuthentication"));

    _sslTrustStore =
        ConfigManager::getHomedPath(configManager->getCurrentValue(
        "sslTrustStore"));

    _sslClientVerificationNotDisabled = (configManager->getCurrentValue(
        "sslClientVerificationMode") != "disabled");

#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    _crlStore =
        ConfigManager::getHomedPath(configManager->getCurrentValue("crlStore"));
#else
    _crlStore = String::EMPTY;
#endif

    PEG_METHOD_EXIT();
}

/** Destructor
 */
CertificateProvider::~CertificateProvider(void)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "CertificateProvider::~CertificateProvider");
    PEG_METHOD_EXIT();
}

/** Delivers a single instance to the CIMOM
 */
void CertificateProvider::getInstance(
                              const OperationContext & context,
                              const CIMObjectPath & cimObjectPath,
                              const Boolean includeQualifiers,
                              const Boolean includeClassOrigin,
                              const CIMPropertyList & propertyList,
                              InstanceResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER, "CertificateProvider::getInstance");

    // Verify authorization
    const IdentityContainer container = context.get(IdentityContainer::NAME);
    if (!_verifyAuthorization(container.getUserName()))
    {
        MessageLoaderParms parms(
            "ControlProviders.CertificateProvider.MUST_BE_PRIVILEGED_USER",
            "Superuser authority is required to run this CIM operation.");
        throw CIMException(CIM_ERR_ACCESS_DENIED, parms);
    }

    CIMName className(cimObjectPath.getClassName());

    // Verify classname
    if (className == PEGASUS_CLASSNAME_CERTIFICATE)
    {
        // process request
        handler.processing();

        // Verify the keys are set
        // ATTN: do we need to do this, or will the getInstance call handle it?
        Array<CIMKeyBinding> keyBindings = cimObjectPath.getKeyBindings();
        String keyName;

        for (Uint32 i=0; i < keyBindings.size(); i++)
        {
            keyName = keyBindings[i].getName().getString();

            if (!String::equal(keyName, ISSUER_NAME_PROPERTY.getString())
                && !String::equal(keyName, SERIAL_NUMBER_PROPERTY.getString()))
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER, keyName);
            }
        }

        CIMInstance cimInstance =
            _repository->getInstance(cimObjectPath.getNameSpace(),
                    cimObjectPath);

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
            "Returning certificate COP %s",
            (const char*)cimInstance.getPath().toString().getCString()));

        // Deliver instance
        handler.deliver(cimInstance);

        // Complete request
        handler.complete();

    }
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    else if (className == PEGASUS_CLASSNAME_CRL)
    {
        // ATTN: Fill in
    }
#endif
    else
    {
        throw CIMException(CIM_ERR_INVALID_CLASS, className.getString());
    }

    PEG_METHOD_EXIT();
}

/** Builds and returns a PG_SSLCertificateRevocationList from an X509_CRL object
 */
inline CIMInstance _getCRLInstance(X509_CRL* xCrl, String host,
        CIMNamespaceName nameSpace)
{
    char issuerName[1024];
    STACK_OF(X509_REVOKED) *revoked = NULL;
    X509_REVOKED* r = NULL;
    int numRevoked = -1;
    long rawSerialNumber;
    char serial[1024];
    CIMDateTime revocationDate;

    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "CertificateProvider::_getCRLInstance");

    // Build instance
    CIMInstance cimInstance(PEGASUS_CLASSNAME_CRL);

    // CA issuer name
    X509_NAME_oneline(
        X509_CRL_get_issuer(xCrl), issuerName, sizeof(issuerName));
    issuerName[sizeof(issuerName) - 1] = 0;
    cimInstance.addProperty(
        CIMProperty(ISSUER_NAME_PROPERTY, CIMValue(String(issuerName))));

    // Validity dates
    CIMDateTime lastUpdate = getDateTime(X509_CRL_get_lastUpdate(xCrl));
    cimInstance.addProperty(CIMProperty(LAST_UPDATE_PROPERTY,
                CIMValue(lastUpdate)));

    CIMDateTime nextUpdate = getDateTime(X509_CRL_get_nextUpdate(xCrl));
    cimInstance.addProperty(CIMProperty(NEXT_UPDATE_PROPERTY,
                CIMValue(nextUpdate)));

    Array<String> revokedSerialNumbers;
    Array<CIMDateTime> revocationDates;

    // Get revoked certificate information
    revoked = X509_CRL_get_REVOKED(xCrl);
    numRevoked = sk_X509_REVOKED_num(revoked);

    for (int i = 0; i < numRevoked; i++)
    {
        r = sk_X509_REVOKED_value(revoked, i);
        rawSerialNumber = ASN1_INTEGER_get(r->serialNumber);
        sprintf(serial, "%lu", (unsigned long)rawSerialNumber);
        revokedSerialNumbers.append(String(serial));

        revocationDate = getDateTime(r->revocationDate);
        revocationDates.append(revocationDate);
    }

    cimInstance.addProperty(CIMProperty(REVOKED_SERIAL_NUMBERS_PROPERTY,
                CIMValue(revokedSerialNumbers)));
    cimInstance.addProperty(CIMProperty(REVOCATION_DATES_PROPERTY,
                CIMValue(revocationDates)));

     // Set keys
    Array<CIMKeyBinding> keys;
    CIMKeyBinding key;
    key.setName(ISSUER_NAME_PROPERTY.getString());
    key.setValue(issuerName);
    key.setType(CIMKeyBinding::STRING);
    keys.append(key);

    // Set object path for instance
    cimInstance.setPath(CIMObjectPath(host, nameSpace,
                PEGASUS_CLASSNAME_CRL, keys));

    PEG_METHOD_EXIT();

    return (cimInstance);
}

/** Delivers the complete collection of instances to the CIMOM
 */
void CertificateProvider::enumerateInstances(
    const OperationContext& context,
    const CIMObjectPath& cimObjectPath,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList& propertyList,
    InstanceResponseHandler& handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "CertificateProvider::enumerateInstances");

    // Verify authorization
    const IdentityContainer container = context.get(IdentityContainer::NAME);
    if (!_verifyAuthorization(container.getUserName()))
    {
        MessageLoaderParms parms(
            "ControlProviders.CertificateProvider.MUST_BE_PRIVILEGED_USER",
            "Superuser authority is required to run this CIM operation.");
        throw CIMException(CIM_ERR_ACCESS_DENIED, parms);
    }

    CIMName className(cimObjectPath.getClassName());

    // Verify classname
    if (className == PEGASUS_CLASSNAME_CERTIFICATE)
    {
        // Process request
        handler.processing();

        // Get instances from the repository
        Array<CIMInstance> cimInstances;
        cimInstances = _repository->enumerateInstancesForClass(
            cimObjectPath.getNameSpace(), PEGASUS_CLASSNAME_CERTIFICATE);

        for (Uint32 i = 0, n = cimInstances.size(); i < n; i++)
        {
            Uint16 truststoreType = 0;

            //
            // The truststore type key property is deprecated. To retain
            // backward compatibility, if there were instances of an earlier
            // version in the repository that specify a truststore type
            // other than cimserver, those instances will be ignored.
            // Also, if there are instances that do not specify a certificate
            // type, the type for such instances is set to unknown (1).
            // And if there are instances that do not have certificate type
            // property, CERTIFICATE_TYPE_PROPERTY is added and is set to
            // unknown. Last case is introduced to retain backward
            // compatibility, if there were instances of an earlier version
            // in the repository that do not have certificate type property.
            //

            //
            // Retrieve the truststore type
            //
            Uint32 tsTypeIndex = cimInstances[i].findProperty(
                            TRUSTSTORE_TYPE_PROPERTY);
            CIMProperty tsTypeProp = cimInstances[i].getProperty(tsTypeIndex);
            tsTypeProp.getValue().get(truststoreType);

            //
            // Filter instances whose truststore type is other than
            // server truststore.
            //

            if ( truststoreType == PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER )
            {
                //
                // If the certificate type property does not have a value set,
                // set its type to "Unknown"
                //

                Uint32 certTypeIndex = cimInstances[i].findProperty(
                               CERTIFICATE_TYPE_PROPERTY);

                //
                // If certificate type property is not there then add the
                // property and set its type to "Unknown"
                //
                if (certTypeIndex != PEG_NOT_FOUND)
                {
                    CIMProperty certTypeProp =
                        cimInstances[i].getProperty(certTypeIndex);

                    if (certTypeProp.getValue().isNull())
                    {
                        PEG_TRACE_CSTRING(
                            TRC_CONTROLPROVIDER,
                            Tracer::LEVEL4,
                            "The instance does not have the certificate"
                                " type set. Setting it to Unknown.");
                        certTypeProp.setValue(CERT_TYPE_UNKNOWN);
                    }
                }
                else
                {
                    PEG_TRACE_CSTRING(
                        TRC_CONTROLPROVIDER,
                        Tracer::LEVEL4,
                        "The instance does not have the certificate "
                            "type property. Adding it and setting to Unknown.");
                    cimInstances[i].addProperty(
                        CIMProperty(CERTIFICATE_TYPE_PROPERTY,
                        CIMValue(CERT_TYPE_UNKNOWN)));
                }

                // Deliver instance
                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                    "Delivering CIMInstance %s",
                    (const char*)
                        cimInstances[i].getPath().toString().getCString()));
                handler.deliver(cimInstances[i]);
            }
            else
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL3,
                    "Ignoring CIMInstance %s",
                    (const char*)
                        cimInstances[i].getPath().toString().getCString()));
            }
        }

        // Complete request
        handler.complete();
    }
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    else if (className == PEGASUS_CLASSNAME_CRL)
    {
        // Process request
        handler.processing();

        FileSystem::translateSlashes(_crlStore);

#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
        if (FileSystem::isDirectory(_crlStore))
#else
        if (FileSystem::isDirectory(_crlStore) &&
            FileSystem::canWrite(_crlStore))
#endif
        {
            Array<String> crlFiles;
            if (FileSystem::getDirectoryContents(_crlStore, crlFiles))
            {
                Uint32 count = crlFiles.size();
                for (Uint32 i = 0; i < count; i++)
                {
                    String filename = crlFiles[i];

                    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                        "Filename %s",(const char*)filename.getCString()));

                    // ATTN: Is this a two-way hash?  If so, I don't need to
                    // read in the CRL just to determine the issuer name
                    AutoPtr<BIO, FreeBIOPtr> inFile(BIO_new(BIO_s_file()));
                    if (inFile.get() == NULL)
                    {
                        // error
                        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                            "Error reading CRL file");
                        throw PEGASUS_STD(bad_alloc)();
                    }

                    char fullPathName[1024];
                    sprintf(fullPathName, "%s/%s",
                            (const char*)_crlStore.getCString(),
                            (const char*)filename.getCString());

                    if (BIO_read_filename(inFile.get(), fullPathName))
                    {
                        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                                         "Successfully read filename");

                        AutoPtr<X509_CRL, FreeX509CRLPtr> xCrl(
                            PEM_read_bio_X509_CRL(inFile.get(),
                                NULL, NULL, NULL));

                        if (xCrl.get())
                        {
                            // Build instance
                            CIMInstance cimInstance = _getCRLInstance(
                                xCrl.get(),
                                cimObjectPath.getHost(),
                                cimObjectPath.getNameSpace());

                            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                                "Delivering CIMInstance: %s",
                                (const char*)cimInstance.getPath().
                                                 toString().getCString()));

                            // Deliver instance
                            handler.deliver(cimInstance);
                        }
                    }
                    else
                    {
                        // error
                        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                            "Error reading CRL file");
                        MessageLoaderParms parms(
                            "ControlProviders.CertificateProvider."
                                "COULD_NOT_READ_CRL",
                            "Failed to read CRL $0.", fullPathName);
                        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);
                    }
                } // end for

                // Complete request
                handler.complete();
            }
            else
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: Could not read sslCRLStore directory.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider."
                        "COULD_NOT_READ_DIRECTORY",
                    "Cannot read directory $0.", _crlStore);
                throw CIMException(CIM_ERR_FAILED, parms);
            }
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                             "Error: sslCRLStore is not a valid directory.");
            MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider."
                        "INVALID_DIRECTORY",
                    "Invalid directory $0.", _crlStore);
            throw CIMException(CIM_ERR_FAILED, parms);
        }
    }
#endif
    else
    {
        throw CIMException(CIM_ERR_INVALID_CLASS, className.getString());
    }

    PEG_METHOD_EXIT();
}

/** Delivers the complete collection of instance names (CIMObjectPaths) to
    the CIMOM
 */
void CertificateProvider::enumerateInstanceNames(
    const OperationContext& context,
    const CIMObjectPath& cimObjectPath,
    ObjectPathResponseHandler& handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "CertificateProvider::enumerateInstanceNames");

    // Verify authorization
    const IdentityContainer container = context.get(IdentityContainer::NAME);
    if (!_verifyAuthorization(container.getUserName()))
    {
        MessageLoaderParms parms(
            "ControlProviders.CertificateProvider.MUST_BE_PRIVILEGED_USER",
            "Superuser authority is required to run this CIM operation.");
        throw CIMException(CIM_ERR_ACCESS_DENIED, parms);
    }

    CIMName className(cimObjectPath.getClassName());

    // Verify classname
    if (className == PEGASUS_CLASSNAME_CERTIFICATE)
    {
        // Process request
        handler.processing();

        Array<CIMObjectPath> instanceNames =
            _repository->enumerateInstanceNamesForClass(
                cimObjectPath.getNameSpace(), PEGASUS_CLASSNAME_CERTIFICATE);

        for (Uint32 i = 0, n = instanceNames.size(); i < n; i++)
        {
            String truststoreType;
            Array<CIMKeyBinding> kb;

            //
            // The truststore type key property is deprecated. To retain
            // backward compatibility, if there were instances of an earlier
            // version in the repository that specify a truststore type
            // other than cimserver, those instances will be ignored.
            //
            kb = instanceNames[i].getKeyBindings();
            Uint32 count = kb.size();

            for (Uint32 j = 0; j < count; j++)
            {
                //
                // Retrieve the truststore type
                //
                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                    "Property name : %s",
                    (const char*)kb[j].getName().getString().getCString()));
                if ( kb[j].getName() == TRUSTSTORE_TYPE_PROPERTY )
                {
                    truststoreType = kb[j].getValue();
                    break;
                }
            }

            //
            // Filter instances whose truststore type is other than
            // server truststore.
            //

            if ( truststoreType == PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER)
            {
                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                    "Delivering CIMObjectPath: %s",
                    (const char*)instanceNames[i].toString().getCString()));

                // Deliver object path
                handler.deliver(instanceNames[i]);
            }
            else
            {
                PEG_TRACE((TRC_DISCARDED_DATA, Tracer::LEVEL3,
                    "Ignoring CIMObjectPath: %s",
                    (const char*)instanceNames[i].toString().getCString()));
            }
        }

        // Complete request
        handler.complete();

    }
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    else if (className == PEGASUS_CLASSNAME_CRL)
    {
         // Process request
        handler.processing();

        FileSystem::translateSlashes(_crlStore);

#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
        if (FileSystem::isDirectory(_crlStore))
#else
        if (FileSystem::isDirectory(_crlStore) &&
            FileSystem::canWrite(_crlStore))
#endif
        {
            Array<String> crlFiles;
            if (FileSystem::getDirectoryContents(_crlStore, crlFiles))
            {
                Uint32 count = crlFiles.size();
                for (Uint32 i = 0; i < count; i++)
                {
                    String filename = crlFiles[i];

                    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL3,
                        "Filename %s",(const char*)filename.getCString()));

                    // ATTN: Is this a two-way hash?  If so, I don't need
                    // to read in the CRL just to determine the issuer name
                    AutoPtr<BIO, FreeBIOPtr> inFile(BIO_new(BIO_s_file()));
                    if (inFile.get() == NULL)
                    {
                        // error
                        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                            "Error reading CRL file");
                        throw PEGASUS_STD(bad_alloc)();
                    }

                    char issuerName[1024];
                    char fullPathName[1024];
                    sprintf(fullPathName, "%s/%s",
                            (const char*)_crlStore.getCString(),
                            (const char*)filename.getCString());

                    if (BIO_read_filename(inFile.get(), fullPathName))
                    {
                        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL3,
                            "Successfully read filename");

                        AutoPtr<X509_CRL, FreeX509CRLPtr> xCrl(
                            PEM_read_bio_X509_CRL(inFile.get(),
                                NULL, NULL, NULL));
                        if (xCrl.get())
                        {
                            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,
                                Tracer::LEVEL3,
                                "Successfully read CRL file");
                            sprintf(issuerName, "%s",
                                X509_NAME_oneline(
                                    X509_CRL_get_issuer(xCrl.get()),
                                NULL, 0));

                            // Build object path
                            Array<CIMKeyBinding> keys;
                            CIMKeyBinding key;
                            key.setName(ISSUER_NAME_PROPERTY.getString());
                            key.setValue(issuerName);
                            key.setType(CIMKeyBinding::STRING);
                            keys.append(key);

                            // Set object path for instance
                            CIMObjectPath instanceName(cimObjectPath.getHost(),
                                cimObjectPath.getNameSpace(),
                                PEGASUS_CLASSNAME_CRL, keys);

                            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                                "Instance Name: %s",
                                (const char*)instanceName.toString()
                                                 .getCString()));

                            handler.deliver(instanceName);
                        }
                    }
                    else
                    {
                        // error
                        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                            "Error reading CRL file");
                        MessageLoaderParms parms(
                            "ControlProviders.CertificateProvider."
                                "COULD_NOT_READ_CRL",
                            "Failed to read CRL $0.", fullPathName);
                        throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED, parms);
                    }
                } // end for

                // Complete request
                handler.complete();
            }
            else
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: Could not read sslCRLStore directory.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider."
                        "COULD_NOT_READ_DIRECTORY",
                    "Cannot read directory $0.", _crlStore);
                throw CIMException(CIM_ERR_FAILED, parms);
            }
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                             "Error: sslCRLStore is not a valid directory.");
            MessageLoaderParms parms(
                "ControlProviders.CertificateProvider.INVALID_DIRECTORY",
                "Invalid directory $0.", _crlStore);
            throw CIMException(CIM_ERR_FAILED, parms);
        }
    }
#endif
    else
    {
        throw CIMException(CIM_ERR_INVALID_CLASS, className.getString());
    }

    PEG_METHOD_EXIT();
}

/** Not supported.  Use invokeMethod to create a certificate or CRL
*/
void CertificateProvider::createInstance(
    const OperationContext& context,
    const CIMObjectPath& cimObjectPath,
    const CIMInstance& cimInstance,
    ObjectPathResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED,
        "CertificateProvider::createInstance");
}

/** Not supported.
  */
void CertificateProvider::modifyInstance(
    const OperationContext& context,
    const CIMObjectPath& cimObjectPath,
    const CIMInstance& cimInstance,
    const Boolean includeQualifiers,
    const CIMPropertyList& propertyList,
    ResponseHandler& handler)
{
    throw CIMException(CIM_ERR_NOT_SUPPORTED,
        "CertificateProvider::modifyInstance");
}

/** Deletes the internal object denoted by the specified CIMObjectPath
 */
void CertificateProvider::deleteInstance(
    const OperationContext& context,
    const CIMObjectPath& cimObjectPath,
    ResponseHandler& handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "CertificateProvider::deleteInstance");

    // Verify authorization
    const IdentityContainer container = context.get(IdentityContainer::NAME);
    if (!_verifyAuthorization(container.getUserName()))
    {
        MessageLoaderParms parms(
           "ControlProviders.CertificateProvider.MUST_BE_PRIVILEGED_USER",
           "Superuser authority is required to run this CIM operation.");
        throw CIMException(CIM_ERR_ACCESS_DENIED, parms);
    }

    CIMName className(cimObjectPath.getClassName());

    // Verify classname
    if (className == PEGASUS_CLASSNAME_CERTIFICATE)
    {
        // Process request
        handler.processing();

        //
        // Check if the subjectName is passed.
        //
        Array<CIMInstance> cimInstances;
        Array<CIMKeyBinding> keys;
        CIMKeyBinding key;
        String certIssuer;
        String certSubject;
        String certSerialNum;
        Boolean subjectSet = true;
        Boolean issuerSet = true;
        Boolean serialNumSet = true;

        keys = cimObjectPath.getKeyBindings();

        if (keys.size() && String::equal(keys[0].getName().getString(),
            ISSUER_NAME_PROPERTY.getString()))
        {
            certIssuer = keys[0].getValue();
        }
        else
        {
            issuerSet = false;
        }

        if (keys.size() && String::equal(keys[1].getName().getString(),
            SUBJECT_NAME_PROPERTY.getString()))
        {
            certSubject = keys[1].getValue();
        }
        else
        {
            subjectSet = false;
        }

        if (keys.size() && String::equal(keys[1].getName().getString(),
            SERIAL_NUMBER_PROPERTY.getString()))
        {
            certSerialNum = keys[1].getValue();
        }
        else
        {
            serialNumSet = false;
        }

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
            "issuerName: %s",(const char*)certIssuer.getCString()));

        //
        // Check if the subject and issuer were specified.
        //
        if (subjectSet && issuerSet)
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "Subject and issuer specified.");
            PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "subjectName: %s",(const char*)certSubject.getCString()));

            Array<CIMInstance> certificateNamedInstances;

            //
            // Get all the instances of class PG_SSLCertificate
            //
            certificateNamedInstances =
                _repository->enumerateInstancesForClass(
                    PEGASUS_NAMESPACENAME_CERTIFICATE,
                    PEGASUS_CLASSNAME_CERTIFICATE, false);

            //
            // Retrieve the instances for the specified subject & issuer
            //
            Uint32 num = certificateNamedInstances.size();

            for (Uint32 i = 0; i < num; i++)
            {
                String issuer;
                String subject;
                Uint16 truststoreType = 0;

                CIMInstance& certificateInstance =
                    certificateNamedInstances[i];

                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                    "Comparing instance: %s",
                    (const char*)
                        certificateInstance.getPath().toString().getCString()));

                //
                // Retrieve the truststore type
                //
                Uint32 tsTypePos = certificateInstance.findProperty(
                    TRUSTSTORE_TYPE_PROPERTY);
                CIMProperty tsTypeProp =
                    certificateInstance.getProperty(tsTypePos);
                tsTypeProp.getValue().get(truststoreType);

                //
                // Filter instances whose truststore type is
                // other than server truststore.
                //

                if ( truststoreType == PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER )
                {
                    //
                    // Check if issuer name and subject are specified
                    // and they match
                    //
                    Uint32 pos = certificateInstance.findProperty(
                                    ISSUER_NAME_PROPERTY);
                    CIMProperty prop = certificateInstance.getProperty(pos);
                    prop.getValue().get(issuer);

                    pos =
                        certificateInstance.findProperty(SUBJECT_NAME_PROPERTY);
                    prop = certificateInstance.getProperty(pos);
                    prop.getValue().get(subject);

                    if ( issuer == certIssuer && subject == certSubject)
                    {
                        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                           "Found a matching instance.");
                        cimInstances.append(certificateInstance);
                    }
                }
                else
                {
                     PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                         "Ignoring instance: %s",
                         (const char*)certificateInstance.getPath().
                                          toString().getCString()));
                }
            }

            // Check if the certificate was found
            if (cimInstances.size() == 0)
            {
                // Certificate does not exist, throw exception
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "The certificate does not exist.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.CERT_DNE",
                    "The certificate does not exist.");
                throw CIMException(CIM_ERR_NOT_FOUND, parms);
            }
        }
        else if (issuerSet && serialNumSet)
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "issuer and serial number specified.");

            PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "serial number: %s",(const char*)certSerialNum.getCString()));

            CIMObjectPath tmpPath = cimObjectPath;

            try
            {
                Array<CIMKeyBinding> newKeys = cimObjectPath.getKeyBindings();

                // Check for deprecated truststore key
                Boolean truststoreKeyFound = false;
                for (Uint32 i = 0; i < newKeys.size() ; ++i)
                {
                    if (newKeys[i].getName() == TRUSTSTORE_TYPE_PROPERTY)
                    {
                        truststoreKeyFound = true;
                        break;
                    }
                }
                //
                // The truststore type key property is deprecated. To retain
                // backward compatibility, add the truststore type property
                // to the key bindings and set it to cimserver truststore.
                //
                if (!truststoreKeyFound)
                {
                    CIMKeyBinding kb (TRUSTSTORE_TYPE_PROPERTY,
                        PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER);
                    newKeys.append(kb);
                }

                tmpPath.setKeyBindings(newKeys);

                cimInstances.append(_repository->getInstance(
                   cimObjectPath.getNameSpace(), tmpPath));

            }
            catch (Exception&)
            {
                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "The certificate does not exist: %s",
                    (const char*)tmpPath.toString().getCString()));
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.CERT_DNE",
                    "The certificate does not exist.");
                throw CIMException(CIM_ERR_NOT_FOUND, parms);
            }
        }
        else
        {
            throw CIMException(CIM_ERR_INVALID_PARAMETER,
                cimObjectPath.toString());
        }

        // Check if there were certificates to be deleted.
        if (cimInstances.size() > 0)
        {
            // Delete the certificates
            _removeCert(cimInstances);
        }

        // Complete request
        handler.complete();

    }
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    else if (className == PEGASUS_CLASSNAME_CRL)
    {
        Array<CIMKeyBinding> keys;
        CIMKeyBinding key;
        String issuerName;

        keys = cimObjectPath.getKeyBindings();
        if (keys.size() && String::equal(keys[0].getName().getString(),
                                          ISSUER_NAME_PROPERTY.getString()))
        {
            issuerName = keys[0].getValue();
        }

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"CRL COP %s",
            (const char*)cimObjectPath.toString().getCString()));
        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"Issuer Name %s",
            (const char*)issuerName.getCString()));

        // ATTN: it would nice to be able to do this by getting the hash
        // directly from the issuerName
        // unfortunately, there does not seem to be an easy way to achieve this
        // the closest I can get is to add the individual DN components using
        // X509_NAME_add_entry_by_NID
        // which involves a lot of tedious parsing.
        // look in the do_subject method of apps.h for how this is done
        // X509_NAME* name = X509_name_new();

        char issuerChar[1024];
        sprintf(issuerChar, "%s", (const char*) issuerName.getCString());

        AutoPtr<X509_NAME, FreeX509NAMEPtr> name(
            getIssuerName(issuerChar, MBSTRING_ASC));
        AutoMutex lock(_crlStoreMutex);

        String crlFileName =
            _getCRLFileName(_crlStore, X509_NAME_hash(name.get()));
        if (FileSystem::exists(crlFileName))
        {
            if (Executor::removeFile(crlFileName.getCString()) == 0)
            {
                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL3,
                    "Successfully deleted CRL file %s",
                    (const char*)crlFileName.getCString()));

                if (_sslClientVerificationNotDisabled)
                {
                    //
                    // Reload the CRL store to refresh the cache
                    //
                    _sslContextMgr->reloadCRLStore();
                }

                PEG_TRACE((
                    TRC_CONTROLPROVIDER,
                    Tracer::LEVEL3,
                    "The CRL from issuer %s has been deleted.",
                    (const char*) issuerName.getCString()));
            }
            else
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                                 "Could not delete file.");
                 MessageLoaderParms parms(
                     "ControlProviders.CertificateProvider.DELETE_FAILED",
                     "Could not delete file $0.",
                     FileSystem::extractFileName(crlFileName));
                 throw CIMException(CIM_ERR_FAILED, parms);
            }
        }
        else
        {
             PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                 "File does not exist.");
             MessageLoaderParms parms(
                 "ControlProviders.CertificateProvider.FILE_DNE",
                 "File does not exist $0.",
                 FileSystem::extractFileName(crlFileName));
             throw CIMException(CIM_ERR_NOT_FOUND, parms);
        }
    }
#endif
    else
    {
        throw CIMException(CIM_ERR_INVALID_CLASS, className.getString());
    }

    PEG_METHOD_EXIT();
}


void CertificateProvider::_removeCert (Array<CIMInstance> cimInstances)
{
    Uint32 num = cimInstances.size();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "Number of certificate instances to be removed : %d " , num));


    for ( Uint32 i = 0; i < num ; i++)
    {
        String issuerName;
        String userName;
        String certificateFileName;
        String serialNumber;
        CIMProperty cimProperty;

        CIMInstance& certificateInstance = cimInstances[i];

        // Certificate file name
        cimProperty = certificateInstance.getProperty(
            certificateInstance.findProperty(FILE_NAME_PROPERTY));
        cimProperty.getValue().get(certificateFileName);

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
            "Certificate file name %s",
            (const char*)certificateFileName.getCString()));

        // Issuer name
        cimProperty = certificateInstance.getProperty(
            certificateInstance.findProperty(ISSUER_NAME_PROPERTY));
        cimProperty.getValue().get(issuerName);

        // User name
        cimProperty = certificateInstance.getProperty(
            certificateInstance.findProperty(USER_NAME_PROPERTY));
        cimProperty.getValue().get(userName);

        // Serial number
        cimProperty = certificateInstance.getProperty(
            certificateInstance.findProperty(SERIAL_NUMBER_PROPERTY));
        cimProperty.getValue().get(serialNumber);

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"Issuer name %s",
            (const char*)issuerName.getCString()));

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"serial number %s",
            (const char*)serialNumber.getCString()));

        if (userName == String::EMPTY)
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "The certificate does not have a username associated with it");
        }
        else
        {
            PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"User name %s",
                (const char*)userName.getCString()));
        }

        AutoMutex lock(_trustStoreMutex);

        if (!FileSystem::exists(certificateFileName))
        {
             //
             // In rare cases a certificate may have been
             // manually removed from the truststore, but the repositoty
             // entry still exists. Delete the Repository instance so that
             // the certificate can be re-added again if required.
             //
             // This is also valid for end-entity certificates as they
             // would not exist in the truststore.
             //

             PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL2,
                 "WARNING: Certificate file does not exist, "
                     "remove entry from repository anyway.");
        }
        else if (Executor::removeFile(
            certificateFileName.getCString()) != 0)
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                "Could not delete file.");
            MessageLoaderParms parms(
                "ControlProviders.CertificateProvider.DELETE_FAILED",
                "Could not delete file $0.", certificateFileName);
            throw CIMException(CIM_ERR_FAILED, parms);
        }

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL3,
            "Successfully deleted certificate file %s",
            (const char*)certificateFileName.getCString()));

        Array<CIMKeyBinding> kbArray;
        CIMKeyBinding kb;

        kb.setName(ISSUER_NAME_PROPERTY);
        kb.setValue(issuerName);
        kb.setType(CIMKeyBinding::STRING);
        kbArray.append(kb);

        kb.setName(SERIAL_NUMBER_PROPERTY);
        kb.setValue(serialNumber);
        kb.setType(CIMKeyBinding::STRING);
        kbArray.append(kb);

        //
        // The truststore type key property is deprecated. To retain
        // backward compatibility, add the truststore type property
        // to the key bindings and set it to cimserver truststore.
        //
        CIMKeyBinding key (TRUSTSTORE_TYPE_PROPERTY,
            PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER);
        kbArray.append (key);

        CIMObjectPath reference(
            String::EMPTY, PEGASUS_NAMESPACENAME_CERTIFICATE,
            PEGASUS_CLASSNAME_CERTIFICATE, kbArray);

        PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,"keys are: %s",
            (const char*)reference.toString().getCString()));

        // Delete from repository.
        _repository->deleteInstance(
            PEGASUS_NAMESPACENAME_CERTIFICATE,
            reference);

        if (userName == String::EMPTY)
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL3,
                "The certificate without an associated user name from "
                    "issuer %s has been deleted from the truststore.",
                (const char*) issuerName.getCString()));
        }
        else
        {
            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL3,
                "The certificate registered to %s from issuer %s "
                    "has been deleted from the truststore.",
                (const char*) userName.getCString(),
                (const char*) issuerName.getCString()));
        }
    }

    //
    // Request SSLContextManager to delete the certificate from the cache
    //
    if (_sslClientVerificationNotDisabled)
    {
        try
        {
            _sslContextMgr->reloadTrustStore();
        }
        catch (SSLException& ex)
        {
            PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                "Trust store reload failed: %s",
                (const char*)ex.getMessage().getCString()));

            MessageLoaderParms parms(
                "ControlProviders.CertificateProvider.TRUSTSTORE_RELOAD_FAILED",
                "Trust store reload failed, certificate deletion will"
                " not be effective until cimserver restart.");
            throw CIMException(CIM_ERR_FAILED, parms);
        }
    }
}

/** Returns the CRL filename associated with the hashvalue that
    represents the issuer name.
    There is only one CRL per issuer so the file name will always end in .r0
 */
String CertificateProvider::_getCRLFileName(
    String crlStore,
    unsigned long hashVal)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "CertificateProvider::_getCRLFileName");

    // The files are looked up by the CA issuer name hash value.
    // Since only one CRL should exist for a given CA, the extension .r0
    // is appended to the CA hash
    char hashBuffer[32];
    sprintf(hashBuffer, "%08lx", hashVal);

    String hashString = "";
    for (int j = 0; j < 32; j++)
    {
        if (hashBuffer[j] != '\0')
        {
            hashString.append(hashBuffer[j]);
        }
        else
        {
            break; // end of hash string
        }
    }

    char filename[1024];
    sprintf(filename, "%s/%s.r0",
            (const char*)crlStore.getCString(),
            (const char*)hashString.getCString());

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "Searching for files like %s in %s",
        (const char*)hashString.getCString(),
        (const char*)crlStore.getCString()));

    FileSystem::translateSlashes(crlStore);
#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
    if (FileSystem::isDirectory(crlStore))
#else
    if (FileSystem::isDirectory(crlStore) && FileSystem::canWrite(crlStore))
#endif
    {
        if (FileSystem::exists(filename))
        {
            // Overwrite
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "CRL already exists, overwriting");

        }
        else
        {
            // Create
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "CRL does not exist, creating");
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
            "Cannot add CRL to CRL store : CRL directory DNE or does not"
            " have write privileges");
        MessageLoaderParms parms(
            "ControlProviders.CertificateProvider.INVALID_DIRECTORY",
            "Invalid directory $0.", crlStore);
        throw CIMException(CIM_ERR_FAILED, parms);
    }

    PEG_METHOD_EXIT();

    return String(filename);
}

/** Returns the new certificate filename for the hashvalue that
    represents the subject name.
 */
String CertificateProvider::_getNewCertificateFileName(
    String trustStore,
    unsigned long hashVal)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "CertificateProvider::_getNewCertificateFileName");

    // The files are looked up by the CA subject name hash value.
    // If more than one CA certificate with the same name hash value exists,
    // the extension must be different (e.g. 9d66eef0.0, 9d66eef0.1 etc)
    char hashBuffer[32];
    sprintf(hashBuffer, "%08lx", hashVal);

    String hashString = "";
    for (int j = 0; j < 32; j++)
    {
        if (hashBuffer[j] != '\0')
        {
            hashString.append(hashBuffer[j]);
        }
        else
        {
            break; // end of hash string
        }
    }

    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "Searching for files like %s",(const char*)hashString.getCString()));

    Uint32 index = 0;
    FileSystem::translateSlashes(trustStore);

#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)
    if (FileSystem::isDirectory(trustStore))
#else
    if (FileSystem::isDirectory(trustStore) && FileSystem::canWrite(trustStore))
#endif
    {
        Array<String> trustedCerts;
        if (FileSystem::getDirectoryContents(trustStore, trustedCerts))
        {
            for (Uint32 i = 0; i < trustedCerts.size(); i++)
            {
                //
                // Check if another certificate with the same
                // subject name already exists. If yes, error out.
                //
                if (String::compare(trustedCerts[i],
                    hashString, hashString.size()) == 0)
                {
                    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,
                        Tracer::LEVEL1,
                        "Error: Certificate with the same subject "
                            "already exists.");
                    MessageLoaderParms parms( "ControlProviders."
                        "CertificateProvider.CERT_WITH_SAME_SUBJECT",
                        "Another certificate with the "
                            "same subject name already exists.");
                    throw CIMException(CIM_ERR_ALREADY_EXISTS, parms);
                }
            }
        }
        else
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                "Error: Could not read truststore directory.");
            MessageLoaderParms parms(
                "ControlProviders.CertificateProvider."
                    "COULD_NOT_READ_DIRECTORY",
                "Cannot read directory $0.", trustStore);
            throw CIMException(CIM_ERR_FAILED, parms);
        }
    }
    else
    {
        PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                         "Error: sslCRLStore is not a valid directory.");
        MessageLoaderParms parms(
            "ControlProviders.CertificateProvider.INVALID_DIRECTORY",
            "Invalid directory $0.", trustStore);
        throw CIMException(CIM_ERR_FAILED, parms);
    }

    char filename[1024];
    sprintf(filename, "%s/%s.%u",
        (const char*)trustStore.getCString(),
        (const char*)hashString.getCString(),
        index);

    PEG_METHOD_EXIT();

    return String(filename);
}


static BIO* _openBIOForWrite(const char* path)
{
#if defined(PEGASUS_ENABLE_PRIVILEGE_SEPARATION)

    FILE* is = Executor::openFile(path, 'w');

    if (!is)
        return 0;

    BIO* bio = BIO_new_fp(is, BIO_CLOSE);

    if (!bio)
        return 0;

    return bio;

#else /* !defined(PEGASUS_PRIVILEGE_SEPARATION) */

    BIO* bio = BIO_new(BIO_s_file());

    if (!bio)
        return 0;

    if (!BIO_write_filename(bio, (char*)path))
        return 0;

    return bio;

#endif /* !defined(PEGASUS_PRIVILEGE_SEPARATION) */
}

/** Calls an extrinsic method on the class.
 */
void CertificateProvider::invokeMethod(
    const OperationContext& context,
    const CIMObjectPath& cimObjectPath,
    const CIMName& methodName,
    const Array<CIMParamValue>& inParams,
    MethodResultResponseHandler& handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,"CertificateProvider::invokeMethod");

    //verify authorization
    const IdentityContainer container = context.get(IdentityContainer::NAME);
    if (!_verifyAuthorization(container.getUserName()))
    {
        MessageLoaderParms parms(
            "ControlProviders.CertificateProvider.MUST_BE_PRIVILEGED_USER",
            "Superuser authority is required to run this CIM operation.");
        throw CIMException(CIM_ERR_ACCESS_DENIED, parms);
    }

    CIMName className(cimObjectPath.getClassName());

    // Verify classname
    if (className == PEGASUS_CLASSNAME_CERTIFICATE)
    {
        // Process request
        handler.processing();

        if (methodName == METHOD_ADD_CERTIFICATE)
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "CertificateProvider::addCertificate()");

            String certificateContents;
            String userName;
            Uint16 certType = 0;
            CIMValue cimValue;

            cimValue = inParams[0].getValue();
            cimValue.get(certificateContents);

            cimValue = inParams[1].getValue();
            cimValue.get(userName);

            cimValue = inParams[2].getValue();
            cimValue.get(certType);

            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "Certificate parameters:\n");
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "\tcertificateContents: %s",
                (const char*)certificateContents.getCString()));
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "\tcertificateType: %d",certType));
            if (userName == String::EMPTY)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                    "\tDoes not have an associated username");
            }
            else
            {
                PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                    "\tuserName: %s",(const char*)userName.getCString()));
            }

            // Check for a valid username if one is specified
            if (userName == String::EMPTY)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                    "The certificate does not have an associated user name");
            }
            else if (!System::isSystemUser(userName.getCString()))
            {
                throw CIMException(CIM_ERR_INVALID_PARAMETER,
                    "The user specified by userName is not a valid system "
                        "user.");
            }

            // Read in the certificate contents
            BIO* mem = BIO_new(BIO_s_mem());
            if (mem == NULL)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: Could not allocate memory for BIO.");
                throw PEGASUS_STD(bad_alloc)();
            }

            BIO_puts(mem, (const char*)certificateContents.getCString());

            //
            // Read the buffer until no more certificates found.
            //
            Uint32 certCount = 0;
            X509* tmpCert;
            while ((tmpCert = PEM_read_bio_X509(mem, NULL , 0, NULL)))
            {
                X509_free(tmpCert);
                certCount++;
            }

            //
            // If more than one certificate was found, error out.
            //
            if (certCount > 1)
            {
                PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: More than one cert in file : %d", certCount));
                BIO_free(mem);
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider."
                        "MULTIPLE_CERT_IN_FILE",
                    "Specified certificate file contains more than one "
                        "certificate.");
                throw CIMException(CIM_ERR_FAILED, parms);
            }

            BIO_free(mem);

            // Read in the certificate contents
            BIO* memCert = BIO_new(BIO_s_mem());
            if (memCert == NULL)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: Could not allocate memory for BIO.");
                throw PEGASUS_STD(bad_alloc)();
            }

            BIO_puts(memCert, (const char*)certificateContents.getCString());

            //
            // Read the certificate from buffer.
            //
            AutoPtr<X509, FreeX509Ptr> xCert(
                PEM_read_bio_X509(memCert, NULL , 0, NULL));
            if (xCert.get() == NULL)
            {
                BIO_free(memCert);

                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: Could not read x509 PEM format.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.BAD_X509_FORMAT",
                    "Could not read x509 PEM format.");
                throw CIMException(CIM_ERR_FAILED, parms);
            }
            BIO_free(memCert);

            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "Read x509 certificate...");

            char buf[256];
            String issuerName;
            String serialNumber;
            String subjectName;
            CIMDateTime notBefore;
            CIMDateTime notAfter;

            // Issuer name
            X509_NAME_oneline(X509_get_issuer_name(xCert.get()), buf, 256);
            issuerName = String(buf);

            // Serial number
            long rawSerialNumber =
                ASN1_INTEGER_get(X509_get_serialNumber(xCert.get()));
            char serial[256];
            sprintf(serial, "%lu", (unsigned long)rawSerialNumber);
            serialNumber = String(serial);

            // Subject name
            X509_NAME_oneline(X509_get_subject_name(xCert.get()), buf, 256);
            subjectName = String(buf);

            // Validity dates
            notBefore = getDateTime(X509_get_notBefore(xCert.get()));
            notAfter = getDateTime(X509_get_notAfter(xCert.get()));

            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "IssuerName: %s",(const char*)issuerName.getCString()));
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "SerialNumber: %s",(const char*)serialNumber.getCString()));
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "SubjectName: %s",(const char*)subjectName.getCString()));
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "NotBefore: %s",
                (const char*)notBefore.toString().getCString()));
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "NotAfter: %s",(const char*)notAfter.toString().getCString()));

            // Check validity with current datetime.
            // openssl will reject the certificate if it's not valid even if
            // we add it to the truststore
            try
            {
                if (CIMDateTime::getDifference(
                        CIMDateTime::getCurrentDateTime(), notBefore) > 0)
                {
                    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                        "Certificate or CRL is not valid yet.  "
                            "Check the timestamps on your machine.");
                    MessageLoaderParms parms(
                        "ControlProviders.CertificateProvider."
                            "CERT_NOT_VALID_YET",
                        "The certificate is not valid yet.  "
                            "Check the timestamps on your machine.");
                    throw CIMException(CIM_ERR_FAILED, parms);
                }

                if (CIMDateTime::getDifference(
                        notAfter, CIMDateTime::getCurrentDateTime()) > 0)
                {
                    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                        "Certificate or CRL is expired.");
                    MessageLoaderParms parms(
                        "ControlProviders.CertificateProvider.CERT_EXPIRED",
                        "The certificate has expired.");
                    throw CIMException(CIM_ERR_FAILED, parms);
                }
            }
            catch (DateTimeOutOfRangeException&)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Certificate or CRL dates are out of range.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.BAD_DATE_FORMAT",
                    "The validity dates are out of range.");
                throw CIMException(CIM_ERR_FAILED, parms);
            }

            AutoMutex lock(_trustStoreMutex);

            String certificateFileName = _getNewCertificateFileName(
                _sslTrustStore,
                X509_subject_name_hash(xCert.get()));
            if (userName != String::EMPTY)
            {
                PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                    "Certificate %s registered to %s",
                    (const char*)certificateFileName.getCString(),
                    (const char*)userName.getCString()));
            }
            else
            {
                PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                    "Certificate %s does not have a user name "
                    "associated with it",
                    (const char*)certificateFileName.getCString()));
            }

            // Build instance
            CIMInstance cimInstance(PEGASUS_CLASSNAME_CERTIFICATE);

            cimInstance.addProperty(CIMProperty(ISSUER_NAME_PROPERTY,
                CIMValue(issuerName)));
            cimInstance.addProperty(CIMProperty(SERIAL_NUMBER_PROPERTY,
                CIMValue(serialNumber)));
            cimInstance.addProperty(CIMProperty(SUBJECT_NAME_PROPERTY,
                CIMValue(subjectName)));
            cimInstance.addProperty(CIMProperty(USER_NAME_PROPERTY,
                CIMValue(userName)));
            cimInstance.addProperty(CIMProperty(FILE_NAME_PROPERTY,
                CIMValue(certificateFileName)));
            cimInstance.addProperty(CIMProperty(TRUSTSTORE_TYPE_PROPERTY,
                CIMValue(PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER)));
            cimInstance.addProperty(CIMProperty(NOT_BEFORE_PROPERTY,
                CIMValue(notBefore)));
            cimInstance.addProperty(CIMProperty(NOT_AFTER_PROPERTY,
                CIMValue(notAfter)));
            cimInstance.addProperty(CIMProperty(CERTIFICATE_TYPE_PROPERTY,
                CIMValue(certType)));

            // Set keys
            Array<CIMKeyBinding> keys;
            CIMKeyBinding key;
            key.setName(ISSUER_NAME_PROPERTY.getString());
            key.setValue(issuerName);
            key.setType(CIMKeyBinding::STRING);
            keys.append(key);

            key.setName(SERIAL_NUMBER_PROPERTY.getString());
            key.setType(CIMKeyBinding::STRING);
            key.setValue(String(serialNumber));
            keys.append(key);

            CIMKeyBinding kb(
                TRUSTSTORE_TYPE_PROPERTY,
                PG_SSLCERTIFICATE_TSTYPE_VALUE_SERVER);
            keys.append(key);

            // Set object path for instance
            cimInstance.setPath(CIMObjectPath(
                cimObjectPath.getHost(),
                cimObjectPath.getNameSpace(),
                PEGASUS_CLASSNAME_CERTIFICATE, keys));

            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "New certificate COP: %s",
                (const char*)cimInstance.getPath().toString().getCString()));

            // Attempt to add the instance to the repository first; that way if
            // this instance already exist it will take care of throwing
            // an error before we add the file to the truststore
            _repository->createInstance("root/PG_Internal", cimInstance);

            //
            // Check if the type of certificate is authority issued end entity.
            // If true, the certificate is not added to the truststore.
            // A username will be associated with the certificate in the
            // repository.
            //
            if ( ! (certType == TYPE_AUTHORITY_END_ENTITY ))
            {
                BIO* bio = _openBIOForWrite(
                    certificateFileName.getCString());

                if (!bio)
                {
                    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                        "Unable to add certificate to truststore. Failed "
                            "to open certificate file for write.");

                    MessageLoaderParms parms(
                        "ControlProviders.CertificateProvider."
                            "ERROR_WRITING_CERT",
                        "Unable to add certificate to truststore. Error while "
                            "trying to write certificate.");

                    throw CIMException(CIM_ERR_FAILED, parms);
                }

                if (!PEM_write_bio_X509(bio, xCert.get()))
                {
                    BIO_free_all(bio);
                    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                        "Unable to add certificate to truststore. "
                            "Error while trying to write certificate, "
                            "PEM_write_bio_X509 returned error");
                    MessageLoaderParms parms(
                        "ControlProviders.CertificateProvider."
                            "ERROR_WRITING_CERT",
                        "Unable to add certificate to truststore. "
                            "Error while trying to write certificate.");
                    throw CIMException(CIM_ERR_FAILED, parms);
                }

                BIO_free_all(bio);

                if (userName == String::EMPTY)
                {
                    PEG_TRACE((
                        TRC_CONTROLPROVIDER,
                        Tracer::LEVEL2,
                        "The certificate without an associated user name "
                            "from issuer %s has been added to the server "
                            "truststore.",
                        (const char*) issuerName.getCString()));
                }
                else
                {
                    PEG_TRACE((
                        TRC_CONTROLPROVIDER,
                        Tracer::LEVEL2,
                        "The certificate registered to %s from issuer "
                            "%s has been added to the server truststore.",
                        (const char*) userName.getCString(),
                        (const char*) issuerName.getCString()));
                }
            }

            CIMValue returnValue(Boolean(true));

            handler.deliver(returnValue);

            handler.complete();
        }
        else
        {
            throw CIMException(
                CIM_ERR_METHOD_NOT_FOUND, methodName.getString());
        }
    }
#ifdef PEGASUS_ENABLE_SSL_CRL_VERIFICATION
    else if (className == PEGASUS_CLASSNAME_CRL)
    {
        if (methodName == METHOD_ADD_CRL)
        {
            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "CertificateProvider::addCertificateRevocationList");

            String crlContents;
            CIMValue cimValue;

            cimValue = inParams[0].getValue();
            cimValue.get(crlContents);

            PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
                "inparam CRL contents: %s",
                (const char*)crlContents.getCString()));

            // Check for a valid CRL. Read in the CRL contents
            BIO* mem = BIO_new(BIO_s_mem());
            if (mem == NULL)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: Could not allocate memory for BIO.");
                throw PEGASUS_STD(bad_alloc)();
            }

            BIO_puts(mem, (const char*)crlContents.getCString());

            AutoPtr<X509_CRL, FreeX509CRLPtr> xCrl(
                PEM_read_bio_X509_CRL(mem, NULL, NULL, NULL));
            if (!xCrl.get())
            {
                BIO_free(mem);

                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Error: Could not read x509 PEM format.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.BAD_X509_FORMAT",
                    "Could not read x509 PEM format.");
                throw CIMException(CIM_ERR_FAILED, parms);
            }
            BIO_free(mem);

            PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "Successfully read x509 CRL...");

            char buf[256];
            String issuerName = String::EMPTY;
            CIMDateTime lastUpdate;
            CIMDateTime nextUpdate;
            Array<String> revokedSerialNumbers;
            Array<CIMDateTime> revocationDates;

            // Issuer name
            X509_NAME_oneline(X509_CRL_get_issuer(xCrl.get()), buf, 256);
            issuerName = String(buf);

            // Check validity of CRL
            // openssl will only issue a warning if the CRL is expired
            // However, we still don't want to let them register an expired
            // or invalid CRL
            lastUpdate = getDateTime(X509_CRL_get_lastUpdate(xCrl.get()));
            nextUpdate = getDateTime(X509_CRL_get_nextUpdate(xCrl.get()));
            try
            {
                if (CIMDateTime::getDifference(
                         CIMDateTime::getCurrentDateTime(), lastUpdate) > 0)
                {
                    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                       "The CRL is not valid yet. "
                           "Check timestamps on your machine.");
                    MessageLoaderParms parms(
                        "ControlProviders.CertificateProvider."
                            "CRL_NOT_VALID_YET",
                        "The CRL is not valid yet.  "
                            "Check the timestamps on your machine.");
                    throw CIMException(CIM_ERR_FAILED, parms);
                }
                if (CIMDateTime::getDifference(nextUpdate,
                        CIMDateTime::getCurrentDateTime()) > 0)
                {
                    PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                        "This CRL is not up-to-date.  "
                            "Check CA for the latest one.");
                    MessageLoaderParms parms(
                        "ControlProviders.CertificateProvider.CRL_EXPIRED",
                        "The CRL is not up-to-date.  "
                            "Check with the issuing CA for the latest one.");
                    throw CIMException(CIM_ERR_FAILED, parms);
                }
            }
            catch (DateTimeOutOfRangeException& ex)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Certificate or CRL dates are out of range.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.BAD_DATE_FORMAT",
                    "Certificate or CRL dates are out of range.");
                throw CIMException(CIM_ERR_FAILED, parms);
            }

            STACK_OF(X509_REVOKED)* revokedCertificates = NULL;
            int revokedCount = -1;

            revokedCertificates = X509_CRL_get_REVOKED(xCrl.get());
            revokedCount = sk_X509_REVOKED_num(revokedCertificates);

            if (revokedCount > 0)
            {
                PEG_TRACE((
                    TRC_CONTROLPROVIDER,
                    Tracer::LEVEL4,
                    "CRL contains %d revoked certificate entries.",
                    revokedCount));
            }
            else
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL3,
                    "Error: CRL is empty.");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.EMPTY_CRL",
                    "The CRL is empty.");
                throw CIMException(CIM_ERR_FAILED, parms);
            }

            AutoMutex lock(_crlStoreMutex);

            String crlFileName = _getCRLFileName(
                _crlStore, X509_NAME_hash(X509_CRL_get_issuer(xCrl.get())));

            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "IssuerName: %s",(const char*)issuerName.getCString()));
            PEG_TRACE((TRC_CONTROLPROVIDER,Tracer::LEVEL4,
                "FileName: %s",(const char*)crlFileName.getCString()));

            // ATTN: Take care of this conversion
            // For some reason i cannot do this in the BIO_write_filename call

            BIO* bio = _openBIOForWrite(crlFileName.getCString());

            if (!bio)
            {
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Unable to add CRL to truststore. Failed to open CRL file "
                        "for write ");

                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.ERROR_WRITING_CRL",
                    "Unable to add CRL to truststore. Error while trying to "
                        "write CRL.");

                throw CIMException(CIM_ERR_FAILED, parms);
            }

            if (!PEM_write_bio_X509_CRL(bio, xCrl.get()))
            {
                BIO_free_all(bio);
                PEG_TRACE_CSTRING(TRC_CONTROLPROVIDER, Tracer::LEVEL1,
                    "Unable to add CRL to truststore. "
                    "Error trying to write CRL,"
                    " PEM_write_bio_X509_CRL returned error");
                MessageLoaderParms parms(
                    "ControlProviders.CertificateProvider.ERROR_WRITING_CRL",
                    "Unable to add CRL to truststore. "
                        "Error while trying to write CRL.");
                throw CIMException(CIM_ERR_FAILED, parms);
            }

            BIO_free_all(bio);

            PEG_TRACE((
                TRC_CONTROLPROVIDER,
                Tracer::LEVEL2,
                "The CRL for issuer %s has been updated.",
                (const char*) issuerName.getCString()));

            if (_sslClientVerificationNotDisabled)
            {
                // Reload the CRL store
                PEG_TRACE_CSTRING(TRC_SSL, Tracer::LEVEL4,
                    "Loading CRL store after an update");
                _sslContextMgr->reloadCRLStore();
            }

            CIMValue returnValue(Boolean(true));

            handler.deliver(returnValue);

            handler.complete();
        }
        else
        {
            throw CIMException(
                CIM_ERR_METHOD_NOT_FOUND, methodName.getString());
        }
    }
#endif
    else
    {
        throw CIMException(CIM_ERR_INVALID_CLASS, className.getString());
    }
}

PEGASUS_NAMESPACE_END
