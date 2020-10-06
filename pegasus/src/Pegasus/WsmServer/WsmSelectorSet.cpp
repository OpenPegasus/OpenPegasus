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
#include "WsmEndpointReference.h"

PEGASUS_NAMESPACE_BEGIN

PEGASUS_WSMSERVER_LINKAGE Boolean operator==(
    const WsmSelectorSet& set1,
    const WsmSelectorSet& set2)
{
    if (set1.selectors.size() != set2.selectors.size())
    {
        return false;
    }

    for (Uint32 i = 0; i < set1.selectors.size(); i++)
    {
        if (!(set1.selectors[i] == set2.selectors[i]))
        {
            return false;
        }
    }

    return true;
}

PEGASUS_WSMSERVER_LINKAGE Boolean operator==(
    const WsmSelector& sel1,
    const WsmSelector& sel2)
{
    if (sel1.type != sel2.type ||
        sel1.name != sel2.name)
    {
        return false;
    }

    if (sel1.type == WsmSelector::EPR)
    {
        return sel1.epr == sel2.epr;
    }

    return sel1.value == sel2.value;
}

PEGASUS_NAMESPACE_END
