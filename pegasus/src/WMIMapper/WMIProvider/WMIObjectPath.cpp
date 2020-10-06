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
//s
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:    Barbara Packard (barbara_packard@hp.com)
//        change Reference to ObjectPath - (bp 5/23/02)
//              Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "WMIObjectPath.h"
#include "WMIString.h"
#include "WMIType.h"

#include <string>
#include <iostream>
#include <locale>

using std::wstring;
using std::cout;

PEGASUS_NAMESPACE_BEGIN

WMIObjectPath::WMIObjectPath(const CIMObjectPath & cimObjectPath)
    : CIMObjectPath(cimObjectPath)
{
}

WMIObjectPath::WMIObjectPath(const BSTR bstr)
{
    String str = WMIString(bstr);

    // autofit string
    str.remove(::wcslen((WCHAR *)((USHORT *) str.getChar16Data())));


    // <object_path> ::= \\<host>\<namespace>:<class>.<key>
    // <object_path> ::= \<namespace>:<class>.<key>
    // <object_path> ::= <class>.<key>
    // <object_path> ::= <class>

    Uint32         pos = 0;
    Uint32         len = 0;
    const Char16 * p = str.getChar16Data();

    // get namespace host (optional)
    if((p[pos] == '\\') && (p[pos + 1] == '\\'))
    {
        pos += 2;    // skip "\\\\"

        // WMI Mapper can act as proxy to get date from other windows
        // systems, The "." as system name represents localhost in WMI.
        // However, the parser was not expecting that. So, we ignore the
        // "." in order to ignore the hostname part so that it will be
        // considered a request to the localhost.
        if (p[pos]=='.')
            pos++;

        // seek to '\\'
        for(len = 0; (p[pos] != Char16(0)) && (p[pos] != '\\'); len++, pos++);

        setHost(String(&p[pos - len], len));
    }

    // get namespace path (optional)
    if(p[pos] == '\\')
    {
        pos++;        // skip '\\'

        // seek to ':'
        for(len = 0; (p[pos] != Char16(0)) && (p[pos] != ':'); len++, pos++);

        // change slashes
        String temp(String(&p[pos - len], len));

        for(Uint32 i = 0; i < temp.size(); i++)
        {
            if(temp[i] == '\\')
            {
                temp[i] = '/';
            }
        }
        setNameSpace(temp);
    }

    p[pos] == ':' ? pos++ : 0;     // skip ':'

    // get class name (required)

    // seek to '.'
    for(len = 0; (p[pos] != Char16(0)) && (p[pos] != '.'); len++, pos++);

    setClassName(String(&p[pos - len], len));

    p[pos] == '.' ? pos++ : 0;     // skip '.'

    // get model key (optional)

    // <key> ::= <name> "=" <value> ["," <name> "=" <value>](0..n)
    //
    // <name> ::= alphabetic_character(1..n)
    // <value> ::= <string> | <number> | <boolean>
    // <string> ::= """ alphabetic_character | numeric_character |
    //                      symbol_character (1..n) """
    // <number> ::= numeric_character(1..n)
    // <boolean> ::= "true" | "false"

    while(p[pos] != Char16(0))
    {
        // get keys
        CIMKeyBinding key;

        // seek to '='
        for(len = 0; (p[pos] != Char16(0)) && (p[pos] != '='); len++, pos++);

        key.setName(String(&p[pos - len], len));

        p[pos] == '=' ? pos++ : 0;     // skip '='

        // A string value may be enclosed in single quotes or double quotes
        if ((p[pos] == '\"') || (p[pos] == '\''))
        {
            char openingQuote = p[pos];

            // parse string value

            // check for embedded quotes. for example,
            // class1.property1A="class2.property2A="2A",
            // property2B="2B"",property2A="foo"
            // or class1.property1A=",",property2A="."

            bool quote = false;

            // seek to closing quote or eos

            for(len = 0;
                (p[pos] != Char16(0)) && !((p[pos] == ',') && (!quote));
                len++, pos++)
            {
                quote = (p[pos] == openingQuote) ? !quote : quote;

                // By Jair - check if it is not an 'internal' quote.
                // If it is, must be discarded in this case.
                if (!quote && len) quote = (p[pos - 1] == '\\');
            }

            // strip outer quotes
            String temp(&p[pos - len], len);
            if((temp.size() != 0) && (temp[0] == openingQuote) &&
                (temp[temp.size() - 1] == openingQuote))
            {
                temp.remove(temp.size() - 1, 1);
                temp.remove(0, 1);
            }

            // remove escape sequences (the parent class will
            //put them back later).
            for(Uint32 i = 0; i < temp.size(); i++)
            {
                if((temp[i] == '\\') &&
                   ((temp[i + 1] == '\\') ||
                    (temp[i + 1] == '"')  ||
                    (temp[i + 1] == '\n') ||
                    (temp[i + 1] == '\r') ||
                    (temp[i + 1] == '\t')))
                {
                    temp.remove(i, 1);
                }
            }

            key.setValue(temp);
            key.setType(CIMKeyBinding::STRING);
        }
        else if((p[pos] == 't') || (p[pos] == 'T') ||
            (p[pos] == 'f') || (p[pos] == 'F'))
        {
            // parse boolean value

            // seek to ',' or eos
            for(len = 0; (p[pos] != Char16(0)) && (p[pos] != ',');
                len++, pos++);

            key.setValue(String(&p[pos - len], len));

            key.setType(CIMKeyBinding::BOOLEAN);
        }
        else if(std::isdigit<wchar_t>(p[pos], std::locale()))
        {
            // parse numeric value

            // seek to ',' or eos
            for(len = 0; (p[pos] != Char16(0)) && (p[pos] != ',');
                len++, pos++);

            key.setValue(String(&p[pos - len], len));

            key.setType(CIMKeyBinding::NUMERIC);
        }
        else
        {
            throw(std::invalid_argument("unrecognized key type"));
        }

        // update key list
        Array<CIMKeyBinding> keySet = getKeyBindings();

        keySet.append(key);

        setKeyBindings(keySet);

        p[pos] == ',' ? pos++ : 0;     // skip ','
    }
}

WMIObjectPath::WMIObjectPath(IWbemClassObject * pObject)
{
    // save pointer in local variable so that in the event an exception is
    // thrown, the object is released
    _com_ptr_t<_com_IIID<IWbemClassObject,&__uuidof(IWbemClassObject)>> object;

    try
    {
        object = pObject;
    }
    catch(_com_error *)
    {
        throw(std::invalid_argument("invalid IWbemClassObject pointer."));
    }

    _variant_t vt;

    // get host (optional)
    vt.Clear();
    pObject->Get(_bstr_t(L"__SERVER"), 0, &vt, 0, 0);

    try
    {
        setHost(WMIString(vt));
    }
    catch(...)
    {
    }

    // get namespace (optional)
    vt.Clear();
    pObject->Get(_bstr_t(L"__NAMESPACE"), 0, &vt, 0, 0);

    try
    {
        // convert slashes
        String temp = WMIString(vt);

        for(Uint32 i = 0; i < temp.size(); i++)
        {
            if(temp[i] == Char16('\\'))
            {
                temp[i] = Char16('/');
            }
        }

        setNameSpace(temp);
    }
    catch(...)
    {
    }

    // get class (required)
    vt.Clear();
    pObject->Get(_bstr_t(L"__CLASS"), 0, &vt, 0, 0);

    try
    {
        setClassName(WMIString(vt));
    }
    catch(...)
    {
        throw(InvalidNameException(WMIString(vt)));
    }

    SAFEARRAY * pKeyArray = 0;

    // get keys (optional)
    vt.Clear();
    pObject->GetNames(0, WBEM_FLAG_KEYS_ONLY, 0, &pKeyArray);

    long lbound = 0;
    long ubound = 0;

    ::SafeArrayGetLBound(pKeyArray, 1, &lbound);
    ::SafeArrayGetUBound(pKeyArray, 1, &ubound);

    Array<CIMKeyBinding> KeyList;

    for(long i = lbound; i <= ubound; ++i)
    {
        BSTR Property = 0;

        ::SafeArrayGetElement(pKeyArray, &i, &Property);

        KeyList.append(CIMKeyBinding(WMIString(_bstr_t(Property, false)),
            String(), CIMKeyBinding::STRING));
    }

    ::SafeArrayDestroy(pKeyArray);

    for(Uint32 j = 0; j < KeyList.size(); ++j)
    {
        CIMTYPE type;

        vt.Clear();
        pObject->Get(_bstr_t(WMIString(KeyList[j].getName().getString())),
            0, &vt, &type, 0);

        KeyList[j].setValue(WMIString(vt));

        switch(type)
        {
        case CIM_SINT8:
        case CIM_UINT8:
        case CIM_SINT16:
        case CIM_UINT16:
        case CIM_SINT32:
        case CIM_UINT32:
        case CIM_SINT64:
        case CIM_UINT64:
        case CIM_REAL32:
        case CIM_REAL64:
            KeyList[j].setType(CIMKeyBinding::NUMERIC);

            break;
        case CIM_STRING:
            KeyList[j].setType(CIMKeyBinding::STRING);

            break;
        case CIM_BOOLEAN:
            KeyList[j].setType(CIMKeyBinding::BOOLEAN);

            break;
        default:
            break;
        }
    }

    setKeyBindings(KeyList);
}

WMIObjectPath::operator const BSTR(void) const
{
    _bstr_t bstr;

    if(!(getNameSpace().isNull()))
    {
        bstr += _bstr_t(WMIString(getNameSpace().getString()));
        bstr += L":";
    }

    bstr += _bstr_t(WMIString(getClassName().getString()));

    Array<CIMKeyBinding> KeyList = getKeyBindings();

    if(KeyList.size() != 0)
    {
        bstr += L".";
    }

    for(Uint32 i = 0; i < KeyList.size(); ++i)
    {
        bstr += _bstr_t(WMIString(KeyList[i].getName().getString()));
        bstr += L"=";
        bstr += L"\"";

        String value(KeyList[i].getValue());

        for(Uint32 j = 0; j < value.size(); j++)
        {
            if((value[j] == '\\') || (value[j] == '\"'))
            {
                bstr += L"\\";
            }

            bstr += _bstr_t(WMIString(value.subString(j, 1)));
        }

        bstr += L"\"";

        if(i != (KeyList.size() - 1))
        {
            bstr += L",";
        }
    }

    return(bstr);
}

PEGASUS_NAMESPACE_END
