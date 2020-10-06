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

#ifndef _Pegasus_WsmSelectorSet_h
#define _Pegasus_WsmSelectorSet_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/WsmServer/WsmEndpointReference.h>

PEGASUS_NAMESPACE_BEGIN

struct WsmSelector
{
    enum Type { VALUE, EPR };

    String name;
    Type type;
    String value;
    WsmEndpointReference epr;

    WsmSelector()
        : type(VALUE)
    {
    }

    WsmSelector(const String& name_, const String& value_)
        : name(name_),
          type(VALUE),
          value(value_)
    {
    }

    WsmSelector(const String& name_, const WsmEndpointReference& epr_)
        : name(name_),
          type(EPR),
          epr(epr_)
    {
    }
};

class WsmSelectorSet
{
public:
    Array<WsmSelector> selectors;
};


PEGASUS_WSMSERVER_LINKAGE Boolean operator==(
    const WsmSelectorSet& set1,
    const WsmSelectorSet& set2);

PEGASUS_WSMSERVER_LINKAGE Boolean operator==(
    const WsmSelector& sel1,
    const WsmSelector& sel2);

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_WsmSelectorSet_h */
