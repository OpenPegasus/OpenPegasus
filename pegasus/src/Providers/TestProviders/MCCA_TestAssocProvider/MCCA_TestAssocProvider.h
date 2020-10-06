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

#ifndef MCCA_TestAssocProvider_h
#define MCCA_TestAssocProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMAssociationProvider.h>
#include <Pegasus/Provider/ProviderException.h>

// Debugging enabling/disabling point
//#define CDEBUG(X) PEGASUS_STD(cout) << "MCCA_AssocTestProvider "
//                                    << X << PEGASUS_STD(endl)
#define CDEBUG(X);

PEGASUS_NAMESPACE_BEGIN

class MCCA_TestAssocProvider : public CIMAssociationProvider
{
public:
    MCCA_TestAssocProvider(void);
    virtual ~MCCA_TestAssocProvider(void);

    // CIMProvider interface
    virtual void initialize(CIMOMHandle & cimom);
    virtual void terminate(void);

    // CIMAssociationProvider interface
    virtual void associators(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void associatorNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        ObjectPathResponseHandler & handler);

    virtual void references(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler);

    virtual void referenceNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        ObjectPathResponseHandler & handler);

    CIMNamespaceName nameSpaceA;
    CIMNamespaceName nameSpaceB;
    CIMName testClassName;
    CIMName assocClassName;

protected:

};
PEGASUS_NAMESPACE_END
#endif // end of #ifdef MCCA_TestAssocProvider_h
