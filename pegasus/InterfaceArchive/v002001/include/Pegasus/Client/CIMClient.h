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


#ifndef Pegasus_Client_h
#define Pegasus_Client_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/SSLContext.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CIMPropertyList.h>
#include <Pegasus/Common/CIMQualifierDecl.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Client/CIMClientException.h>
#include <Pegasus/Client/Linkage.h>

PEGASUS_NAMESPACE_BEGIN


class CIMClientRep;

/** This class provides the interface that a client uses to communicate
    with a CIM Server.
*/
class PEGASUS_CLIENT_LINKAGE CIMClient
{
public:

    /** Constructor for a CIM Client object.
    */
    CIMClient();

    ///
    ~CIMClient();

    ///
    Uint32 getTimeout() const;

    /** Sets the timeout in milliseconds for the CIMClient.
        @param timeoutMilliseconds Defines the number of milliseconds the
        CIMClient will wait for a response to an outstanding request.  If a
        request times out, the connection gets reset (disconnected and
        reconnected).  Default is 20 seconds (20000 milliseconds).
    */
    void setTimeout(Uint32 timeoutMilliseconds);

    /** Creates an HTTP connection with the server
        defined by the host and portNumber.
        @param host String defining the server to which the client should
        connect.
        @param portNumber Uint32 defining the port number for the server
        to which the client should connect.
        @param userName String containing the name of the user
        the client is connecting as.
        @param password String containing the password of the user
        the client is connecting as.
        @exception AlreadyConnectedException
            If a connection has already been established.
        @exception InvalidLocatorException
            If the specified address is improperly formed.
        @exception CannotCreateSocketException
            If a socket cannot be created.
        @exception CannotConnectException
            If the socket connection fails.
        @exception CIMClientConnectionException
            If any other failure occurs.
        <PRE>
            CIMClient client;
            client.connect("localhost", 5988, "guest", "guest");
        </PRE>
    */
    void connect(
        const String& host,
        const Uint32 portNumber,
        const String& userName,
        const String& password
    );

    /** Creates an HTTP connection with the server
        defined by the URL in address.
        @param host String defining the server to which the client should
        connect
        @param portNumber Uint32 defining the port number for the server
        to which the client should connect
        @param sslContext The SSL context to use for this connection
        @param userName String containing the name of the user
        the client is connecting as.
        @param password String containing the password of the user
        the client is connecting as.
        @exception AlreadyConnectedException
            If a connection has already been established.
        @exception InvalidLocatorException
            If the specified address is improperly formed.
        @exception CannotCreateSocketException
            If a socket cannot be created.
        @exception CannotConnectException
            If the socket connection fails.
        @exception CIMClientConnectionException
            If any other failure occurs.
    */
    void connect(
        const String& host,
        const Uint32 portNumber,
        const SSLContext& sslContext,
        const String& userName,
        const String& password
    );

    /** Creates connection to the server for
        Local clients. The connectLocal connects to the CIM server
        running on the local system in the default location.  The
        connection is automatically authenticated for the current
        user.
        @See connect - The exceptions are defined in connect.
    */
    void connectLocal();

    /** Closes the connection with the server if the connection
        was open, simply returns if the connection was not open. Clients are
        expected to use this method to close the open connection before
        opening a new connection.
    */
    void disconnect();


    /** The <TT>getClass</TT> method executes a CIM operation that returns
	a single CIM Class from the
	target Namespace where the ClassName input parameter defines the name of
	the class to be retrieved.

	@param nameSpace The <TT>nameSpace</TT> parameter is a string that
	defines the target Namespace.
	See definition of \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

	@param className The <TT>className</TT> input parameter defines the name
	of the Class to be retrieved.

	@param localOnly If the <TT>localOnly</TT> input parameter is true, this
	specifies that only CIM Elements (properties, methods and qualifiers)
	overridden within the definition of the Class are returned.  If false,
	all elements are returned.  This parameter therefore effects a CIM
	Server-side mechanism to filter certain elements of the returned object
	based on whether or not they have been propagated from the parent Class
	(as defined by the PROPAGATED attribute).

	@param includeQualifiers If the <TT>includeQualifiers</TT> input
	parameter is true, this specifies that all Qualifiers for that Class
	(including Qualifiers on the Class and on any returned Properties,
	Methods or CIMMethod Parameters) MUST be included as <QUALIFIER>
	elements in the response.  If false no <QUALIFIER> elements are
	present in the returned Class.

	@param includeClassOrigin If the <TT>includeClassOrigin</TT> input
	parameter is true, this specifies that the CLASSORIGIN attribute MUST be
	present on all appropriate elements in the returned Class. If false, no
	CLASSORIGIN attributes are present in the returned Class.

	@param propertyList If the <TT>propertyList</TT> input parameter is not
	NULL, the members of the array define one or more CIMProperty names. The
	returned Class MUST NOT include elements for any Properties missing from
	this list. Note that if LocalOnly is specified as true this acts as an
	additional filter on the set of Properties returned (for example, if
	CIMProperty A is included in the PropertyList but LocalOnly is set to
	true and A is not local to the requested Class, then it will not be
	included in the response). If the PropertyList input parameter is an
	empty array this signifies that no Properties are included in the
	response. If the PropertyList input parameter is NULL this specifies
	that all Properties (subject to the conditions expressed by the other
	parameters) are included in the response.

	If the <TT>propertyList</TT> contains duplicate elements, the Server
	MUST ignore the duplicates but otherwise process the request normally.
	If the PropertyList contains elements which are invalid CIMProperty
	names for the target Class, the Server MUST ignore such entries but
	otherwise process the request normally.

	@return If successful, the return value is a single CIM Class.

	If unsuccessful, one of the following status codes MUST be returned by
	this method, where the first applicable error in the list (starting with
	the first element of the list, and working down) is the error returned.
	Any additional method-specific interpretation of the error is given
	in parentheses.
	<UL>
		<LI>CIM_ERR_ACCESS_DENIED
		<LI>CIM_ERR_INVALID_NAMESPACE
		<LI>CIM_ERR_INVALID_PARAMETER (including missing,
		duplicate,unrecognized or otherwise incorrect parameters)
		<LI>CIM_ERR_NOT_FOUND (the request CIM Class does not exist in
		the specified namespace)
		<LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
	</UL>
    */
    CIMClass getClass(
	const CIMNamespaceName& nameSpace,
	const CIMName& className,
	Boolean localOnly = true,
	Boolean includeQualifiers = true,
	Boolean includeClassOrigin = false,
	const CIMPropertyList& propertyList = CIMPropertyList()
    );

    ///
    CIMInstance getInstance(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& instanceName,
	Boolean localOnly = true,
	Boolean includeQualifiers = false,
	Boolean includeClassOrigin = false,
	const CIMPropertyList& propertyList = CIMPropertyList()
    );

    ///
    void deleteClass(
	const CIMNamespaceName& nameSpace,
	const CIMName& className
    );

    ///
    void deleteInstance(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& instanceName
    );

    ///
    void createClass(
	const CIMNamespaceName& nameSpace,
	const CIMClass& newClass
    );

    ///
    CIMObjectPath createInstance(
	const CIMNamespaceName& nameSpace,
	const CIMInstance& newInstance
    );

    ///
    void modifyClass(
	const CIMNamespaceName& nameSpace,
	const CIMClass& modifiedClass
    );

    ///
    void modifyInstance(
	const CIMNamespaceName& nameSpace,
	const CIMInstance& modifiedInstance,
	Boolean includeQualifiers = true,
	const CIMPropertyList& propertyList = CIMPropertyList()
    );

    ///
    Array<CIMClass> enumerateClasses(
	const CIMNamespaceName& nameSpace,
	const CIMName& className = CIMName(),
	Boolean deepInheritance = false,
	Boolean localOnly = true,
	Boolean includeQualifiers = true,
	Boolean includeClassOrigin = false
    );

    ///
    Array<CIMName> enumerateClassNames(
	const CIMNamespaceName& nameSpace,
	const CIMName& className = CIMName(),
	Boolean deepInheritance = false
    );

    ///
    Array<CIMInstance> enumerateInstances(
	const CIMNamespaceName& nameSpace,
	const CIMName& className,
	Boolean deepInheritance = true,
	Boolean localOnly = true,
	Boolean includeQualifiers = false,
	Boolean includeClassOrigin = false,
	const CIMPropertyList& propertyList = CIMPropertyList()
    );

    ///
    Array<CIMObjectPath> enumerateInstanceNames(
	const CIMNamespaceName& nameSpace,
	const CIMName& className
    );

    ///
    Array<CIMObject> execQuery(
	const CIMNamespaceName& nameSpace,
	const String& queryLanguage,
	const String& query
    );

    ///
    Array<CIMObject> associators(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& objectName,
	const CIMName& assocClass = CIMName(),
	const CIMName& resultClass = CIMName(),
	const String& role = String::EMPTY,
	const String& resultRole = String::EMPTY,
	Boolean includeQualifiers = false,
	Boolean includeClassOrigin = false,
	const CIMPropertyList& propertyList = CIMPropertyList()
    );

    ///
    Array<CIMObjectPath> associatorNames(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& objectName,
	const CIMName& assocClass = CIMName(),
	const CIMName& resultClass = CIMName(),
	const String& role = String::EMPTY,
	const String& resultRole = String::EMPTY
    );

    ///
    Array<CIMObject> references(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& objectName,
	const CIMName& resultClass = CIMName(),
	const String& role = String::EMPTY,
	Boolean includeQualifiers = false,
	Boolean includeClassOrigin = false,
	const CIMPropertyList& propertyList = CIMPropertyList()
    );

    ///
    Array<CIMObjectPath> referenceNames(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& objectName,
	const CIMName& resultClass = CIMName(),
	const String& role = String::EMPTY
    );

    ///
    CIMValue getProperty(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& instanceName,
	const CIMName& propertyName
    );

    ////
    void setProperty(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& instanceName,
	const CIMName& propertyName,
	const CIMValue& newValue = CIMValue()
    );

    ///
    CIMQualifierDecl getQualifier(
	const CIMNamespaceName& nameSpace,
	const CIMName& qualifierName
    );

    ///
    void setQualifier(
	const CIMNamespaceName& nameSpace,
	const CIMQualifierDecl& qualifierDeclaration
    );

    ///
    void deleteQualifier(
	const CIMNamespaceName& nameSpace,
	const CIMName& qualifierName
    );

    ///
    Array<CIMQualifierDecl> enumerateQualifiers(
	const CIMNamespaceName& nameSpace
    );

    ///
    CIMValue invokeMethod(
	const CIMNamespaceName& nameSpace,
	const CIMObjectPath& instanceName,
	const CIMName& methodName,
	const Array<CIMParamValue>& inParameters,
	Array<CIMParamValue>& outParameters
    );

private:

    CIMClientRep* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Client_h */
