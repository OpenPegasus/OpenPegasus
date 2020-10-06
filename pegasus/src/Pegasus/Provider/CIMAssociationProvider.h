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

#ifndef Pegasus_CIMAssociationProvider_h
#define Pegasus_CIMAssociationProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMProvider.h>

#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class defines the set of methods implemented by an association
    provider.  A providers that derives from this class must implement all of
    the methods.  A minimal method implementation simply throws a
    CIMNotSupportedException. The methods match the association operations
    defined for the client:
        <UL>
        <LI> referenceNames
        <LI> references
        <LI> associatorNames
        <LI> associators
        </UL>
    Provider instrumentation differs semantically from a client request in
    that a provider implementation is specific to one association class while
    a client request may span many association classes.  Also, an assocation
    provider deals only with instance data, while a client may also issue
    class level association requests.
*/
class PEGASUS_PROVIDER_LINKAGE CIMAssociationProvider :
    public virtual CIMProvider
{
public:
    /**
        Constructs a default CIMAssociationProvider object.
    */
    CIMAssociationProvider();

    /**
        Destructs a CIMAssociationProvider object.
    */
    virtual ~CIMAssociationProvider();

    /**
        Enumerates CIM instances that are associated to a specified instance
            via a specified association class.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param objectName A fully qualified CIMObjectPath specifying the
            "source" instance for which to enumerate associated instances.

        @param associationClass The name of the association class through
            which the returned instances are associated with the source
            instance.  Instances associated through another class must not be
            returned.

        @param resultClass The name of the class to which the returned
            instances must belong.  Instances of this class and its subclasses
            may be returned, but not instances of another class.  If null,
            the set of returned instances is not constrained by their class.

        @param role The role of the source instance in the association
            instance.  If the role is not an empty string, the returned
            instances must be associated with the source instance such that
            the name of the assocation property referring to the source
            instance matches the role value.

        @param resultRole The role of the returned instances in the association
            instance.  If the role is not an empty string, the returned
            instances must be associated with the source instance such that
            the name of the assocation property referring to the returned
            instances matches the role value.

        @param includeQualifiers A Boolean indicating whether the returned
            instances must include the qualifiers for the instance and its
            properties.  Qualifiers may be included even if this flag is false.

        @param includeClassOrigin A Boolean indicating whether the returned
            instances must include the class origin for each of the instance
            elements.

        @param propertyList A CIMPropertyList specifying the minimum set of
            properties required in the returned instance.  Support for this
            parameter is optional.This parameter can be used by the provider
            to optimize their code and not fill properties which are not
            requested.The cimserver will filter all properties using an
            efficient algorithm hence the returned instance may contain
            properties not specified in the list.A null propertyList
            indicates that all properties must be included.  A non-null,
            but empty, propertyList indicates that no properites are required.

        @param handler ResponseHandler object for delivery of results.
            Note: The delivered instances are expected to contain the
            instance path.  If the host and namespace are not included in the
            path, they are added by the CIM Server.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
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
        ObjectResponseHandler & handler) = 0;

    /**
        Enumerates CIM instance names that are associated to a specified
        instance via a specified association class.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param objectName A fully qualified CIMObjectPath specifying the
            "source" instance for which to enumerate associated instance names.

        @param associationClass The name of the association class through
            which the returned instance names are associated with the source
            instance.  Names of instances associated through another class
            must not be returned.

        @param resultClass The name of the class to which the returned
            instance names must belong.  Names of instances of this class and
            its subclasses may be returned, but not instances of another class.
            If null, the set of returned instance names is not constrained by
            their class.

        @param role The role of the source instance in the association
            instance.  If the role is not an empty string, the returned
            instance names must be associated with the source instance such
            that the name of the assocation property referring to the source
            instance matches the role value.

        @param resultRole The role of the returned instance names in the
            association instance.  If the role is not an empty string, the
            returned instance names must be associated with the source
            instance such that the name of the assocation property referring
            to the returned instance names matches the role value.

        @param handler ResponseHandler object for delivery of results.
            Note: The delivered instance names are expected to contain host
            and namespace information.  If not included, they are added by
            the CIM Server.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void associatorNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & associationClass,
        const CIMName & resultClass,
        const String & role,
        const String & resultRole,
        ObjectPathResponseHandler & handler) = 0;

    /**
        Enumerates CIM association instances that refer to a specified
        instance.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param objectName A fully qualified CIMObjectPath specifying the
            "source" instance for which to enumerate referring instances.

        @param resultClass The class name of the association instances to be
            returned.  Instances of other classes must not be returned.

        @param role The role of the source instance in the association
            instance.  If the role is not an empty string, the returned
            association instances must refer to the source instance via a
            property whose name matches the role value.

        @param includeQualifiers A Boolean indicating whether the returned
            instances must include the qualifiers for the instance and its
            properties.  Qualifiers may be included even if this flag is false.

        @param includeClassOrigin A Boolean indicating whether the returned
            instances must include the class origin for each of the instance
            elements.

        @param propertyList A CIMPropertyList specifying the minimum set of
            properties required in the returned instance.  Support for this
            parameter is optional.This parameter can be used by the provider
            to optimize their code and not fill properties which are not
            requested.The cimserver will filter all properties using an
            efficient algorithm hence the returned instance may contain
            properties not specified in the list.A null propertyList
            indicates that all properties must be included.  A non-null,
            but empty, propertyList indicates that no properites are required.

        @param handler ResponseHandler object for delivery of results.
            Note: The delivered instances are expected to contain the
            instance path.  If the host and namespace are not included in the
            path, they are added by the CIM Server.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void references(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        ObjectResponseHandler & handler) = 0;

    /**
        Enumerates the names of CIM association instances that refer to a
        specified instance.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param objectName A fully qualified CIMObjectPath specifying the
            "source" instance for which to enumerate referring instance names.

        @param resultClass The class name of the association instance names to
            be returned.  Names of instances of other classes must not be
            returned.

        @param role The role of the source instance in the association
            instance.  If the role is not an empty string, the returned
            association instance names must refer to the source instance via a
            property whose name matches the role value.

        @param handler ResponseHandler object for delivery of results.
            Note: The delivered instance names are expected to contain host
            and namespace information.  If not included, they are added by
            the CIM Server.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void referenceNames(
        const OperationContext & context,
        const CIMObjectPath & objectName,
        const CIMName & resultClass,
        const String & role,
        ObjectPathResponseHandler & handler) = 0;
};

PEGASUS_NAMESPACE_END

#endif
