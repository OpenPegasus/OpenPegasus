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
/* NOCHKSRC */

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
/** This class defines a set of functions that support the
manipulation of instances of a CIM object class and their
properties.

<p>The Instance Provider is the most common provider, and
is the provider interface used by the CIM Server to perform instance
and property
manipulation requests from CIM clients. Instance providers
may be implemented for any CIM class, including <i>Association</i>
classes.</p>

<p>In addition to
functions inherited from the
{@link CIMProvider CIMProvider} interface,
the functions in the Instance Provider interface are:</p>

<p><ul>
<li>{@link getInstance getInstance}</li>
<li>{@link enumerateInstances enumerateInstances}</li>
<li>{@link enumerateInstanceNames enumerateInstanceNames}</li>
<li>{@link modifyInstance modifyInstance}</li>
<li>{@link createInstance createInstance}</li>
<li>{@link deleteInstance deleteInstance}</li>
</ul></p>

<p>The Instance Provider receives operation requests from
clients through calls to these functions by the CIM Server. Its
purpose is to convert these to calls to system services,
operations on system resources, or whatever platform-specific
behavior is required to perform the operation modeled by
the request. The specific requirements for each of the interface
functions are discussed in their respective sections.</p>
*/
class PEGASUS_PROVIDER_LINKAGE CIMInstanceProvider : public virtual CIMProvider
{
public:
    /**
    Constructor.
    The constructor should not do anything.
    */
    CIMInstanceProvider(void);

    /**
    Destructor.
    The destructor should not do anything.
    */
    virtual ~CIMInstanceProvider(void);

    /**
    \Label{getInstance}
    Return a single instance.

    <p><tt>getInstance</tt> is called with an
    {@link CIMObjectPath instanceReference} specifying a CIM
    instance to be returned. The provider should determine whether
    the specification corresponds to a valid instance. If so, it will
    construct a <tt>{@link CIMInstance CIMInstance}</tt>
    and deliver this to the CIM Server via the
    <tt>{@link ResponseHandler ResponseHandler}</tt>
    callback. If the specified instance does not exist, this
    function should throw an <tt>{@link CIMObjectNotFoundException CIMObjectNotFoundException}.</tt>
    </p>

    <p>A provider can be implemented and registered to perform
    operations for
    several levels of the same line of descent (e.g.,
    CIM_ComputerSystem and CIM_UnitaryComputerSystem). When this
    is done, care must be taken to return the same set of key
    values regardless of which class was specified in the
    operation.</p>

    @param context specifies the client user's context for this operation,
    including the User ID.

    @param instanceReference specifies the fully qualified object path
    of the instance of interest.

    @param includeQualifiers indicates whether the returned instance must
    include the qualifiers for the instance and properties.  Qualifiers may
    be included even if this flag is false.

    @param includeClassOrigin indicates whether the returned instance must
    include the class origin for each of the instance elements.

    @param propertyList if not null, this parameter
    specifies the minimum set of properties required in instances
    returned by this operation. Because
    support for this parameter is optional, the instances may contain
    additional properties not specified in the list.
	NOTE: The provider does NOT receive the client filtering parameter
	localOnly.  This is resolved in the CIMOM into the propertyList so 
	that the property list represents the complete set of properties to
	be returned.
	If the propertyList is NULL all properties are returned.  If it is
	nonNULL but empty, no properites are to be returned.

    @param handler a {@link ResponseHandler ResponseHandler} object used
    to deliver results to the CIM Server.

    @exception CIMNotSupportedException
    @exception CIMInvalidParameterException
    @exception CIMObjectNotFoundException
    @exception CIMAccessDeniedException
    @exception CIMOperationFailedException
    */
    virtual void getInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler) = 0;

    /**
    \Label{enumerateInstances}
    Return all instances of the specified class.

    <p>A typical implementation of this function will call the
    <tt>{@link processing processing}</tt> function in the
    <tt>{@link ResponseHandler handler}</tt> object, then
    iterate over the system resources representing instances of
    the CIM object, calling <tt>{@link deliver deliver}</tt> on
    each iteration. It must call <tt>deliver</tt> with an
    argument of type <tt>{@link CIMInstance CIMInstance}</tt>.
    Finally, it will call <tt>{@link complete complete}</tt> to
    inform the CIM Server that it has delivered all known instances.</p>

    <p>A provider can be implemented and registered to perform
    operations for several levels of the same line of descent (e.g.,
    CIM_ComputerSystem and CIM_UnitaryComputerSystem). When this
    is done, the provider must return instances <i>only</i>
    for the deepest class for which it is registered, since
    the CIM Server will invoke <tt>enumerateInstances</tt> for all
    classes at and beneath that specified in the
    {@link CIMObjectPath classReference}.</p>

    @param context specifies the client user's context for this operation,
    including the User ID.

    @param classReference specifies the fully qualified object path
    to the class of interest.

    @param includeQualifiers indicates whether the returned instances must
    include the qualifiers for the instance and properties.  Qualifiers may
    be included even if this flag is false.

    @param includeClassOrigin indicates whether the returned instances must
    include the class origin for each of the instance elements.

    @param propertyList If not null, this parameter
    specifies the minimum set of properties required in instances returned by this operation. Because
    support for this parameter is optional, the instances may contain additional properties not specified
    in the list. NOTE: The provider does NOT receive the client filtering parameters
	localOnly or deepInheritance.  These are resolved in the CIMOM into the propertyList.

    @param handler {@link ResponseHandler ResponseHandler} object for
    delivery of results.

    @exception CIMNotSupportedException
    @exception CIMInvalidParameterException
    @exception CIMObjectNotFoundException
    <br>should never be thrown by this function; if
    there are no instances to return, this function should deliver an empty
    set of instances by calling the
    handler's <tt>{@link processing processing} </tt> and
    <tt>{@link complete complete}</tt> functions without calling
    <tt>{@link deliver deliver}</tt>.

    @exception CIMAccessDeniedException
    @exception CIMOperationFailedException

    */
    virtual void enumerateInstances(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        const Boolean includeQualifiers,
        const Boolean includeClassOrigin,
        const CIMPropertyList & propertyList,
        InstanceResponseHandler & handler) = 0;

    /**
    \Label{enumerateInstanceNames}
    Return all instance names of a single class.

    <p>Like <tt>enumerateInstances</tt>, a typical implementation
    of <tt>enumerateInstanceNames</tt> will call the
    <tt>{@link processing processing}</tt> function in the
    <tt>{@link ResponseHandler handler}</tt> object, then
    iterate over the system resources representing instances of
    the CIM object, calling <tt>{@link deliver deliver}</tt> on
    each iteration. It must call {@link deliver deliver} with an
    argument of type <tt>{@link CIMObjectPath CIMObjectPath}</tt>
    containing the information that uniquely identifies each
    instance.
    Finally, it will call <tt>{@link complete complete}</tt> to
    inform the CIM Server that it has delivered all known instances.</p>

    <p>A provider can be implemented and registered to perform
    operations for several levels of the same line of descent (e.g.,
    CIM_ComputerSystem and CIM_UnitaryComputerSystem). When this
    is done, the provider must return instance names <i>only</i>
    for the deepest class for which it is registered, since
    the CIM Server will invoke <tt>enumerateInstanceNames</tt> for all
    classes at and beneath that specified in the
    {@link CIMObjectPath classReference}.</p>

    @param context specifies the client user's context for this operation,
    including the User ID.

    @param classReference specifies the fully qualified object path to
    the class of interest.

    @param handler {@link ResponseHandler ResponseHandler} object for
    delivery of results.

    @exception CIMNotSupportedException
    @exception CIMInvalidParameterException
    @exception CIMObjectNotFoundException
    @exception CIMAccessDeniedException
    @exception CIMOperationFailedException
    */
    virtual void enumerateInstanceNames(
        const OperationContext & context,
        const CIMObjectPath & classReference,
        ObjectPathResponseHandler & handler) = 0;

    /**
    \Label{modifyInstance}
    Replace the current instance specified in the
    instanceReference parameter.

    <p><tt>modifyInstance</tt> sets the values of properties of
    the instance specified by the <tt>instanceReference</tt> parameter
    to those specified in the <tt>instanceObject</tt> parameter, as
    controlled by the <tt>propertyList</tt> parameter. If the
    <tt>propertyList</tt>
    is NULL, then the operation sets all properties. Otherwise,
    it sets only those specified in the <tt>propertyList</tt>.
    Properties specified in the <tt>propertyList</tt> but not present in
    the <tt>instanceObject</tt> are replaced by
    the class default values or left null.</p>

    <p>Ideally, <tt>modifyInstance</tt> is intended to be
    an <i>atomic</i> operation on values of the instance. That is,
    concurrent accesses to the instance by other threads should be
    blocked during the operation, so that all of the affected property
    values can be
    changed without intervening accesses by concurrent requests.
    Otherwise, other requests could obtain intermediate, and
    possibly inconsistent, results.</p>

    <p>If the specified instance does not exist, the provider
    should throw an {@link CIMObjectNotFoundException ObjectNotFound}
    exception.

    @param context specifies the client user's context for this operation,
    including the User ID.

    @param instanceReference specifies the fully qualified object path of the instance of interest.

    @param instanceObject contains the partial or complete set of
    properties whose values should be changed.

    @param includeQualifiers indicates whether the instance qualifiers must
    be updated as specified in the modified instance.  If false, no
    qualifiers are explicitly modified by this operation.

    @param propertyList If not null, this parameter
    specifies the set of properties required to be updated in the instance. Support
    for this parameter is NOT optional.  Providers that do not support this
    feature must throw a {@link CIMNotSupportedException CIMNotSupportedException} exception.
	NOTE: The provider does NOT receive the client filtering parameters
	localOnly or deepInheritance.  These are resolved in the CIMOM into the propertyList.
	
    @param handler {@link ResponseHandler ResponseHandler} object for
    delivery of results.

    @exception CIMNotSupportedException
    @exception CIMInvalidParameterException
    @exception CIMObjectNotFoundException
    @exception CIMAccessDeniedException
    @exception CIMOperationFailedException
    */
    virtual void modifyInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        const Boolean includeQualifiers,
        const CIMPropertyList & propertyList,
        ResponseHandler & handler) = 0;

    /**
    \Label{createInstance}
    Create a new instance.

    <p>Create a new instance of the specified class as specified
    by the <tt>instanceReference</tt> and <tt>instanceObject</tt>
    parameters.</p>

    @param context specifies the client user's context for this operation,
    including the User ID.

    @param instanceReference Specifies the namespace and class name
    of the instance to create.  The key bindings are not present in
    the instanceReference, because an instance name is not defined
    until after the instance has been created.

    @param instanceObject contains the partial or complete instance to create.
    If a key property is null, the provider <em>must</em> supply a valid
    value for the property or throw a {@link CIMInvalidParameterException
    CIMInvalidParameterException}.  If any property value is invalid, the
    provider should throw a {@link CIMInvalidParameterException
    CIMInvalidParameterException}.

    @param handler {@link ResponseHandler ResponseHandler} object for
    delivery of results.  If the operation is successful, the provider must
    deliver the complete instance name of the created instance.

    @exception CIMNotSupportedException
    @exception CIMInvalidParameterException
    @exception CIMObjectAlreadyExistsException
    @exception CIMAccessDeniedException
    @exception CIMOperationFailedException
    */
    virtual void createInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        const CIMInstance & instanceObject,
        ObjectPathResponseHandler & handler) = 0;

    /**
    \Label{deleteInstance}
    Delete the instance specified by the instanceReference parameter.

    @param context specifies the client user's context for this operation,
    including the User ID.

    @param instanceReference specifies the fully qualified object
    path of the instance to delete. If the specified object does
    not exist, the provider should throw an
    {@link CIMObjectNotFoundException ObjectNotFound} exception.

    @param handler {@link ResponseHandler ResponseHandler} object for
    delivery of results.

    @exception CIMNotSupportedException
    @exception CIMInvalidParameterException
    @exception CIMObjectNotFoundException
    @exception CIMAccessDeniedException
    @exception CIMOperationFailedException
    */
    virtual void deleteInstance(
        const OperationContext & context,
        const CIMObjectPath & instanceReference,
        ResponseHandler & handler) = 0;
};

PEGASUS_NAMESPACE_END

#endif
