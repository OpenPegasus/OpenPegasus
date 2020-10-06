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

//
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By:Karl Schopmeyer(k.schopmeyer@opengroup.org)
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
the methods.  The minimal method implementation simply throws the 
NotSupported exception. The methods implemented match the association
operations defined for the client:
    <UL>
    <LI> referenceNames
    <LI> references
    <LI> associatorNames
    <LI> associators
    </UL>
Note that the major difference is that the attributes implemented are not exactly
the same as the attributes of the operations implemented for the client.

*/
class PEGASUS_PROVIDER_LINKAGE CIMAssociationProvider : public virtual CIMProvider
{
public:
    CIMAssociationProvider(void);
    virtual ~CIMAssociationProvider(void);

    /** Enumerates CIM Objects (Instances only) that are associated to a
        particular source CIM Object. Returns CIM Objects. NOTE: if the objects
        do not include the host and namespace information this information will
        be inserted by the CIMOM based on the name of the host in which the cimom
        resides and the namespace in the request.

        @param context Contains security and locale information relevant for the 
        lifetime of this operation.  

        @param objectName The ObjectName input parameter defines the source CIM 
        Object whose associated Objects are to be returned.  This may be either a 
        Class name or Instance name (CIMObjectpath).  
            
        @param associationClass The AssocClass input parameter, if not NULL, MUST 
        be a valid CIM Association Class name.  It acts as a filter on the 
        returned set of Objects by mandating that each returned Object MUST be 
        associated to the source Object via an Instance of this Class or one of 
        its subclasses.  
            
        @param resultClass If not NULL, this parameter MUST be a valid CIM class 
        name.  It act as a filter on the returned set of Objects by mandating that 
        each returned Object MUST be an Instance of this Class (or one of 
        its subclasses). NOTE: This call does not provide the subclasses needed
        to satisfy the request.  It provides the resultClass defined by the request
        and which is a valid result class for the request.
            
        @param role If not NULL, this parameter MUST be a valid Property name.  It 
        acts as a filter on the returned set of Objects by mandating that each 
        returned Object MUST be associated to the source Object via an Association 
        in which the source Object plays the specified role (i.e.  the name of the 
        Property in the Association Class that refers to the source Object MUST 
        match the value of this parameter).  
            
        @param resultRole If not NULL, this parameter MUST be a valid Property 
        name.  It acts as a filter on the returned set of Objects by mandating 
        that each returned Object MUST be associated to the source Object via an 
        Association in which the returned Object plays the specified role (i.e.  
        the name of the Property in the Association Class that refers to the 
        returned Object MUST match the value of this parameter).  
            
        @param includeQualifiers If true, this specifies that all Qualifiers for 
        each Object (including Qualifiers on the Object and on any returned 
        Properties) MUST be included as <QUALIFIER> elements in the response.  If 
        false no <QUALIFIER> elements are present in each returned Object.  
        
        @param includeClassOrigin If true, this specifies that the CLASSORIGIN 
        attribute MUST be present on all appropriate elements in each returned 
        Object.  If false, no CLASSORIGIN attributes are present in each returned 
        Object.  
            
        @param propertyList If not NULL, the members of the array define one or 
        more roperty names.  Each returned Object MUST NOT include elements for 
        any Properties missing from this list.  If the PropertyList input 
        parameter is an empty array this signifies that no Properties are included 
        in each returned Object.  If the PropertyList input parameter is NULL this 
        specifies that all Properties (subject to the conditions expressed by the 
        other parameters) are included in each returned Object.  

    @param handler Asynchronously processes the results of this operation.

    @exception NotSupported Methods not all supported by the provider return this
    exception.
    @exception InvalidParameter If a parameter is not specified correctly.
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

    /**  Enumerate the names of CIM Objects (Instances) associated to a
        particular source CIM Object. Returns multiple CIMObjectPath objects
        through the handler. The returned CIMObjectPaths are expected to be
        absolute including host name and namespace.  If these fields are not
        supplied by the provider they will be inserted by the CIMOM based on
        the host in which the CIMOM resides and the namespace in the request.
     
        @param context Contains security and locale information relevant for the 
        lifetime of this operation.  
                
        @param objectName The ObjectName input parameter defines the source CIM 
        Object whose associated Objects are to be returned.  This may be either a 
        Class name or Instance name (CIMObjectpath).  
                
        @param associationClass The AssocClass input parameter, if not NULL, MUST 
        be a valid CIM Association Class name.  It acts as a filter on the 
        returned set of Objects by mandating that each returned Object MUST be 
        associated to the source Object via an Instance of this Class or one of 
        its subclasses.  
                
        @param resultClass If not NULL, MUST be a valid CIM Class name.  It acts 
        as a filter on the returned set of Objects by mandating that each returned 
        Object MUST be either an Instance of this Class (or one of its 
        subclasses).  NOTE: This call does not provide the subclasses needed
        to satisfy the request.  It provides the resultClass defined by the request
        and which is a valid result class for the request.
        
        @param role If not NULL, this parameter MUST be a valid Property name.  It 
        acts as a filter on the returned set of Objects by mandating that each 
        returned Object MUST be associated to the source Object via an Association 
        in which the source Object plays the specified role (i.e.  the name of the 
        Property in the Association Class that refers to the source Object MUST 
        match the value of this parameter).  
        
        @param resultRole If not NULL, this parameter MUST be a valid Property 
        name.  It acts as a filter on the returned set of Objects by mandating 
        that each returned Object MUST be associated to the source Object via an 
        Association in which the returned Object plays the specified role (i.e.  
        the name of the Property in the Association Class that refers to the 
        returned Object MUST match the value of this parameter).  
        
        @param handler Asynchronously processes the results of this operation.
        
        @exception NotSupported returned by methods that are not implemented by the provider..
        @exception InvalidParameter If a parameter is not specified correctly.
    */
    virtual void associatorNames(
	const OperationContext & context,
	const CIMObjectPath & objectName,
	const CIMName & associationClass,
	const CIMName & resultClass,
	const String & role,
	const String & resultRole,
	ObjectPathResponseHandler & handler) = 0;

    /** Enumerate the association objects that refer to a particular target CIM Object
        (Instance).  Returns multiple CIMObjectPath objects
        through the handler. The returned CIMObjectPaths are expected to be
        absolute including host name and namespace.  If these fields are not
        supplied by the provider they will be inserted by the CIMOM based on
        the host in which the CIMOM resides and the namespace in the request.
	 
        @param context Contains security and locale information relevant for the lifetime
        of this operation.
        
        @param objectName The target CIM Object whose referring object names are to be returned.
         Note that only instances will be forwarded to the provider. All class level
         requests are handled by the CIM Server internally.
          
        @param resultClass If not NULL, MUST be a valid CIM Class name. It acts as a filter
         on the returned set of Object Names by mandating that each returned Object Name
         MUST identify an Instance of this Class (or one of its subclasses). NOTE: This call
         does not provide the subclasses needed to satisfy the request.  It provides the
         resultClass defined by the request and which is a valid result class for the request.
         
        @param role  The Role input parameter, if not NULL, MUST be a valid Property name. 
         It acts as a filter on the returned set of Objects by mandating that each returned
         Object MUST be associated to the source Object via an Association in which the 
         source Object plays the specified role (i.e. the name of the Property
         in the Association Class that refers to the source Object MUST match the value 
         of this parameter.
        
        @param resultRole If not NULL, MUST be a valid Property name. It acts as a
         filter on the returned set of Objects by mandating that each returned Object
         MUST be associated to the source Object via an Association in which the
         returned Object plays the specified role (i.e. the name of the Property in
         the Association Class that refers to the returned Object MUST match the
         value of this parameter).
        
         @param includeQualifiers If true, this specifies that all Qualifiers for each
         Object (including Qualifiers on the Object and on any returned Properties)
         MUST be included as <QUALIFIER> elements in the response. If false no
         <QUALIFIER> elements are present in each returned Object. 
        
         @param includeClassOrigin If true, this specifies that the CLASSORIGIN attribute
         MUST be present on all appropriate elements in each returned Object.
         If false, no CLASSORIGIN attributes are present in each returned Object. 
        
         @param propertyList - If not NULL, the members of the array define one or more
         Property names. Each returned Object MUST NOT include elements for any
         Properties missing from this list. If the PropertyList input parameter
         is an empty array this signifies that no Properties are included in each
         returned Object. If the PropertyList input parameter is NULL this specifies
         that all Properties (subject to the conditions expressed by the other
         parameters) are included in each returned Object.
        
        @param handler Asynchronously processes the results of this operation.
        
        @exception NotSupported Returned for any methods not implemented by the provider.
        @exception InvalidParameter If a parameter is not specified correctly.
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

    /** Enumerate the association object names that refer to a particular target CIM Object
        (Instance). Returns CIMOobjectPath objects via the handler.
        The returned CIMObjectPaths are expected to be
        absolute including host name and namespace.  If these fields are not
        supplied by the provider they will be inserted by the CIMOM based on
        the host in which the CIMOM resides and the namespace in the request.

        @param context Contains security and locale information relevant for the lifetime
        of this operation.
    
        @param objectName The target CIM Object whose referring object names are to be returned.
         Note that only instances will be forwarded to the provider. All class level
         requests are handled by the CIM Server internally.
         
        @param resultClass If not NULL, MUST be a valid CIM Class name. It acts as a filter
         on the returned set of Object Names by mandating that each returned Object Name
         MUST identify an Instance of this Class (or one of its subclasses).
         NOTE: This call does not provide the subclasses needed
         to satisfy the request.  It provides the resultClass defined by the request
         and which is a valid result class for the request.
         
        @param role  The Role input parameter, if not NULL, MUST be a valid Property name. 
         It acts as a filter on the returned set of Objects by mandating that each returned
         Object MUST be associated to the source Object via an Association in which the 
         source Object plays the specified role (i.e. the name of the Property
         in the Association Class that refers to the source Object MUST match the value 
         of this parameter. 
    
        @param handler Asynchronously processes the results of this operation.
    
        @exception NotSupported xception returned for any method not implemented by the provider.
        @exception InvalidParameter If the parameter is not specified correctly.
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
