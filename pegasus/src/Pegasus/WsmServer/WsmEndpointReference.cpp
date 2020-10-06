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

#include <Pegasus/Common/Config.h>
#include <Pegasus/WsmServer/WsmSelectorSet.h>
#include <Pegasus/WsmServer/WsmConstants.h>
#include "WsmEndpointReference.h"

PEGASUS_NAMESPACE_BEGIN

WsmEndpointReference::WsmEndpointReference()
{
    selectorSet = new WsmSelectorSet;
}

WsmEndpointReference::~WsmEndpointReference()
{
    delete selectorSet;
}

WsmEndpointReference::WsmEndpointReference(const WsmEndpointReference& epr)
    : address(epr.address),
      resourceUri(epr.resourceUri)
{
    if (epr.selectorSet)
    {
        selectorSet = new WsmSelectorSet;
        selectorSet->selectors = epr.selectorSet->selectors;
    }
    else
    {
        selectorSet = 0;
    }
}

const String& WsmEndpointReference::getNamespace() const
{
    if (selectorSet)
    {
        for (Uint32 i = 0; i < selectorSet->selectors.size(); i++)
        {
            if (selectorSet->selectors[i].type == WsmSelector::VALUE &&
                selectorSet->selectors[i].name == "__cimnamespace")
            {
                return selectorSet->selectors[i].value;
            }
        }
    }

    return String::EMPTY;
}

void WsmEndpointReference::setNamespace(const String& Namespace)
{
    if (selectorSet)
    {
        for (Uint32 i = 0; i < selectorSet->selectors.size(); i++)
        {
            if (selectorSet->selectors[i].type == WsmSelector::VALUE &&
                selectorSet->selectors[i].name == PEGASUS_WS_CIMNAMESPACE)
            {
                selectorSet->selectors[i].value = Namespace;
            }
        }
    }
}

WsmEndpointReference& WsmEndpointReference::operator=(
    const WsmEndpointReference& epr)
{
    if (this != &epr)
    {
        address = epr.address;
        resourceUri = epr.resourceUri;

        delete selectorSet;
        if (epr.selectorSet)
        {
            selectorSet = new WsmSelectorSet;
            selectorSet->selectors = epr.selectorSet->selectors;
        }
        else
        {
            selectorSet = 0;
        }
    }

    return *this;
}

Boolean operator==(
    const WsmEndpointReference& epr1,
    const WsmEndpointReference& epr2)
{
    if (epr1.address != epr2.address ||
        epr1.resourceUri != epr2.resourceUri ||
        (epr1.selectorSet && !epr2.selectorSet) ||
        (!epr1.selectorSet && epr2.selectorSet))
    {
        return false;
    }

    if (epr1.selectorSet && epr2.selectorSet)
    {
        return *epr1.selectorSet == *epr2.selectorSet;
    }

    return true;
}

Boolean operator!=(
    const WsmEndpointReference& epr1,
    const WsmEndpointReference& epr2)
{
    return !(epr1 == epr2);
}

PEGASUS_NAMESPACE_END
