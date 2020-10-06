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

#include <stdlib.h>
#include "WsmInstance.h"


PEGASUS_NAMESPACE_BEGIN

WsmInstance::WsmInstance(const String& className) :
    _className(className)
{
}

WsmInstance::WsmInstance(const WsmInstance& inst) :
    _className(inst._className),
    _properties(inst._properties)
{
}

void WsmInstance::addProperty(const WsmProperty& prop)
{
    _properties.append(prop);
}

WsmProperty& WsmInstance::getProperty(Uint32 index)
{
    return _properties[index];
}

Uint32 WsmInstance::getPropertyCount() const
{
    return _properties.size();
}

static int _compare(const void* p1, const void* p2)
{
    const WsmProperty* prop1 = (WsmProperty*) p1;
    const WsmProperty* prop2 = (WsmProperty*) p2;
    return String::compare(prop1->getName(), prop2->getName());
}

void WsmInstance::sortProperties()
{
    qsort((void*) _properties.getData(), _properties.size(),
        sizeof(WsmProperty), _compare);
}

PEGASUS_NAMESPACE_END
