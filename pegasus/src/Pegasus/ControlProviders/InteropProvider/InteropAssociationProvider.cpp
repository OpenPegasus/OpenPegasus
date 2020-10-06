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


///////////////////////////////////////////////////////////////////////////////
//  Interop Provider - This provider services those classes from the
//  DMTF Interop schema in an implementation compliant with the SMI-S v1.1
//  Server Profile
//
//  Please see PG_ServerProfile20.mof in the directory
//  $(PEGASUS_ROOT)/Schemas/Pegasus/InterOp/VER20 for details regarding the
//  classes supported by this control provider.
//
//  Interop forces all creates to the PEGASUS_NAMESPACENAME_INTEROP
//  namespace. There is a test on each operation that returns
//  the Invalid Class CIMDError
//  This is a control provider and as such uses the Tracer functions
//  for data and function traces.  Since we do not expect high volume
//  use we added a number of traces to help diagnostics.
///////////////////////////////////////////////////////////////////////////////


#include "InteropProvider.h"
#include "InteropProviderUtils.h"
#include "InteropConstants.h"
#include <Pegasus/Common/ArrayIterator.h>
#include <Pegasus/Common/Pegasus_inl.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

/*****************************************************************************
 *
 * Implementation of AssociationProvider associators method
 *
 *****************************************************************************/
void InteropProvider::associators(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
            "InteropProvider::associators()");
    initProvider();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s associators. objectName= %s, assocClass= %s resultClass= %s "
            "role= %s resultRole %s, includeQualifiers= %s, "
            "includeClassOrigin= %s, PropertyList= %s",
        thisProvider,
        (const char *)objectName.toString().getCString(),
        (const char *)associationClass.getString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char *)role.getCString(),
        (const char *)resultRole.getCString(),
                      boolToString(includeQualifiers),
                      boolToString(includeClassOrigin),
        (const char *)propertyList.toString().getCString()));

    handler.processing();
    String originRole = role;
    String targetRole = resultRole;
    Uint32 numIterations = 1;
    //
    // The localReferences call retrieves instances of the desired association
    // class and sets the originRole and targetRole properties if currently
    // empty.
    //
    if (associationClass.equal(PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE))
    {
        if (originRole.size() == 0 && targetRole.size() == 0)
        {
            originRole = String("Antecedent");
            targetRole = String("Dependent");
            numIterations = 2;
        }
    }
    for (Uint32 i = 0; i < numIterations; ++i)
    {
        Array<CIMInstance> refs = localReferences(
            context,
            objectName,
            associationClass,
            originRole,
            targetRole,
            CIMPropertyList(),
            resultClass);

        if( refs.size() )
        {
            Array<CIMInstance> refObjs =
                getReferencedInstances(refs,targetRole,context,propertyList);
            ConstArrayIterator<CIMInstance> refsIterator(refObjs);
            for(Uint32 i = 0; i < refsIterator.size(); i++)
            {
                handler.deliver(refsIterator[i]);
            }
        }

        if (numIterations == 2)
        {
            originRole = String("Dependent");
            targetRole = String("Antecedent");
        }
    }
    handler.complete();

    PEG_METHOD_EXIT();
}

/*****************************************************************************
 *
 * Implementation of AssociationProvider associatorNames method
 *
 *****************************************************************************/
void InteropProvider::associatorNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & associationClass,
    const CIMName & resultClass,
    const String & role,
    const String & resultRole,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::associatorNames()");
    initProvider();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s associatorNames.objectName= %s, assocClass= %s resultClass= %s "
            "role= %s resultRole = %s",
        thisProvider,
        (const char *)objectName.toString().getCString(),
        (const char *)associationClass.getString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char *)role.getCString(),
        (const char *)resultRole.getCString()));

    handler.processing();
    String originRole = role;
    String targetRole = resultRole;
    Uint32 numIterations = 1;
    //
    // The localReferences call retrieves instances of the desired association
    // class and sets the originRole and targetRole properties if currently
    // empty.
    //
    if (associationClass.equal(PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE))
    {
        if (originRole.size() == 0 && targetRole.size() == 0)
        {
            originRole = String("Antecedent");
            targetRole = String("Dependent");
            numIterations = 2;
        }
    }
    for (Uint32 i = 0; i < numIterations; ++i)
    {
        Array<CIMInstance> refs = localReferences(
            context,
            objectName,
            associationClass,
            originRole,
            targetRole,
            CIMPropertyList(),
            resultClass);
        for (Uint32 j = 0, n = refs.size(); j < n; ++j)
        {
            CIMInstance & currentRef = refs[j];
            CIMObjectPath currentTarget = getRequiredValue<CIMObjectPath>(
                currentRef,
                targetRole);
            handler.deliver(currentTarget);
        }
        if (numIterations == 2)
        {
            originRole = String("Dependent");
            targetRole = String("Antecedent");
        }
    }
    handler.complete();
    PEG_METHOD_EXIT();
}


/*****************************************************************************
 *
 * Implementation of AssociationProvider references method
 *
 *****************************************************************************/
void InteropProvider::references(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    ObjectResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::references()");
    initProvider();
    PEG_TRACE((TRC_CONTROLPROVIDER, Tracer::LEVEL4,
        "%s references. objectName= %s, resultClass= %s role= %s "
            "includeQualifiers= %s, includeClassOrigin= %s, PropertyList= %s",
        thisProvider,
        (const char *)objectName.toString().getCString(),
        (const char *)resultClass.getString().getCString(),
        (const char *)role.getCString(),
                      boolToString(includeQualifiers),
                      boolToString(includeClassOrigin),
        (const char *)propertyList.toString().getCString()));

    handler.processing();
    String tmpRole = role;
    String tmpTarget;
    Uint32 numIterations = 1;
    //
    // Makes call to internal references method to get result, supplying the
    // role parameter, but obviously not setting a resultRole/target parameter.
    //
    if (resultClass.equal(PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE))
    {
        if (tmpRole.size() == 0)
        {
            tmpRole = String("Antecedent");
            tmpTarget = String("Dependent");
            numIterations = 2;
        }
    }
    for (Uint32 i = 0; i < numIterations; ++i)
    {
        Array<CIMInstance> refs = localReferences(
            context,
            objectName,
            resultClass,
            tmpRole,
            tmpTarget);
        for (Uint32 j = 0, n = refs.size(); j < n; ++j)
        {
            handler.deliver((CIMObject)refs[j]);
        }
        if (numIterations == 2)
        {
            tmpRole = String("Dependent");
            tmpTarget = String("Antecedent");
        }
    }
    handler.complete();
    PEG_METHOD_EXIT();
}

/*****************************************************************************
 *
 * Implementation of AssociationProvider referenceNames method
 *
 *****************************************************************************/
void InteropProvider::referenceNames(
    const OperationContext & context,
    const CIMObjectPath & objectName,
    const CIMName & resultClass,
    const String & role,
    ObjectPathResponseHandler & handler)
{
    PEG_METHOD_ENTER(TRC_CONTROLPROVIDER,
        "InteropProvider::referenceNames()");
    initProvider();
    handler.processing();

    String tmpRole = role;
    String tmpTarget;
    Uint32 numIterations = 1;
    //
    // Makes call to internal references method to get result, supplying the
    // role parameter, but obviously not setting a resultRole/target parameter.
    //
    if (resultClass.equal(PEGASUS_CLASSNAME_PG_REFERENCEDPROFILE))
    {
        if (tmpRole.size() == 0)
        {
            tmpRole = String("Antecedent");
            tmpTarget = String("Dependent");
            numIterations = 2;
        }
    }
    for (Uint32 i = 0; i < numIterations; ++i)
    {
        Array<CIMInstance> refs = localReferences(
            context,
            objectName,
            resultClass,
            tmpRole,
            tmpTarget);
        for (Uint32 j = 0, n = refs.size(); j < n; ++j)
        {
            handler.deliver(refs[j].getPath());
        }
        if (numIterations == 2)
        {
            tmpRole = String("Dependent");
            tmpTarget = String("Antecedent");
        }
    }

    handler.complete();

    PEG_METHOD_EXIT();
}


PEGASUS_NAMESPACE_END
// END_OF_FILE
