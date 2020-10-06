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

#ifndef Pegasus_CIMInstanceProvider_h
#define Pegasus_CIMInstanceProvider_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Provider/CIMProvider.h>

#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Provider/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    This class defines a set of functions that support the manipulation of
    instances of a CIM object class and their properties.

    <p>The Instance Provider is the most common provider, and is the provider
    interface used by the CIM Server to perform instance and property
    manipulation operations.</p>

    <p>The Instance Provider receives operation requests from clients through
    calls to these functions by the CIM Server. Its purpose is to convert
    these to calls to system services, operations on system resources, or
    whatever platform-specific behavior is required to perform the operation
    modeled by the request. The specific requirements for each of the interface
    functions are discussed in their respective sections.</p>
*/
class PEGASUS_PROVIDER_LINKAGE CIMInstanceProvider : public virtual CIMProvider
{
public:
    /**
        Constructs a default CIMInstanceProvider object.
    */
    CIMInstanceProvider();

    /**
        Destructs a CIMInstanceProvider object.
    */
    virtual ~CIMInstanceProvider();

    /**
        Returns a specified CIM instance.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param instanceReference A fully qualified CIMObjectPath specifying
            the instance to be retrieved.

        @param includeQualifiers A Boolean indicating whether the returned
            instance must include the qualifiers for the instance and its
            properties.  Qualifiers may be included even if this flag is false.

        @param includeClassOrigin A Boolean indicating whether the returned
            instance must include the class origin for each of the instance
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
            Note: The client PropertyList and LocalOnly parameters are
            consolidated by the CIM Server into this single parameter.

        @param handler ResponseHandler object for delivery of results.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMObjectNotFoundException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void getInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler) = 0;

    /**
        Returns all instances of a specified class.

        <p>A typical implementation of this method will call the
        <tt>processing</tt> method in the <tt>ResponseHandler</tt> object,
        then iterate over the system resources representing instances of
        the CIM object, calling <tt>deliver</tt> on each iteration.
        Finally, it will call <tt>complete</tt> to inform the CIM Server that
        it has delivered all known instances. It is correct to call
        <tt>complete</tt> without calling <tt>deliver</tt> if no instances
        exist.</p>

        <p>A provider can be implemented and registered to perform
        operations for several levels of the same line of descent (e.g.,
        CIM_ComputerSystem and CIM_UnitaryComputerSystem). When this
        is done, the provider must return instances <i>only</i>
        for the deepest class for which it is registered, since
        the CIM Server will invoke <tt>enumerateInstances</tt> for all
        classes at and beneath the class specified by the client.</p>

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param classReference A fully qualified CIMObjectPath specifying
            the class for which to retrieve the instances.

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
            Note: The client PropertyList and LocalOnly parameters are
            consolidated by the CIM Server into this single parameter.

        @param handler ResponseHandler object for delivery of results.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void enumerateInstances(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList& propertyList,
        InstanceResponseHandler& handler) = 0;

    /**
        Returns the names of all instances of a specified class.

        <p>A typical implementation of this method will call the
        <tt>processing</tt> method in the <tt>ResponseHandler</tt> object,
        then iterate over the system resources representing instances of
        the CIM object, calling <tt>deliver</tt> on each iteration.
        Finally, it will call <tt>complete</tt> to inform the CIM Server that
        it has delivered all known instances. It is correct to call
        <tt>complete</tt> without calling <tt>deliver</tt> if no instances
        exist.</p>

        <p>A provider can be implemented and registered to perform
        operations for several levels of the same line of descent (e.g.,
        CIM_ComputerSystem and CIM_UnitaryComputerSystem). When this
        is done, the provider must return instances <i>only</i>
        for the deepest class for which it is registered, since
        the CIM Server will invoke <tt>enumerateInstanceNames</tt> for all
        classes at and beneath the class specified by the client.</p>

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param classReference A fully qualified CIMObjectPath specifying
            the class for which to retrieve the instance names.

        @param handler ResponseHandler object for delivery of results.
        The delivered CIMObjectPath values should not contain host or
        namespace information, as these attributes are not included in the
        WBEM protocol.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void enumerateInstanceNames(
        const OperationContext& context,
        const CIMObjectPath& classReference,
        ObjectPathResponseHandler& handler) = 0;

    /**
        Replaces all or part of a specified instance.

        This method is intended to be atomic.  Intermediate states should
        not be visible to other operations that access the instance.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param instanceReference A fully qualified CIMObjectPath specifying
            the instance to be modified.

        @param instanceObject A CIMInstance containing the properties and
            qualifiers with which to update the instance.

        @param includeQualifiers A Boolean indicating whether the instance
            qualifiers are to be updated in the instance and its properties.
            If false, no qualifiers are explicitly modified by this operation.

        @param propertyList A CIMPropertyList specifying the set of properties
            to be updated in the instance.  Support for this parameter is NOT
            optional.  If the propertyList cannot be honored, a
            CIMNotSupportedException must be thrown.  A null propertyList
            indicates that all properties must be updated.  Properties
            specified in the propertyList but not present in the
            <tt>instanceObject</tt> are to be replaced by the class default
            values or left null.

        @param handler ResponseHandler object for delivery of results.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMObjectNotFoundException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void modifyInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList& propertyList,
        ResponseHandler& handler) = 0;

    /**
        Creates a new instance.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param instanceReference Specifies the namespace and class name
            of the instance to create.  The key bindings are not present in
            the instanceReference, because an instance name is not defined
            until after the instance has been created.

        @param instanceObject The CIM instance to create.  If a key property
            is null, the provider <em>must</em> supply a valid value for the
            property or throw a CIMInvalidParameterException.  If any property
            value is invalid, the provider should throw a
            CIMInvalidParameterException.

        @param handler ResponseHandler object for delivery of results.  On
            a successful operation, the name of the newly created instance
            must be delivered.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMObjectAlreadyExistsException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void createInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        const CIMInstance& instanceObject,
        ObjectPathResponseHandler& handler) = 0;

    /**
        Deletes a specified instance.

        @param context An OperationContext object containing the context for
            the processing of the operation.  The context includes the name of
            the requesting user, language information, and other data.

        @param instanceReference A fully qualified CIMObjectPath specifying
            the instance to be deleted.

        @param handler ResponseHandler object for delivery of results.

        @exception CIMNotSupportedException
        @exception CIMInvalidParameterException
        @exception CIMObjectNotFoundException
        @exception CIMAccessDeniedException
        @exception CIMOperationFailedException
    */
    virtual void deleteInstance(
        const OperationContext& context,
        const CIMObjectPath& instanceReference,
        ResponseHandler& handler) = 0;
};

PEGASUS_NAMESPACE_END

#endif
