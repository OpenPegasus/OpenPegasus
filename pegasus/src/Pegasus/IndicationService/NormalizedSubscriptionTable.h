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

#ifndef Pegasus_NormalizedSubscriptionTable_h
#define Pegasus_NormalizedSubscriptionTable_h

#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/ReadWriteSem.h>
#include <Pegasus/Server/Linkage.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/General/SubscriptionKey.h>

PEGASUS_NAMESPACE_BEGIN

/**
    NormalizedSubscriptionTable holds the all normalized subscriptions. This is
    used to avoid creation of duplicate subscriptions and handling of similar
    subscription requests at the same time.
*/

class PEGASUS_SERVER_LINKAGE NormalizedSubscriptionTable
{
public:
    NormalizedSubscriptionTable(const Array<CIMInstance> &subscriptions);
    ~NormalizedSubscriptionTable();

    /**
        Adds subscription path to the subscriptionTable after
        subscription path normalization.
        @param subPath, subscription path.
        @param value, associated key value.
        @return true if inserted successfully else false.
    */
    Boolean add(const CIMObjectPath &subPath, const Boolean &value = true);

    /**
        Deletes subscription path from the subscriptionTable after
        subscription path normalization.
        @param subPath subscription path.
        @return true if deleted successfully else false.
    */
    Boolean remove(const CIMObjectPath &subPath);

    /**
        Checks for subscription path from the subscriptionTable after
        subscription path normalization.
        @param subPath subscription path.
        @param value, output arg, contains associated key value if found.
        @return true if found else false.
    */
    Boolean exists(const CIMObjectPath &subPath, Boolean &value);

private:
    NormalizedSubscriptionTable& operator =
        (const NormalizedSubscriptionTable&);
    NormalizedSubscriptionTable(const NormalizedSubscriptionTable&);


    /**
        NormalizedSubscriptionTable maintains all the subscription objectpaths
        in normalized form.
    */
    HashTable <SubscriptionKey, Boolean, SubscriptionKeyEqualFunc,
        SubscriptionKeyHashFunc> _subscriptionTable;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_NormalizedSubscriptionTable_h */

