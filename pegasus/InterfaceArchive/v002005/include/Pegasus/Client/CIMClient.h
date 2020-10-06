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
#include <Pegasus/Common/AcceptLanguageList.h>
#include <Pegasus/Common/ContentLanguageList.h>
#include <Pegasus/Client/ClientOpPerformanceDataHandler.h>


PEGASUS_NAMESPACE_BEGIN

class CIMClientInterface;

/**
    The CIMClient class provides an interface for a client application
    to communicate with a CIM Server.  The client application specifies
    the target CIM Server by providing connection parameters and
    authentication credentials (as necessary).

    The CIMClient class models the functionality defined in the DMTF's
    Specification for CIM Operations over HTTP version 1.1, using similar
    operations and parameters.
*/
class PEGASUS_CLIENT_LINKAGE CIMClient
{
public:

    /**
        Constructs a CIMClient object.
    */
    CIMClient();

    /**
        Destructs a CIMClient object.
    */
    ~CIMClient();

    /**
        Gets the currently configured timeout value for the CIMClient object.
        @return An integer indicating the currently configured timeout
        value (in milliseconds).
    */
    Uint32 getTimeout() const;

    /**
        Sets the timeout value for CIMClient operations.  If an operation
        response is not received within this timeout value, a
        ConnectionTimeoutException is thrown and the connection is reset.
        The default timeout value is 20 seconds (20000 milliseconds).
        @param timeoutMilliseconds An integer specifying the timeout value
        (in milliseconds).
    */
    void setTimeout(Uint32 timeoutMilliseconds);

    /** Connects to the CIM Server at the specified host name and port number.
        Example usage:
        <PRE>
            CIMClient client;
            String host("localhost");
            try
            {
                client.connect(host, 5988, "guest", "guest");
            }
            catch(Exception& e)
            {
                cerr << "Pegasus Exception: " << e.getMessage() <<
                    ". Trying to connect to " << host << endl;
            }
        </PRE>
        @param host A String host name to connect to.
        @param portNumber A Uint32 port number to connect to.
        @param userName A String specifying the user name for the connection.
        @param password A String containing the password of the specified user.
        @exception AlreadyConnectedException
            If the CIMClient is already connected to a CIM Server.
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
        const String& userName,
        const String& password
    );

    /** Connects to the CIM Server at the specified host name, port number
        and SSL context.
        @param host A String host name to connect to.
        @param portNumber A Uint32 port number to connect to.
        @param sslContext The SSL context to use for this connection.
        @param userName A String specifying the user name for the connection.
        @param password A String containing the password of the specified user.
        @exception AlreadyConnectedException
            If the CIMClient is already connected to a CIM Server.
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

    /** Connects to the local CIM Server as the current user.  Authentication
        is performed automatically, so no credentials are required for this
        connection.
        @exception AlreadyConnectedException
            If the CIMClient is already connected to a CIM Server.
        @exception CannotCreateSocketException
            If a socket cannot be created.
        @exception CannotConnectException
            If the socket connection fails.
        @exception CIMClientConnectionException
            If any other failure occurs.
    */
    void connectLocal();

    /**
        Disconnects from the CIM Server.  If no connection is established,
        this method has no effect.  A CIMClient with an existing connection
        must be disconnected before establishing a new connection.
    */
    void disconnect();

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
        Configures the accept languages to be specified on subsequent
        requests from this client.  Accept languages are the preferred
        languages for response data.
        @param langs An AcceptLanguageList object specifying the languages
        preferred by this client.
    */
    void setRequestAcceptLanguages(const AcceptLanguageList& langs);

    /** <I><B>Experimental Interface</B></I><BR>
        Gets the accept languages currently configured for this client.
        Accept languages are the preferred languages for response data.
        @return An AcceptLanguageList object specifying the preferred languages
        configured for this client.
    */
    AcceptLanguageList getRequestAcceptLanguages() const;

    /** <I><B>Experimental Interface</B></I><BR>
        Configures the content languages to be specified on subsequent
        requests from this client.  The content languages indicate the
        languages associated with request data sent from this client.
        @param langs A ContentLanguageList object specifying the languages
        associated with this client's request data.
    */
    void setRequestContentLanguages(const ContentLanguageList& langs);

    /** <I><B>Experimental Interface</B></I><BR>
        Gets the content languages currently configured for this client.
        The content languages indicate the languages associated with request
        data sent from this client.
        @return A ContentLanguageList object specifying the languages used in
        request data from this client.
    */
    ContentLanguageList getRequestContentLanguages() const;

    /** <I><B>Experimental Interface</B></I><BR>
        Gets the content languages of the last response received by this
        client.  The content languages indicate the languages associated
        with the data included in the response.
        @return A ContentLanguageList object specifying the languages used in
        the last response received by this client.
    */
    ContentLanguageList getResponseContentLanguages() const;

    /** <I><B>Experimental Interface</B></I><BR>
    */
    void setRequestDefaultLanguages();
#endif // PEGASUS_USE_EXPERIMENTAL_INTERFACES


    /** The <TT>getClass</TT> method executes a CIM operation that returns
        a single CIM Class from the target Namespace where the ClassName input 
        parameter defines the name of the class to be retrieved.  
    
        @param nameSpace (Required) The <TT>nameSpace</TT> parameter is a CIMName 
        object that defines the target Namespace.  See definition of 
        \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.  
    
        @param className (Required)The <TT>className</TT> input parameter is a CIMName 
        object that defines the name of the Class to be retrieved.  
    
        @param localOnly (Boolean, Optional, default = true, Input)  If the <TT>localOnly</TT> input 
        parameter is true, this specifies that only CIM Elements (properties, 
        methods and qualifiers) overridden within the definition of the Class are 
        returned.  If false, all elements are returned.  This parameter therefore 
        effects a CIM Server-side mechanism to filter certain elements of the 
        returned object based on whether or not they have been propagated from the 
        parent Class (as defined by the PROPAGATED attribute).  
    
        @param includeQualifiers (Boolean, Optional, default = true, Input) If the 
        <TT>includeQualifiers</TT> input parameter is true, this specifies that 
        all Qualifiers for that Class (including Qualifiers on the Class and on 
        any returned Properties, Methods or CIMMethod Parameters) MUST be included 
        as elements in the response.  If false no QUALIFIER elements are present 
        in the returned Class object.  
    
        @param includeClassOrigin (Boolean,Optional, default = false, Input) If 
        the <TT>includeClassOrigin</TT> input parameter is true, this specifies 
        that the CLASSORIGIN attribute MUST be present on all appropriate elements 
        in the returned Class.  If false, no CLASSORIGIN attributes are present in 
        the returned Class.  
    
        @param propertyList (optional, Input) If the <TT>propertyList</TT> 
        CIMPropertyList input parameter is not NULL, the members of the array 
        define one or more CIMProperty names.  The returned Class WILL NOT include 
        elements for any Properties missing from this list.  Note that if 
        LocalOnly is specified as true this acts as an additional filter on the 
        set of Properties returned (for example, if CIMProperty A is included in 
        the PropertyList but LocalOnly is set to true and A is not local to the 
        requested Class, then it will not be included in the response).  If the 
        PropertyList input parameter is an empty array this signifies that no 
        Properties are included in the response.  If the PropertyList input 
        parameter is NULL this specifies that all Properties (subject to the 
        conditions expressed by the other parameters) are included in the 
        response.  
        @see CIMPropertyList
    
        If the <TT>propertyList</TT> contains duplicate elements, the Server
        MUST ignore the duplicates but otherwise process the request normally.
        If the PropertyList contains elements which are invalid CIMProperty
        names for the target Class, the Server MUST ignore such entries but
        otherwise process the request normally.
    
        @return If successful, the return value is a single CIMClass objcet.
    
        If unsuccessful, an exception is executed. If the error is local, this may
        be any local exception.
        If the error is in the host normally a CIMException is returned with one of the
        following CIMException codes, where the first applicable error in the list (starting with
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
        @exceptions REVIEWERS: Need to complete this definition
        </UL>
        <pre>
            ... Connect sequence.
            CIMNamespace("root/cimv2);
            Boolean localOnly = true;
            Boolean includQualifiers = true;
            Boolean includeClassOrigin = false;
            CIMPropertyList propertyList;      // empty property list
            try
            CIMException checkClassException;
            { 
            CIMClass cimClass = client.getClass(nameSpace,
                                        className,
                                        localOnly,
                                        includeQualifiers,
                                        includeClassOrigin,
                                        propertyList);
            }
            catch(CIMException& e)
            {
                if (checkClassException.getCode() = CIM_ERR_NOT_FOUND)
                    cout << "Class " << className << " not found." << endl;
                ...
            }
            catch(Exception& e)
            {
            }
            ...
        
            // An alternate call with the default parameters would be:
            // This uses the defaults localOnly = includeQualifiers = true
            // includeClassOrigin = false. propertyList = Null;
            
            CIMClass cimClass = client.getClass(nameSpace, className);
        </pre>
        @exception REVIEWERS: Complete this
        @see CIMExcetpion
    */
    CIMClass getClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    /** Gets the CIM instance for the specified CIM object path.
        
        @param nameSpace (Required) The <TT>nameSpace</TT> parameter is a CIMName 
        object that defines the target Namespace.  See definition of 
        \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.  
        
        @param instanceName CIMobjectpath that identifies this CIM instance. 
        This must include all of the keys. 

        @param localOnly If true, only properties and qualifiers overridden 
        or defined in the returned Instance are included in the response. 
        If false, all elements of the returned Instance are returned. 

        @param includeQualifiers If true, all Qualifiers for each Object 
        (including Qualifiers on the Object and on any returned Properties) 
        MUST be included. If false no Qualifiers are present in the returned Object. 

        @param includeClassOrigin If true, CLASSORIGIN attribute MUST be 
        present on all appropriate elements in each returned Object. If false, 
        no CLASSORIGIN attributes are present in each returned Object. The CLASSORIGIN
        attribute is defined in the DMTF's Specification for the Representation of CIM
        in XML. CLASSORIGIN is an XML tag identifying the following text as a class name.
        It is attached to a property or method (when specified in XML), to indicate the
        class where that property or method is first defined. Where the same property
        name is locally defined in another superclass or subclass, the Server will
        return the value for the property in the lowest subclass. 

        @param propertyList If the PropertyList input parameter is not NULL, the
        members of the array define one or more Property names. Each returned Object
        MUST NOT include elements for any Properties missing from this list.
        Note that if LocalOnly is specified as true this acts as an additional
        filter on the set of Properties returned (for example, if Property A is
        included in the PropertyList but LocalOnly is set to true and A is not local
        to a returned Instance, then it will not be included in that Instance).
        If the PropertyList input parameter is an empty array this signifies that
        no Properties are included in each returned Object. If the PropertyList
        input parameter is NULL this specifies that all Properties (subject to
        the conditions expressed by the other parameters) are included in each
        returned Object. If the PropertyList contains duplicate elements, the
        Server ignores the duplicates but otherwise process the request normally.
        If the PropertyList contains elements which are invalid Property names for
        any target Object, the Server ignores such entries but otherwise process
        the request normally. 

        @return If successful, the CIM instance identified by the CIMObjectPath. If 
        unsuccessful, an exception is executed.
        REVIEWERS: COmplete this.
    */

    CIMInstance getInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    /** The <TT>DeleteClass</TT> method deletes a single CIM Class from the
        target Namespace.

        @param nameSpace The nameSpace parameter is a CIMName that defines
        the target namespace.  See defintion of
        \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

        @param className The <TT>className</TT> input parameter defines the name
        of the Class to be deleted.

        @return If successful, the specified Class (including any subclasses
        and any instances) MUST have been removed by the CIM Server.  The
        operation MUST fail if any one of these objects cannot be deleted.

        If unsuccessful, one of the following status codes MUST be returned by
        this method, where the first applicable error in the list (starting
        with the first element of the list, and working down) is the error
        returned. Any additional method-specific interpretation of the error
        in is given in parentheses.

        <UL>
            <LI>CIM_ERR_ACCESS_DENIED
            <LI>CIM_ERR_NOT_SUPPORTED
            <LI>CIM_ERR_INVALID_NAMESPACE
            <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
                unrecognized or otherwise incorrect parameters)
            <LI>CIM_ERR_NOT_FOUND (the CIM Class to be deleted does not exist)
            <LI>CIM_ERR_CLASS_HAS_CHILDREN (the CIM Class has one or more
                subclasses which cannot be deleted)
            <LI>CIM_ERR_CLASS_HAS_INSTANCES (the CIM Class has one or more
                instances which cannot be deleted)
            <LI>CIM_ERR_FAILED (some other unspecified error occurred)
        </UL>
    */
    void deleteClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className
    );

    /** The <TT>DeleteInstance</TT> operation deletes a single CIM Instance
        from the target Namespace.

        @param nameSpace The nameSpace parameter is a string that defines
        the target namespace.  See defintion of
        \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

        @param instanceName The <TT>instanceName</TT> input parameter defines
        the name (model path) of the Instance to be deleted.

        @return If successful, the specified Instance MUST have been removed
        by the CIM Server.

        If unsuccessful, one of the following status codes MUST be returned by
        this method, where the first applicable error in the list (starting
        with the first element of the list, and working down) is the error
        returned. Any additional method-specific interpretation of the error in
        is given in parentheses.

        <UL>
            <LI>CIM_ERR_ACCESS_DENIED
            <LI>CIM_ERR_NOT_SUPPORTED
            <LI>CIM_ERR_INVALID_NAMESPACE
            <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
                unrecognized or otherwise incorrect parameters)
            <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the
                specified namespace)
            <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested
                CIM Instance does not exist in the specified namespace)
            <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
        </UL>
    */
    void deleteInstance(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName
    );

    /** The <TT>createClass</TT> method creates a single CIM Class in
    the target Namespace. The Class MUST NOT already exist. The NewClass input
    parameter defines the new Class.  The proposed definition MUST be a correct
    Class definition according to the CIM specification.

    In processing the creation of the new Class, the following rules MUST be
    conformed to by the CIM Server:

    Any CLASSORIGIN and PROPAGATED attributes in the NewClass MUST be ignored by
    the Server. If the new Class has no Superclass, the NewClass parameter
    defines a new base Class. The Server MUST ensure that all Properties and
    Methods of the new Class have a CLASSORIGIN attribute whose value is the
    name of the new Class. If the new Class has a Superclass, the NewClass
    parameter defines a new Subclass of that Superclass. The Superclass MUST
    exist. The Server MUST ensure that:

    <UL>
        <LI>Any Properties, Methods or Qualifiers in the Subclass not defined in
        the Superclass are created as new elements of the Subclass. In
        particular the Server MUST set the CLASSORIGIN attribute on the new
        Properties and Methods to the name of the Subclass, and ensure that all
        other Properties and Methods preserve their CLASSORIGIN attribute value
        from that defined in the Superclass

        If a CIMProperty is defined in the Superclass and in the Subclass, the
        value assigned to that property in the Subclass (including NULL) becomes
        the default value of the property for the Subclass. If a CIMProperty or
        CIMMethod of the Superclass is not specified in the Subclass, then that
        CIMProperty or CIMMethod is inherited without modification by the
        Subclass

        <LI>Any Qualifiers defined in the Superclass with a TOSUBCLASS attribute
        value of true MUST appear in the resulting Subclass. Qualifiers in the
        Superclass with a TOSUBCLASS attribute value of false MUST NOT be
        propagated to the Subclass . Any CIMQualifier propagated from the
        Superclass cannot be modified in the Subclass if the OVERRIDABLE
        attribute of that CIMQualifier was set to false in the Superclass. It is
        a
        Client error to specify such a CIMQualifier in the NewClass with a
        different definition to that in the Superclass (where definition
        encompasses the name, type and flavor attribute settings of the
        <QUALIFIER> element, and the value of the CIMQualifier).
        </LI>
    </UL>

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param newClass The <TT>newClass<?TT> input parameter defines the new Class.

    @return If successful, the specified Class MUST have been created by the CIM
    Server.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
         <LI>CIM_ERR_ACCESS_DENIED
         <LI>CIM_ERR_NOT_SUPPORTED
         <LI>CIM_ERR_INVALID_NAMESPACE
         <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
         unrecognized or otherwise incorrect parameters)
         <LI>CIM_ERR_ALREADY_EXISTS (the CIM Class already exists)
         <LI>CIM_ERR_INVALID_SUPERCLASS (the putative CIM Class declares a
         non-existent superclass)
         <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    void createClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& newClass
    );

    /** The <TT>createInstance</TT> method creates a single CIM
    Instance in the target Namespace. The Instance MUST NOT already exist.

    In processing the creation of the new Instance, the following rules MUST be
    conformed to by the CIM Server:

    Any CLASSORIGIN and PROPAGATED attributes in the NewInstance MUST be ignored
    by the Server.

    The Server MUST ensure that:

    <UL>
        <LI>Any Qualifiers in the Instance not defined in the Class are created
        as new elements of the Instance.
        <LI>All Properties of the Instance preserve their CLASSORIGIN attribute
        value from that defined in the Class.
        <LI>If a CIMProperty is specified in the ModifiedInstance parameter, the
        value assigned to that property in the Instance (including NULL) becomes
        the value of the property for the Instance. Note that it is a Client
        error to specify a CIMProperty that does not belong to the Class.
        <LI>If a CIMProperty of the Class is not specified in the Instance, then
        that CIMProperty is inherited without modification by the Instance.
        <LI>Any Qualifiers defined in the Class with a TOINSTANCE attribute
        value of true appear in the Instance. Qualifiers in the
        Class with a TOINSTANCE attribute value of false MUST NOT be propagated
        to the Instance.
        <LI>Any CIMQualifier propagated from the Class cannot be modified in the
        Instance if the OVERRIDABLE attribute of that CIMQualifier was set to
        false
        in the Class. It is a Client error to specify such a CIMQualifier in the
        NewInstance with a different definition to that in the Class (where
        definition encompasses the name, type and flavor attribute settings of
        the <QUALIFIER> element, and the value of the CIMQualifier).
    </UL>

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param newInstance The <TT>newInstance</TT> input parameter defines the new
    Instance. The proposed definition MUST be a correct Instance definition for
    the underlying CIM Class according to the CIM specification.

    @return If successful, the return value defines the object path of the new
    CIM Instance relative to the target Namespace (i.e. the Model Path as
    defined by the CIM specification), created by the CIM Server.  It is
    returned in case one or more of the new keys of the Instance are allocated
    dynamically during the creation process rather than specified in the
    request.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
        <LI>CIM_ERR_ACCESS_DENIED
        <LI>CIM_ERR_NOT_SUPPORTED
        <LI>CIM_ERR_INVALID_NAMESPACE
        <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
            unrecognized or otherwise incorrect parameters)
        <LI>CIM_ERR_INVALID_CLASS (the CIM Class of which this is to be a new
            Instance does not exist)
        <LI>CIM_ERR_ALREADY_EXISTS (the CIM Instance already exists)
        <LI>CIM_ERR_FAILED (some other unspecified error occurred)
    </UL>
    */
    CIMObjectPath createInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& newInstance
    );

    /** The <TT>modifyClass</TT> method modifies an existing CIM Class in the
    target Namespace.

    The Class MUST already exist. The <TT>ModifiedClass</TT>
    input parameter defines the set of changes (which MUST be  correct
    amendments to the CIM Class as defined by the CIM Specification) to be made
    to the current class definition.

    In processing the  modifcation of the Class, the following rules MUST be
    conformed to by the CIM Server.

    <UL>
      <LI>Any <TT>CLASSORIGIN</TT> and <TT>PROPAGATED</TT> attributes in the
      <TT>ModifiedClass</TT> MUST be ignored by the Server.
      <LI>If the  modified Class has no Superclass,
      the<TT>ModifiedClass</TT> parameter defines modifications to a base Class.
      The Server MUST ensure that:
      <UL>
        <LI>All Properties and Methods of the modified Class have a
        <TT>CLASSORIGIN</TT> attribute whose value is the name of this Class.
        <LI>Any Properties, Methods or Qualifiers in the existing Class
        definition which do not appear in the   <FONT face="Courier
        New">ModifiedClass</TT> parameter are removed from the resulting
        modified Class.</LI>
      </UL>
      <LI>If the  modified Class has a Superclass,the <TT>ModifiedClass</TT>
      parameter defines modifications to a Subclass of that Superclass. The
      Superclass MUST exist, and the Client MUST NOT change the name of the
      Superclass in the modified Subclass. The Server MUST ensure that:
      <UL>
        <LI>Any Properties, Methods or Qualifiers in the Subclass not
        defined in the Superclass are created as elements of the Subclass. In
        particular the Server MUST set the <TT>CLASSORIGIN</TT> attribute on the
        new Properties and Methods to the name of the Subclass, and MUST ensure
        that all other Properties and Methods preserve their
        <TT>CLASSORIGIN</TT> attribute value from that defined in the
        Superclass.
        <LI>Any CIMProperty, CIMMethod or CIMQualifier previously defined in the
        Subclass
        but not defined in the Superclass, and which is not present in the
        <TT>ModifiedClass</TT> parameter, is removed from the Subclass.
        <LI>If a CIMProperty is specified in the <TT>ModifiedClass</TT>
        parameter, the value assigned to that property therein (including
        NULL) becomes the default value of the property for the Subclass.
        <LI>If a CIMProperty or CIMMethod of the Superclass is not specified in
        the
        Subclass, then that CIMProperty or CIMMethod is inherited
        without modification by the Subclass (so that any previous changes to
        such an Element in the Subclass are lost).
        <LI>If a CIMQualifier in the Superclass is not specified in the
        Subclass, and the CIMQualifier is defined in the Superclass with a
        <TT>TOSUBCLASS</TT> attribute value of <TT>true</TT>, then the
        CIMQualifier
        MUST still be present in the resulting modified Subclass (it is not
        possible to remove a propagated CIMQualifier from a Subclass).
        <LI>Any CIMQualifier propagated from the Superclass cannot be
        modified in the Subclass if the <TT>OVERRIDABLE</TT> attribute of
        that CIMQualifier was set to <TT>false</TT> in the Superclass. It is a
        Client error to specify such a CIMQualifier in the
        <TT>ModifiedClass</TT>
        with a different definition to that in the Superclass (where definition
        encompasses the name, type and flavor attribute settings of the
        <TT>&lt;QUALIFIER&gt;</TT> element, and the value of the CIMQualifier).
        <LI>Any Qualifiers defined in the Superclass with a <TT>TOSUBCLASS</TT>
        attribute value of  <TT>false</TT> MUST NOT be propagated to the
        Subclass.</LI> </UL>
       </LI></UL>

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param modifiedClass The <TT>modifiedClass</TT>
    input parameter defines the set of changes (which MUST be correct
    amendments to the CIM Class as defined by the CIM Specification) to be made
    to the current class definition.

    @return If successful, the specified Class MUST have been updated by
    the CIM Server.

    The request to modify the Class MUST fail if the Server cannot update any
    existing Subclasses or Instances of that Class in a consistent manner.

    @return If unsuccessful, one of the following status codes MUST be
    returned by this method, where the first applicable error in the list
    (starting with the first element of the list, and working down) is the
    error returned. Any additional method-specific interpretation of the error
    in is given in parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_NOT_FOUND (the CIM Class does not
        exist)
      <LI>CIM_ERR_INVALID_SUPERCLASS (the putative CIM Class
        declares a non-existent or incorrect superclass)
      <LI>CIM_ERR_CLASS_HAS_CHILDREN (the modification could
        not be performed because it was not possible to update the subclasses of
        the Class in a consistent fashion)
      <LI>CIM_ERR_CLASS_HAS_INSTANCES (the modification could
        not be performed because it was not possible to update
        the instances of the Class in a consistent fashion)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
     </LI></UL>
    */
    void modifyClass(
        const CIMNamespaceName& nameSpace,
        const CIMClass& modifiedClass
    );

    /** The <TT>modifyInstance</TT> method modifies an existing CIM
    Instance in the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param modifiedInstance The <TT>modifiedInstance</TT> input parameter
    identifies the name of the Instance to be modified, and defines the set of
    changes (which MUST be correct amendments to the Instance as
    defined by the CIM Specification) to be made to the current Instance
    definition.

    In processing the modifcation of the Instance, the following rules MUST
    be conformed to by the CIM Server:

    <UL>
      <LI>Any <TT>CLASSORIGIN</TT> and <TT>PROPAGATED</TT> attributes in the
      <TT>ModifiedInstance</TT> MUST be ignored by the Server.
      <LI>The Class MUST exist, and the Client MUST NOT change
      the name of the Class in the modified Instance. The Server MUST ensure
      that:
      <UL>
        <LI>Any Qualifiers in the Instance not defined in
        the Class are created as new elements of the Instance.
        <LI>All Properties of the Instance preserve their
        <TT>CLASSORIGIN</TT> attribute value from that defined in the Class.
        <LI>Any CIMQualifier previously defined in the Instance but not
        defined in the Class, and which is not present in the
        <TT>ModifiedInstance</TT> parameter, is removed from the Instance.

        <LI>If a CIMProperty is specified in the <TT>ModifiedInstance</TT>
        parameter, the value assigned to that property therein
        (including NULL) becomes the value of the property for the Instance.
        Note that it is a Client error to specify a CIMProperty that does not
        belong to the Class.

        <LI>If a CIMProperty of the Class is not specified in the Instance,
        then that CIMProperty is inherited without modification by the Instance
        (so that any previous changes to that CIMProperty in the Instance are
        lost).
        <LI>Any Qualifiers defined in the Class with a <TT>TOINSTANCE</TT>
        attribute value of <TT>true</TT> appear in the Instance (it is not
        possible remove a propagated CIMQualifier from an Instance. Qualifiers
        in the Class with a <TT>TOINSTANCE</TT> attribute value of <TT>false</TT>
        MUST NOT be propagated to the Instance.
        <LI>Any CIMQualifier propagated from the Class cannot be modified by the
        Server if the <TT>OVERRIDABLE</TT> attribute of that CIMQualifier was
        set to <TT>false</TT> in the Class. It is a Client error to specify such
        a CIMQualifier in the <TT>ModifiedInstance</TT> with a different
        definition to that in the Class (where definition encompasses the name,
        type and flavor attribute settings of the
        <TT>&lt;QUALIFIER&gt;</TT> element, and the value of the CIMQualifier).
        <LI>Any CIMQualifier propagated from the Class cannot be modified in
        the Instance if the <TT>OVERRIDABLE</TT> attribute of that CIMQualifier
        was
        set to <TT>false</TT> in the Class. It is a Client error to specify such
        a CIMQualifier in the <TT>ModifiedInstance</TT> with a different
        definition
        to that in the Class (where definition encompasses the name, type and
        flavor attribute settings of the <TT>&lt;QUALIFIER&gt;</TT>
        element, and the value of the CIMQualifier).</LI>
        </UL>
      </LI></UL>

    @return If successful, the specified Instance MUST have been updated by the
    CIM Server.

    If unsuccessful, one of the following status codes MUST be returned by
    this method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class of which this is
        to be a new Instance does not exist)
      <LI>CIM_ERR_NOT_FOUND (the CIM Instance does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI></UL>
    */
    void modifyInstance(
        const CIMNamespaceName& nameSpace,
        const CIMInstance& modifiedInstance,
        Boolean includeQualifiers = true,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    /** The <TT>enumerateClasses</TT> method is used to enumerate subclasses of
    a CIM Class in the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param className The <TT>className</TT> input parameter defines the Class
    that is the basis for the enumeration.

    @param deepInheritance If the <TT>deepInheritance</TT> input
    parameter is <TT>true</TT>, this specifies that all subclasses of
    the specified Class should be returned (if the <TT>ClassName</TT> input
    parameter is absent, this implies that all Classes in the target Namespace
    should be returned).  If <TT>false</TT>, only immediate child
    subclasses are returned (if the <TT>ClassName</TT> input parameter is
    NULL, this implies that all base Classes in the target Namespace should be
    returned).

    @param localOnly If the <TT>localOnly</TT> input parameter is
    <TT>true</TT>, it specifies that, for each returned Class, only elements
    (properties, methods and qualifiers) overriden within the definition of
    that Class are included.  If <TT>false</TT>, all elements are
    returned.  This parameter therefore effects a CIM Server-side mechanism
    to filter certain elements of the returned object based on whether or not
    they have been propagated from the parent Class (as defined by the
    <TT>PROPAGATED</TT> attribute).

    @param includeQualifiers If the <TT>includeQualifiers</TT> input parameter
    is <TT>true</TT>, this specifies that all Qualifiers for each Class
    (including Qualifiers on the Class and on any returned Properties, Methods
    or CIMMethod Parameters) MUST be included as <TT>&lt;QUALIFIER&gt;</TT>
    elements in the response.  If false no <TT>&lt;QUALIFIER&gt;</TT> elements
    are present in each returned Class.

    @param includeClassOrigin If the <TT>IncludeClassOrigin</TT> input
    parameter is <TT>true</TT>, this specifies that the <TT>CLASSORIGIN</TT>
    attribute MUST be present on all appropriate elements in each returned
    Class. If false, no <TT>CLASSORIGIN</TT> attributes are present in each
    returned Class.

    @return If successful, the method returns zero or more Classes that meet the
    required criteria.

    If unsuccessful, one of the following status codes MUST be returned by
    this method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
        basis for this enumeration does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
      </LI>
    </UL>
    */
    Array<CIMClass> enumerateClasses(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false,
        Boolean localOnly = true,
        Boolean includeQualifiers = true,
        Boolean includeClassOrigin = false
    );

    /** The <TT>enumerateClassNames</TT> operation is used to enumerate the
    names of subclasses of a CIM Class in the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param className The <TT>className</TT> input parameter defines the Class
    that is the basis for the enumeration.

    @param deepInheritance If the <TT>deepInheritance</TT> input parameter is
    true, this specifies that the names of all subclasses of the specified Class
    should be returned (if the ClassName input parameter is absent, this implies
    that the names of all Classes in the target Namespace should be returned).
    If false, only the names of immediate child subclasses are returned (if the
    className input parameter is NULL, this implies that the names of all base
    Classes in the target Namespace should be returned).  @return If successful,
    the method returns zero or more names of Classes that meet the requested
    criteria.  If unsuccessful, one of the following status codes MUST be
    returned by this method, where the first applicable error in the list
    (starting with the first element of the list, and working down) is the error
    returned.  Any additional method-specific interpretation of the error in is
    given in parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
        duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
        basis for this enumeration does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    Array<CIMName> enumerateClassNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className = CIMName(),
        Boolean deepInheritance = false
    );

    /** The <TT>enumerateInstances</TT> method enumerates instances of a CIM
    Class in the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param className The <TT>className</TT> input parameter defines the
    Class that is the basis for the enumeration.

    @param localOnly If the <TT>localOnly</TT> input parameter is
    <TT>true</TT>, this specifies that, for each returned Instance,
    only elements (properties and qualifiers) overriden within the
    definition of that Instance are included.  If <TT>false</TT>,
    all elements are returned.  This parameter therefore effects a CIM
    Server-side mechanism to filter certain elements of the returned object
    based on whether or not they have been propagated from the parent
    Class (as defined by the <TT>PROPAGATED</TT> attribute).

    Only elements (properties, methods and qualifiers) defined or
    overridden within the class are included in the response. Propagated
    properties are not included because their values are based on another class
    information. If not specified, all elements of the class definition are
    returned.  Note: When instances are returned, the InstanceName must include
    all keys, including propagated keys. Therefore, these attributes are
    included in the name part of the method response, but not in the value
    information.

    @param deepInheritance If the <TT>deepInheritance</TT> input
    parameter is <TT>true</TT>, this specifies that, for each
    returned Instance of the Class, all properties of the Instance MUST
    be present (subject to constraints imposed by the other
    parameters), including any which were added by subclassing the specified
    Class. If <TT>false</TT>, each returned Instance includes only
    properties defined for the specified Class.

    The Enumerate Instances operation returns the same number of instances
    regardless of whether or not the DeepInheritance flag is set.  The
    DeepInheritance flag is only used to determine whether or not the subclass
    property values should be returned.

    @param includeQualifiersIf the <TT>includeQualifiers</TT> input
    parameter is <TT>true</TT>, this specifies that all Qualifiers
    for each Instance (including Qualifiers on the Instance
    and on any returned Properties) MUST be included as
    <TT>&lt;QUALIFIER&gt;</TT> elements in the response.  If false no
    <TT>&lt;QUALIFIER&gt;</TT> elements are present in each
    returned Instance.

    @param includeClassOrigin If the <TT>includeClassOrigin</TT> input
    parameter is <TT>true</TT>, this specifies that the
    <TT>CLASSORIGIN</TT> attribute MUST be present on all appropriate
    elements in each returned Instance. If false, no
    <TT>CLASSORIGIN</TT> attributes are present in each returned
    Instance.

    @param propertyList If the <TT>propertyList</TT> input parameter is not
    <TT>NULL</TT>, the members of the array define one or more CIMProperty
    names.  Each returned Instance MUST NOT include elements
    for any Properties missing from this list.  Note that if
    <TT>LocalOnly</TT> is specified as <TT>true</TT> (or
    <TT>DeepInheritance</TT> is specified as <TT>false</TT>) this acts as an
    additional filter on the set of Properties returned (for example,
    if CIMProperty <TT>A</TT> is included in the
    <TT>PropertyList</TT> but <TT>LocalOnly</TT> is set to true and
    <TT>A</TT> is not local to a returned Instance, then it will not be
    included in that Instance). If the <TT>PropertyList</TT> input parameter
    is an empty array this signifies that no Properties are included in each
    returned Instance. If the <TT>PropertyList</TT> input parameter is
    NULL this specifies that all Properties (subject to the conditions
    expressed by the other parameters) are included in each returned
    Instance.

    If the <TT>propertyList</TT> contains duplicate elements,
    the Server MUST ignore the duplicates but otherwise process the request
    normally.  If the <TT>PropertyList</TT> contains elements which are
    invalid CIMProperty names for any target Instance, the Server MUST
    ignore such entries but otherwise process the request normally.

    @return If successful, the method returns zero or more named
    Instances that meet the required criteria.

    If unsuccessful, one of the following status codes MUST be returned
    by this method, where the first applicable error in the list (starting
    with the first element of the list, and working down) is the error
    returned. Any additional method-specific interpretation of the error in
    is given in parentheses.

        <UL>
          <LI>CIM_ERR_ACCESS_DENIED
          <LI>CIM_ERR_NOT_SUPPORTED
          <LI>CIM_ERR_INVALID_NAMESPACE
          <LI>CIM_ERR_INVALID_PARAMETER (including missing,
                duplicate, unrecognized or otherwise incorrect parameters)
          <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
                basis for this enumeration does not exist)
          <LI>CIM_ERR_FAILED (some other unspecified erroroccurred)</LI>
        </UL>
    */
    Array<CIMInstance> enumerateInstances(
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        Boolean deepInheritance = true,
        Boolean localOnly = true,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    /** The <TT>enumerateInstanceNames</TT> operation enumerates the
    names (model paths) of the instances of a CIM Class in the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param className The <TT>className</TT> input parameter defines the Class
    that is the basis for the enumeration.

    @return If successful, the method returns zero or more names of Instances
    (model paths) that meet the requsted criteria.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class that is the
      basis for this enumeration does not exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
     </UL>
    */
    Array<CIMObjectPath> enumerateInstanceNames(
        const CIMNamespaceName& nameSpace,
        const CIMName& className
    );

    /** The <TT>execQuery</TT> is used to execute a query against the target
    Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param queryLanguage The <TT>queryLanguage</TT> String input parameter
    defines the query language in which the Query parameter is expressed.

    @param query The <TT>query</TT> input parameter defines the query to be
    executed.

    @return If successful, the method returns zero or more CIM Classes or
    Instances that correspond to the results set of the query.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_QUERY_LANGUAGE_NOT_SUPPORTED (the requested query language is
      not recognized)
      <LI>CIM_ERR_INVALID_QUERY (the query is not a valid query in the
      specified query language)
      <LI>CIM_ERR_FAILED (some other unspecified error ccurred)</LI>
     </UL>
    */
    Array<CIMObject> execQuery(
        const CIMNamespaceName& nameSpace,
        const String& queryLanguage,
        const String& query
    );

    /** The <TT>Associators</TT> method enumerates CIM Objects
    (Classes or Instances) that are associated to a particular source CIM
    Object.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param objectName The <TT>objectName</TT> input parameter defines the source
    CIM Object whose associated Objects are to be returned.  This may be either
    a Class name or Instance name (model path).

    @param assocClass The <TT>assocClass</TT> input parameter, if not NULL, MUST
    be a valid CIM Association Class name. It acts as a filter on the returned
    set of Objects by mandating that each returned Object MUST be associated to
    the source Object via an Instance of this Class or one of its subclasses.

    @param resultClass The <TT>resultClass</TT> input parameter, if not NULL,
    MUST be a valid CIM Class name. It acts as a filter on the returned set of
    Objects by mandating that each returned Object MUST be either an Instance of
    this Class (or one of its subclasses) or be this Class (or one of its
    subclasses).

    @param role The <TT>role</TT> input parameter, if not NULL, MUST be a valid
    CIMProperty name. It acts as a filter on the returned set of Objects by
    mandating that each returned Object MUST be associated to the source Object
    via an Association in which the source Object plays the specified role (i.e.
    the name of the CIMProperty in the Association Class that refers to the
    source object MUST match the value of this parameter).

    @param resultRole The <TT>resultRole</TT> input parameter, if not NULL, MUST
    be a valid CIMProperty name. It acts as a filter on the returned set of
    Objects by mandating that each returned Object MUST be associated to the
    source Object via an Association in which the returned Object plays the
    specified role (i.e. the name of the CIMProperty in the Association Class
    that refers to the returned Object MUST match the value of this parameter).

    @param includeQualifiers If the <TT>includeQualifiers</TT> input parameter
    is true, this specifies that all Qualifiers for each Object (including
    Qualifiers on the Object and on any returned Properties) MUST be included as
    <QUALIFIER> elements in the response.  If false no <QUALIFIER> elements are
    present in each returned Object.

    @param includeClassOrigin If the <TT>includeClassOrigin</TT> input parameter
    is true, this specifies that the CLASSORIGIN attribute MUST be present on
    all appropriate elements in each returned Object. If false, no CLASSORIGIN
    attributes are present in each returned Object.

    @param propertyList If the <TT>propertyList</TT> input parameter is not
    NULL, the members of the array define one or more CIMProperty names.  Each
    returned Object MUST NOT include elements for any Properties missing from
    this list. Note that if LocalOnly is specified as true (or DeepInheritance
    is specified as false) this acts as an additional filter on the set of
    Properties returned (for example, if CIMProperty A is included in the
    PropertyList but LocalOnly is set to true and A is not local to a returned
    Instance, then it will not be included in that Instance). If the
    PropertyList input parameter is an empty array this signifies that no
    Properties are included in each returned Object. If the PropertyList input
    parameter is NULL this specifies that all Properties (subject to the
    conditions expressed by the other parameters) are included in each returned
    Object.

    If the propertyList contains duplicate elements, the Server MUST ignore the
    duplicates but otherwise process the request normally.  If the PropertyList
    contains elements which are invalid CIMProperty names for any target Object,
    the Server MUST ignore such entries but otherwise process the request
    normally.

    Clients SHOULD NOT explicitly specify properties in the PropertyList
    parameter unless they have specified a non-NULL value for the ResultClass
    parameter.

    @return If successful, the method returns zero or more CIM Classes or
    Instances meeting the requested criteria.  Since it is possible for CIM
    Objects from different hosts or namespaces to be associated, each returned
    Object includes location information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including
      missing,duplicate, unrecognized or
        otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
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

    /** The <TT>associatorNames</TT> operation enumerates the names of
    CIM Objects (Classes or Instances) that are associated to a particular
    source CIM Object.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param objectName The <TT>objectName</TT> input parameter defines the source
    CIM Object whose associated names are to be returned. This is either a Class
    name or Instance name (model path).

    @param assocClass The <TT>assocClass</TT> input parameter, if not NULL,
    MUST be a valid CIM Association Class name. It acts as a filter on the
    returned set of names by mandating that each returned name identifies an
    Object that MUST be associated to the source Object via an Instance of this
    Class or one of its subclasses.

    @param resultClass The <TT>resultClass</TT> input parameter, if not NULL,
    MUST be a valid CIM Class name. It acts as a filter on the returned set of
    names by mandating that each returned name identifies an Object that MUST be
    either an Instance of this Class (or one of its subclasses) or be this Class
    (or one of its subclasses).

    @param role The <TT>role</TT> input parameter, if not NULL, MUST be a valid
    CIMProperty name. It acts as a filter on the returned set of names by
    mandating that each returned name identifies an Object that MUST be
    associated to the source Object via an Association in which the source
    Object plays the specified role (i.e. the name of the CIMProperty in the
    Association Class that refers to the source Object MUST match the value of
    this parameter).

    @param resultRole The <TT>resultRole</TT> input parameter, if not
    <TT>NULL</TT>, MUST be a valid CIMProperty name. It acts as a filter on the
    returned set of names by mandating that each returned name identifies an
    Object that MUST be associated to the source Object via an Association in
    which the named returned Object plays the specified role (i.e. the name of
    the CIMProperty in the Association Class that refers to the returned Object
    MUST match the value of this parameter).

    @return If successful, the method returns zero or more full CIM Class paths
    or Instance paths of Objects meeting the requested criteria. Since it is
    possible for CIM Objects from different hosts or namespaces to be
    associated, each returned path is an absolute path that includes host and
    namespace information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE;
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    Array<CIMObjectPath> associatorNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& assocClass = CIMName(),
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        const String& resultRole = String::EMPTY
    );

    /** The <TT>references</TT> operation enumerates the association
    objects that refer to a particular target CIM Object (Class or Instance).

    @param The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param objectName The <TT>objectName</TT> input parameter defines the target
    CIM Object whose referring Objects are to be returned. This is either a
    Class name or Instance name (model path).

    @param resultClass The <TT>resultClass</TT> input parameter, if not NULL,
    MUST be a valid CIM Class name. It acts as a filter on the returned set of
    Objects by mandating that each returned Object MUST be an Instance of this
    Class (or one of its subclasses), or this Class (or one of its subclasses).

    @param role The <TT>role</TT> input parameter, if not NULL, MUST be a valid
    CIMProperty name. It acts as a filter on the returned set of Objects by
    mandating that each returned Objects MUST refer to the target Object via a
    CIMProperty whose name matches the value of this parameter.

    @param includeQualifiers.  If the <TT>includeQualifiers</TT> input parameter
    is true, this specifies that all Qualifiers for each Object (including
    Qualifiers on the Object and on any returned Properties) MUST be included as
    <QUALIFIER> elements in the response.  If false no <QUALIFIER> elements are
    present in each returned Object.

    @param includeClassOrigin If the <TT>includeClassOrigin</TT> input parameter
    is true, this specifies that the CLASSORIGIN attribute MUST be present on
    all appropriate elements in each returned Object. If false, no CLASSORIGIN
    attributes are present in each returned Object.

    @param propertyList If the <TT>propertyList</TT> input parameter is not
    NULL, the members of the array define one or more CIMProperty names.  Each
    returned Object MUST NOT include elements for any Properties missing from
    this list. Note that if LocalOnly is specified as true (or DeepInheritance
    is specified as false) this acts as an additional filter on the set of
    Properties returned (for example, if CIMProperty A is included in the
    PropertyList but LocalOnly is set to true and A is not local to a returned
    Instance, then it will not be included in that Instance). If the
    PropertyList input parameter is an empty array this signifies that no
    Properties are included in each returned Object. If the PropertyList input
    parameter is NULL this specifies that all Properties (subject to the
    conditions expressed by the other parameters) are included in each returned
    Object.

    If the PropertyList contains duplicate elements, the Server MUST ignore the
    duplicates but otherwise process the request normally.  If the PropertyList
    contains elements which are invalid CIMProperty names for any target Object,
    the Server MUST ignore such entries but otherwise process the request
    normally.

    Clients SHOULD NOT explicitly specify properties in the PropertyList
    parameter unless they have specified a non-NULL value for the ResultClass
    parameter.

    @return If successful, the method returns zero or more CIM Classes or
    Instances meeting the requested criteria.  Since it is possible for CIM
    Objects from different hosts or namespaces to be associated, each returned
    Object includes location information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
          duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
     </UL>
    */
    Array<CIMObject> references(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY,
        Boolean includeQualifiers = false,
        Boolean includeClassOrigin = false,
        const CIMPropertyList& propertyList = CIMPropertyList()
    );

    /** The <TT>referenceNames</TT> operation enumerates the association
    objects that refer to a particular target CIM Object (Class or Instance).

    @param The NameSpace parameter is a string that defines the target
    namespace \Ref{NAMESPACE}

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param objectName The <TT>objectName</TT> input parameter defines the target
    CIM Object whose referring object names are to be returned. It may be either
    a Class name or an Instance name (model path).

    @param resultClass The <TT>resultClass</TT> input parameter, if not NULL,
    MUST be a valid CIM Class name. It acts as a filter on the returned set of
    Object Names by mandating that each returned Object CIMName MUST identify an
    Instance of this Class (or one of its subclasses), or this Class (or one of
    its subclasses).

    @param role The <TT>role</TT> input parameter, if not NULL, MUST be a valid
    CIMProperty name. It acts as a filter on the returned set of Object Names by
    mandating that each returned Object CIMName MUST identify an Object that
    refers to the target Instance via a CIMProperty whose name matches the value
    of this parameter.

    @return If successful,the method returns the names of zero or more full CIM
    Class paths or Instance paths of Objects meeting the requested criteria.
    Since it is possible for CIM Objects from different hosts or namespaces to
    be associated, each returned path is an absolute path that includes host and
    namespace information.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
        <LI>CIM_ERR_ACCESS_DENIED
        <LI>CIM_ERR_NOT_SUPPORTED
        <LI>CIM_ERR_INVALID_NAMESPACE
        <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
        unrecognized or otherwise incorrect parameters)
        <LI>CIM_ERR_FAILED (some other unspecified error occurred)
     </UL>
    */
    Array<CIMObjectPath> referenceNames(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& objectName,
        const CIMName& resultClass = CIMName(),
        const String& role = String::EMPTY
    );

    /**
    The <TT>getProperty</TT>operation is used to retrieve a single property
    value from a CIM Instance in the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param instanceName The <TT>instanceName</TT> input parameter specifies the
    name of the Instance (model path) from which the CIMProperty value is
    requested. \\Ref{INSTANCENAME}

    @param propertyName The <TT>propertyName</TT> input parameter specifies the
    name of the CIMProperty whose value is to be returned.

    @return If successful, the return value specifies the value of the requested
    CIMProperty. If the value is NULL then no element is returned.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
        <LI>CIM_ERR_ACCESS_DENIED
        <LI>CIM_ERR_INVALID_NAMESPACE
        <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
        unrecognized or otherwise incorrect parameters)
        <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the specified
        namespace)
        <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested CIM
        Instance does not exist in the specified namespace)
        <LI><LI>CIM_ERR_NO_SUCH_PROPERTY (the CIM Instance does exist, but the
        requested CIMProperty does not)
        <LI>CIM_ERR_FAILED (some other unspecified error occurred)
    </UL>
    */
    CIMValue getProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName
    );

    /** The <TT>setProperty</TT> operation sets a single property value in a CIM
    Instance in the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param instanceName The <TT>instanceName</TT> input parameter specifies the
    name of the Instance (model path) for which the CIMProperty value is to be
    updated.

    @param propertyName The <TT>propertyName</TT> input parameter specifies the
    name of the CIMProperty whose value is to be updated.

    @param newValue The NewValue input parameter specifies the new value for the
    CIMProperty (which may be NULL).

    @return If unsuccessful, one of the following status codes MUST be returned
    by this method, where the first applicable error in the list (starting with
    the first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.
    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_INVALID_NAMESPACE

      <LI>CIM_ERR_INVALID_PARAMETER (including
      missing,duplicate, unrecognized or otherwise incorrect parameters)

      <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the specified
      namespace)
      <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested
      CIM Instance does not exist in the specified namespace)
      <LI>CIM_ERR_NO_SUCH_PROPERTY (the CIM Instance does exist, but the
      requested CIMProperty does not)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    void setProperty(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& propertyName,
        const CIMValue& newValue = CIMValue()
    );

    /** The <TT>getQualifier</TT> operation retrieves a single CIMQualifier
    declaration from the target Namespace.

    @param nameSpace The nameSpace parameter is a CIMNameSpace object that defines 
    the target namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param qualifierName The <TT>qualifierName</TT> input parameter  is a CIMName object 
    that identifies the CIMQualifier whose declaration to be retrieved.

    @return If successful, the method returns the CIMQualifier declaration for
    the named CIMQualifier.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
          duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_INVALID_CLASS (the CIM Class does not exist in the specified
          namespace)
      <LI>CIM_ERR_NOT_FOUND (the CIM Class does exist, but the requested
          CIM Instance does not exist in the specified namespace)
      <LI>CIM_ERR_NO_SUCH_PROPERTY (the CIM Instance does exist, but the
          requested CIMProperty does not)
      <LI>CIM_ERR_TYPE_MISMATCH (the supplied value is incompatible with the
          type of the CIMProperty)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
    </UL>
    */
    CIMQualifierDecl getQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName
    );

    /** The <TT>setQualifier</TT> creates or update a single CIMQualifier
    declaration in the target Namespace.  If the CIMQualifier declaration
    already exists it is overwritten.

    @param nameSpace The nameSpace parameter is a CIMName that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param CIMQualifierDecl The <TT>CIMQualifierDecl</TT> input parameter is a
    CIMQualifier object that defines the CIMQualifier Declaration to be added to
    the Namespace.

    @return If successful, the CIMQualifier declaration MUST have been added to
    the target Namespace. If a CIMQualifier declaration with the same
    CIMQualifier name already existed, then it MUST have been replaced by the
    new declaration.

    If unsuccessful, a CIMException with one of the following status codes MUST be returned
    by this method, where the first applicable error in the list (starting with
    the first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_NOT_FOUND (the requested CIMQualifier declaration did not
      exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
      </LI>
    </UL>
    @exception CIMException - Any of the CIMException codes defined above may
    be returned.
    @Exception Exception - TBD
    <pre>
        ... Connect with CIMClient object client;
        CIMNamespaceName nameSpace("root/cimv2");
        CIMQualifierDecl qual1(
            CIMName ("CIMTYPE"), 
            String(),
            CIMScope::PROPERTY,
            CIMFlavor::TOINSTANCE);
        try
        {
            client.setQualifier(nameSpace, qual1);
        }
        catch(CIMException e)
        {
            ...
        }
    </pre>
    */
    void setQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMQualifierDecl& qualifierDeclaration
    );

    /** The <TT>deleteQualifier</TT> operation deletes a single CIMQualifier
    declaration from the target Namespace.

    @param nameSpace The nameSpace parameter is a CIMName that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param qualifierName CIMName <TT>qualifierName</TT> input parameter identifies
    the CIMQualifier whose declaration to be deleted. 
        
    @return If successful, the specified CIMQualifier declaration MUST have been deleted
    from the Namespace with a normal return from the call.

    If there is any error one of the CIMException errors is returned.  The 
    errors are based on the CIM error codes defined below:  
      <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate, unrecognized or 
           otherwise incorrect parameters)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)</LI>
      </UL>
    @exception CIMexception - May return any of the CIMException codes defined above.
    @exception Exception
    <pre>
    // simple function to delete qualifier named "Expensive".  Exceptions ignored.
        CIMClient client;
        client.connect("localhost", 5988, String::EMPTY, String::EMPTY);
        CIMName qualifierName("Expensive");
        client.deleteQualifier(CIMName("root/cimv2",qualifierName);
    </pre>
    */
    void deleteQualifier(
        const CIMNamespaceName& nameSpace,
        const CIMName& qualifierName
    );

    /** The <TT>enumerateQualifiers</TT> operation is used to enumerate
    CIMQualifier declarations from the target Namespace.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @return If successful, the method returns zero or more CIMQualifier
    declarations.

    If unsuccessful, one of the following status codes MUST be returned by this
    method, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional method-specific interpretation of the error in is given in
    parentheses.

    <UL>
      <LI>CIM_ERR_ACCESS_DENIED
      <LI>CIM_ERR_NOT_SUPPORTED
      <LI>CIM_ERR_INVALID_NAMESPACE
      <LI>CIM_ERR_INVALID_PARAMETER (including missing,
      duplicate, unrecognized or otherwise incorrect parameters)
      <LI>CIM_ERR_NOT_FOUND (the requested CIMQualifier declaration did not
      exist)
      <LI>CIM_ERR_FAILED (some other unspecified error occurred)
      </LI>
     </UL>
    <pre>
        // function to get all qualifer declarations and print xml form.
        try
        {
            CIMClient client;
            client.connect("localhost", 5988, String::EMPTY, String::EMPTY);
            Array<CIMQualifierDecl> qualifierDecls;
            CIMNamespaceName nameSpace("root/cimv2");
            qualifierDecls = client.enumerateQualifiers(nameSpace);
        }
        catch(Exception& e)
        {
            PEGASUS_STD(cerr) << "Error: " << e.getMessage() << PEGASUS_STD(endl);
            exit(1);
        }
        for (Uint32 i = 0; i < qualifierDecls.size(); i++)
        {
            XmlWriter::printQualifierDeclElement(qualifierDecls[i], cout);
        }
    </pre>
    */
    Array<CIMQualifierDecl> enumerateQualifiers(
        const CIMNamespaceName& nameSpace
    );

    /** Execute an extrinsic CIM method.
    Any CIM Server is assumed to support extrinsic methods. Extrinsic methods
    are defined by the Schema supported by the Cim Server. If a CIM Server does
    not support extrinsic method invocations, it MUST (subject to the
    considerations described in the rest of this section) return the error code
    CIM_ERR_NOT_SUPPORTED to any request to execute an extrinsic method. This
    allows a CIM client to determine that all attempts to execute extrinsic
    methods will fail.

    @param nameSpace The nameSpace parameter is a string that defines the target
    namespace. See defintion of
    \URL[Namespace]{DefinitionofTerms.html#NAMESPACE}.

    @param instanceName The <TT>instanceName</TT> parameter is a CIMReference
    that defines the CIM instance for which the method is defined

    @param methodName The <TT>methodName</TT> parameter is a String with the
    name of the method to be executed.

    @param inParameters This parameter defines an array of input parameters for
    the method execution

    @param outParameters This parameter defines an array of parameters returned
    by the executed method

    @return If the Cim Server is unable to perform the extrinsic method
    invocation, one of the following status codes MUST be returned by the
    CimServer, where the first applicable error in the list (starting with the
    first element of the list, and working down) is the error returned. Any
    additional specific interpretation of the error is given in parentheses.

    ATTN: We have not defined the CIMValue returned
    <UL>

         <LI>CIM_ERR_ACCESS_DENIED
         <LI>CIM_ERR_NOT_SUPPORTED (the CimServer does not support extrinsic
         method invocations)
         <LI>CIM_ERR_INVALID_NAMESPACE
         <LI>CIM_ERR_INVALID_PARAMETER (including missing, duplicate,
         unrecognized or otherwise incorrect parameters)
         <LI>CIM_ERR_NOT_FOUND (the target CIM Class or instance does not exist
         in the specified namespace)
         <LI>CIM_ERR_METHOD_NOT_FOUND
         <LI>CIM_ERR_METHOD_NOT_AVAILABLE (the CimServer is unable to honor the
         invocation request)
         <LI>CIM_ERR_FAILED (some other unspecified error occurred)
    </UL>

    */
    CIMValue invokeMethod(
        const CIMNamespaceName& nameSpace,
        const CIMObjectPath& instanceName,
        const CIMName& methodName,
        const Array<CIMParamValue>& inParameters,
        Array<CIMParamValue>& outParameters
    );

    /**
        Registers a ClientOpPerformanceDataHandler object.  The specified
        object is called with performance data relative to each operation
        performed by this CIMClient object.  Only one
        ClientOpPerformanceDataHandler can be registered at a time.
        A subsequent registration replaces the previous one.
        @param handler The ClientOpPerformanceDataHandler object to register.
    */
    void registerClientOpPerformanceDataHandler(
        ClientOpPerformanceDataHandler & handler);

    /**
        Unregisters the current ClientOpPerformanceDataHandler, if applicable.
    */
    void deregisterClientOpPerformanceDataHandler();

private:

    /**
        The copy constructor is not available for the CIMClient class.
    */
    CIMClient(const CIMClient&);

    /**
        The assignment operator is not available for the CIMClient class.
    */
    CIMClient& operator=(const CIMClient&);

    CIMClientInterface* _rep;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Client_h */
