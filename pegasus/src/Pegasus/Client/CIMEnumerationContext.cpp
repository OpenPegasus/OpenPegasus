//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Client/CIMEnumerationContext.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/InternalException.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/CIMOperationType.h>

PEGASUS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////
//
// CIMEnumerationContextRep
//
//////////////////////////////////////////////////////////////

class CIMEnumerationContextRep
{
public:
    String enumerationContextString;
    CIMNamespaceName nameSpace;
    CIMOperationType _operationType;
};

//////////////////////////////////////////////////////////////
//
// CIMEnumerationContext Methods
//
//////////////////////////////////////////////////////////////

CIMEnumerationContext::CIMEnumerationContext()
{
    _rep = new CIMEnumerationContextRep();
}

CIMEnumerationContext::~CIMEnumerationContext()
{
    delete _rep;
}

CIMNamespaceName& CIMEnumerationContext::getNameSpace()
{
    return _rep->nameSpace;
}

void CIMEnumerationContext::setNameSpace(const CIMNamespaceName& n)
{
    _rep->nameSpace = n;
}

String& CIMEnumerationContext::getContextString()
{
    return _rep->enumerationContextString;
}

void CIMEnumerationContext::setContextString(const String& s)
{
    _rep->enumerationContextString = s;
}

void CIMEnumerationContext::clear()
{
    _rep->enumerationContextString.clear();
    _rep->nameSpace = CIMNamespaceName();
}

void CIMEnumerationContext::clearEnumeration()
{
    _rep->enumerationContextString.clear();
}

PEGASUS_NAMESPACE_END
