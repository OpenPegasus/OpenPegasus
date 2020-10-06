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

#ifndef Pegasus_ProviderClassList_h
#define Pegasus_ProviderClassList_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMName.h>


PEGASUS_NAMESPACE_BEGIN

/**
 * NamespaceClassList holds the indication subclasses with the
 * associated source namespace. This is used in the providerClassList
 * to keep track of the indication subclasses serviced by the provider
 * for each source namespace.
*/
struct NamespaceClassList
{
    CIMNamespaceName nameSpace;
    Array<CIMName> classList;
};

/**
 * SubscriptionWithSrcNamespace holds the subscription with the associated
 * source namespace. This is used by the getMatchingSubscriptions() function
 * to return the matched subscriptions with the source namespace of their
 * service. 
*/
struct SubscriptionWithSrcNamespace
{
    CIMNamespaceName nameSpace;
    CIMInstance subscription;
};

/**
    Entry for list of indication providers
 */
struct providerClassList
{
    CIMInstance provider;
    CIMInstance providerModule;
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
    Uint32 matchedIndCountPerSubscription;
#endif
    Array <NamespaceClassList> classList;
   // Enabling Indications on Remote CMPI -V 5245
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
    Boolean isRemoteNameSpace;
    String remoteInfo;
#endif
    String controlProviderName;
    providerClassList()
    {
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
        matchedIndCountPerSubscription = 0;
#endif
    }

    providerClassList (const providerClassList & rh)
    :   provider (rh.provider),
        providerModule (rh.providerModule),
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
        matchedIndCountPerSubscription(rh.matchedIndCountPerSubscription),
#endif
        classList (rh.classList)
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
        ,
        isRemoteNameSpace (rh.isRemoteNameSpace),
        remoteInfo (rh.remoteInfo)
#endif
        ,controlProviderName(rh.controlProviderName)
    {
    }

    providerClassList & operator= (const providerClassList & rh)
    {
        if (this != & rh)
        {
            provider = rh.provider;
            providerModule = rh.providerModule;
#ifdef PEGASUS_ENABLE_INDICATION_COUNT
            matchedIndCountPerSubscription = rh.matchedIndCountPerSubscription;
#endif
            classList = rh.classList;
#ifdef PEGASUS_ENABLE_REMOTE_CMPI
            isRemoteNameSpace = rh.isRemoteNameSpace;
            remoteInfo = rh.remoteInfo;
#endif
            controlProviderName = rh.controlProviderName;
        }
        return * this;
    }
};

typedef struct providerClassList ProviderClassList;

PEGASUS_NAMESPACE_END

#endif  /* Pegasus_ProviderClassList_h */
