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

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/CIMInstance.h>

#include "NormalizedSubscriptionTable.h"
#include "IndicationConstants.h"

PEGASUS_NAMESPACE_BEGIN

NormalizedSubscriptionTable::NormalizedSubscriptionTable(
    const Array<CIMInstance> &subscriptions)
{
    for (Uint32 i = 0; i < subscriptions.size() ; ++i)
    {
        if (!add(subscriptions[i].getPath()))
        {
            PEG_TRACE((TRC_INDICATION_SERVICE, Tracer::LEVEL2,
                "Subscription already exists : %s",
                (const char*)
                     subscriptions[i].getPath().toString().getCString()));
        }
    }
}

NormalizedSubscriptionTable::~NormalizedSubscriptionTable()
{
}

Boolean NormalizedSubscriptionTable:: remove(const CIMObjectPath &subPath)
{
    return _subscriptionTable.remove(SubscriptionKey(subPath));
}

Boolean NormalizedSubscriptionTable::add(const CIMObjectPath &subPath,
    const Boolean &value)
{
    return _subscriptionTable.insert(SubscriptionKey(subPath), value);
}

Boolean NormalizedSubscriptionTable::exists(const CIMObjectPath &subPath,
    Boolean &value)
{
    return _subscriptionTable.lookup(SubscriptionKey(subPath), value);
}

PEGASUS_NAMESPACE_END
