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

#include <Pegasus/Common/Config.h>
#include "SubscriptionKey.h"

PEGASUS_USING_PEGASUS;

PEGASUS_NAMESPACE_BEGIN


SubscriptionKey::SubscriptionKey(
    const CIMObjectPath & subscription)
{
    //
    //  Get filter and handler object paths from subscription Filter and Handler
    //  reference property values
    //
    Array<CIMKeyBinding> subscriptionKB = subscription.getKeyBindings ();

    subscriptionNamespace=subscription.getNameSpace().getString();
    subscriptionClassName=subscription.getClassName().getString();
    
    SubscriptionKey::_parseObjectName(
        subscriptionKB[0].getValue(),
        filterName,
        filterNamespace,
        filterClassName);

    SubscriptionKey::_parseObjectName(
        subscriptionKB[1].getValue(),
        handlerName,
        handlerNamespace,
        handlerClassName);

    if (String::equalNoCase(filterNamespace,subscriptionNamespace))
    {
        filterNamespace.clear();
    }
    if (String::equalNoCase(handlerNamespace,subscriptionNamespace))
    {
        handlerNamespace.clear();
    }
}

String SubscriptionKey::toString() const
{
    String str;
    str.reserveCapacity(200);
    str.append(subscriptionNamespace);
    str.append(":");
    str.append(subscriptionClassName);

    str.append(";Filter=");
    str.append(filterNamespace);
    str.append(":");
    str.append(filterClassName);
    str.append(".Name=\"");
    str.append(filterName);
    str.append("\"");

    str.append(";Handler=");
    str.append(handlerNamespace);
    str.append(":");
    str.append(handlerClassName);
    str.append(".Name=\"");
    str.append(handlerName);
    str.append("\"");
    
    return str;
};

void SubscriptionKey::_parseObjectName(
    const String & objectName,
    String & name,
    String & ns,
    String & className)
{
    static const Char16 slash = 0x002F;
    static const Char16 dot = 0x002E;
    static const Char16 quote = 0x0022;
    static const Char16 colon = 0x003A;
    static const Char16 equal = 0x003D;

    String x=objectName;
    Uint32 nsStart=0;
    Uint32 cnStart=0;
    
    // check for hostname, don't need it
    if (x[0] == slash && x[1] == slash)
    {
        // namespace starts after next slash
        nsStart = x.find(2, slash);
    }
    // namespace ends at a colon
    Uint32 nsEnd = x.find(nsStart,colon);
    if (PEG_NOT_FOUND != nsEnd)
    {
        // ignore slash in front of namespace if existing
        if (x[nsStart] == slash)
        {
            nsStart++;
        }
        // the colon needs be before the dot starting the keybindings,
        // else there is no namespace
        if (nsEnd < x.find(nsStart,dot))
        {
            ns=objectName.subString(nsStart,nsEnd-nsStart);
            cnStart=nsEnd+1;
        }
    }

    // Next is class name which is ending at a dot
    Uint32 classNameEnd = x.find(cnStart, dot);
    className = x.subString(cnStart,classNameEnd-cnStart);
    
    Uint32 keyNameStart=classNameEnd+1;
    // Name keybinding always is second in list because order is:
    // CreationClassName=,Name=,SystemCreationClassName=,SystemName=
    Uint32 keyNameEnd=x.find(keyNameStart,equal);
    keyNameEnd=x.find(keyNameEnd+1,equal);

    Uint32 valueStart=keyNameEnd+2; // one step forward + jump over quote
    Uint32 valueEnd=x.find(valueStart,quote);

    name = x.subString(valueStart,valueEnd-valueStart);
}

Boolean SubscriptionKeyEqualFunc::equal(
    const SubscriptionKey& x,
    const SubscriptionKey& y)
{
    if (x.filterName != y.filterName)
    {
        return false;
    }
    if (x.handlerName != y.handlerName)
    {
        return false;
    }
    if (!String::equalNoCase(x.subscriptionNamespace,y.subscriptionNamespace))
    {
        return false;
    }
    if (!String::equalNoCase(x.filterNamespace,y.filterNamespace))
    {
        return false;
    }
    if (!String::equalNoCase(x.handlerNamespace,y.handlerNamespace))
    {
        return false;
    }
    if (!String::equalNoCase(x.subscriptionClassName,y.subscriptionClassName))
    {
        return false;
    }
    if (!String::equalNoCase(x.filterClassName,y.filterClassName))
    {
        return false;
    }
    if (!String::equalNoCase(x.handlerClassName,y.handlerClassName))
    {
        return false;
    }
    return true;
};

inline Uint32 generateHash(const String& str)
{
    Uint32 h = 0;
    const Uint16* p = reinterpret_cast<const Uint16*> (str.getChar16Data());
    Uint32 n = str.size();

    while (n--)
    {
        h = 5 * h + *p++;
    }
    return h;
}

Uint32 SubscriptionKeyHashFunc::hash(const SubscriptionKey& x)
{
        return generateHash(x.handlerName)+generateHash(x.filterName);
};

Boolean operator==(const SubscriptionKey& key1, const SubscriptionKey& key2)
{
    return SubscriptionKeyEqualFunc::equal(key1,key2);
}

PEGASUS_NAMESPACE_END
