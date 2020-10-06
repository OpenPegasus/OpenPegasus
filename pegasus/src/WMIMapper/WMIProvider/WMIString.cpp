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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:    Barbara Packard    (bpackard@hp.com)
//              Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "WMIString.h"

PEGASUS_NAMESPACE_BEGIN

WMIString::WMIString(const String & s) : String(s)
{
}

WMIString::WMIString(const BSTR & s)
{
    try
    {
        CMyString mystr;
        mystr = s;
        *this = String((LPCTSTR)mystr);
    }
    catch(...)
    {
        PEG_TRACE_CSTRING(TRC_WMIPROVIDER, Tracer::LEVEL2,
            "WMIString::WMIString(const BSTR & s) - Constructor failed");
    }
}

WMIString::WMIString(const VARIANT & var)
{
    try
    {
        *this = WMIString(_bstr_t(_variant_t(var)));
    }
    catch(...)
    {
    }
}

WMIString::operator const BSTR(void) const
{
    return(_bstr_t((const wchar_t *)getChar16Data()));
}

WMIString::operator const VARIANT(void) const
{
    return(_variant_t(_bstr_t(BSTR(*this))));
}

PEGASUS_NAMESPACE_END
