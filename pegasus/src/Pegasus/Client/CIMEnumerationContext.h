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

#ifndef Pegasus_CIMEnumerationContext_h
#define Pegasus_CIMEnumerationContext_h

#include <Pegasus/Client/Linkage.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>

PEGASUS_NAMESPACE_BEGIN

class CIMEnumerationContextRep;

//////////////////////////////////////////////////////////////
//
// CIMEnumerationContext
//
//////////////////////////////////////////////////////////////

/**
    This class represents the object containing the
    EnumerationContext for a sequence of Pull operations that
    must be communicated between the client application and
    CIMCLient.  It is created by the client for an Open and
    maintained by the CIMClient infrastructure.   To the Client
    application is completely opaque. The only functions
    available are constructiondestruction anc clear.
*/

class PEGASUS_CLIENT_LINKAGE CIMEnumerationContext
{
public:
    /**
        Constructs an empty CIMEnumerationContext object.
    */
    CIMEnumerationContext();

    /**
        Destructor.
    */
    ~CIMEnumerationContext();

    void clear();

    // Used to support error tests only. Clears only the enumeration
    // context parameter
    void clearEnumeration();

protected:
    CIMNamespaceName& getNameSpace();

    void setNameSpace(const CIMNamespaceName& n);

    // This is just a diagnostic tool
    String& getContextString();

    void setContextString(const String& s);
private:
    CIMEnumerationContextRep *_rep;

    // Make copy-constructor and assignment operators private
    CIMEnumerationContext(const CIMEnumerationContext& ec);
    CIMEnumerationContext& operator=(const CIMEnumerationContext& ec);
    friend class CIMClientRep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_CIMEnumerationContext_h */

