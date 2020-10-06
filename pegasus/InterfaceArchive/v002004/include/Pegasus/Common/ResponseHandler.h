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
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_ResponseHandler_h
#define Pegasus_ResponseHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Linkage.h>

#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Common/CIMValue.h>
#include <Pegasus/Common/CIMIndication.h>
#include <Pegasus/Common/CIMObject.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/OperationContext.h>


PEGASUS_NAMESPACE_BEGIN

/**
<p>The <tt>ResponseHandler</tt> class allows a provider
to report operation progress and results to the CIM Server.
Subclasses are defined for each of the types of object
that a provider can deliver to the CIM Server.
A <tt>ResponseHandler</tt> object of the appropriate type
is passed to provider
functions that are invoked to process client requests (it
is not passed to the <tt>{@link initialize initialize}</tt>
or <tt>{@link terminate terminate}</tt> functions). It
contains the following public member functions that
may be used to deliver results to the CIM Server:</p>
<ul>
<li><tt>{@link processing processing}</tt> - inform the CIM Server
that delivery of results is beginning.</li>
<li><tt>{@link deliver deliver}</tt> - deliver an incremental
result to the CIM Server; the CIM Server accumulates results as
they are received from the provider.</li>
<li><tt>{@link complete complete}</tt> - inform the CIM Server that
process of the request is complete and that no further
results will be delivered.</li>
</ul>
*/

class PEGASUS_COMMON_LINKAGE ResponseHandler
{
public:

    /**
    ResponseHandler destructor.
    */
    virtual ~ResponseHandler(void);

    // This method is defined in subclasses, specialized for 
    // the appropriate data type.
    //virtual void deliver(const T & object);

    // This method is defined in subclasses, specialized for 
    // the appropriate data type.
    //virtual void deliver(const Array<T> & objects);

    /**
    Inform the CIM server that delivery of results will begin.
    <p>The provider must call <tt>processing</tt> before
    attempting to call <tt>deliver</tt>.
    */
    virtual void processing(void) = 0;

    /**
    Inform the CIM server that delivery of results is complete.
    <p>The provider must call <tt>complete</tt> when all
    results have been delivered. The provider must not call <tt>deliver</tt>
    after calling <tt>complete</tt>.</p>
    */
    virtual void complete(void) = 0;

#ifdef PEGASUS_USE_EXPERIMENTAL_INTERFACES
    /** <I><B>Experimental Interface</B></I><BR>
    Set the context for the results delivered to the CIM Server.
    <p>The <tt>setContext</tt> function is used by providers to
    set a context for the response, in the form of an OperationContext
    object.  The context of the response contains the settings that
    apply to all the results delivered to the CIM Server.  An example
    context is the language of the response, in the form of a 
    ContentLanguageListContainer in the OperationContext.</p>
    <p>This method may be called at any point in the response
    processing before <tt>complete</tt> is called.</p>
    <p>Currently supported OperationContext containers:
    <li>
    ContentLanguageListContainer: used to set the ContentLanguages of the
    results.
    </li>
    </p>
    <p>Implementation Note:  This method is concrete to preserve 
    binary compatibility with previous releases of the CIMServer</p> 
    */
#ifdef PEGASUS_OS_OS400
virtual
#endif
    void setContext(const OperationContext & context);
#endif  // PEGASUS_USE_EXPERIMENTAL_INTERFACES

protected:
    
    ResponseHandler();

    ResponseHandler(const ResponseHandler& handler);

    ResponseHandler& operator=(const ResponseHandler& handler);

    /**
    Gets the context for the results delivered to the CIM server.
    */
    OperationContext getContext(void) const;
};


/**
The InstanceResponseHandler class is a subclass to the ResponseHandler class.
The InstanceResponseHandler class contains functions that are specific to an 
instance of the ResponseHandler class.
*/
class PEGASUS_COMMON_LINKAGE InstanceResponseHandler : virtual public ResponseHandler
{
public:
    /** <p>The <tt>deliver</tt> function is used by providers to
        deliver results to the CIM Server. For operations that require a
        single element result (<tt>getInstance</tt>, for example),
        <tt>deliver</tt> should be called only once to deliver the
        entire result. For operations that involve
        enumeration, the single-element form shown here may be
        used, each iteration delivering an incremental element
        of the total result. The Array form below may be used
        to deliver a larger set of result elements.</p>
    */
    virtual void deliver(const CIMInstance & instance) = 0;

    /** <p>This form of the <tt>deliver</tt> function may be used
        to return a set of elements to the CIM Server. The set is not
        required to be complete, and the provider may invoke this
        function multiple times, if necessary. This form should only
        be used when the operation requires a result consisting
        of more than one element, such as an enumeration.</p>
    */
    virtual void deliver(const Array<CIMInstance> & instances) = 0;
};


//
// ObjectPathResponseHandler
///
class PEGASUS_COMMON_LINKAGE ObjectPathResponseHandler : virtual public ResponseHandler
{
public:
    /** <p>The <tt>deliver</tt> function is used by providers to
        deliver results to the CIM Server. For operations that require a
        single element result (<tt>getInstance</tt>, for example),
        <tt>deliver</tt> should be called only once to deliver the
        entire result. For operations that involve
        enumeration, the single-element form shown here may be
        used, each iteration delivering an incremental element
        of the total result. The Array form below may be used
        to deliver a larger set of result elements.</p>
    */
    virtual void deliver(const CIMObjectPath & objectPath) = 0;

    /** <p>This form of the <tt>deliver</tt> function may be used
        to return a set of elements to the CIM Server. The set is not
        required to be complete, and the provider may invoke this
        function multiple times, if necessary. This form should only
        be used when the operation requires a result consisting
        of more than one element, such as an enumeration.</p>
    */
    virtual void deliver(const Array<CIMObjectPath> & objectPaths) = 0;
};


/**
The MethodResultResponseHandler class is a subclass to the ResponseHandler class.
*/
class PEGASUS_COMMON_LINKAGE MethodResultResponseHandler : virtual public ResponseHandler
{
public:
    /**
	Add documentation here.
	*/
    virtual void deliverParamValue(const CIMParamValue & outParamValue) = 0;

    /**
	Add documentation here.
	*/
    virtual void deliverParamValue(const Array<CIMParamValue> & outParamValues) = 0;

    /**
	Add documentation here.
	*/
    virtual void deliver(const CIMValue & returnValue) = 0;
};


/**
The IndicationResponseHandler class is a subclass to the ResponseHandler class.
<p> NOTE: This class definition should not be considered complete until indication
support has been completed in Pegasus.  Implementation of indication support may
reveal a need for API changes in this class.</p>
*/
class PEGASUS_COMMON_LINKAGE IndicationResponseHandler : virtual public ResponseHandler
{
public:
    /**
	Add documentation here.
	*/
    virtual void deliver(const CIMIndication & indication) = 0;

	/**
	Add documentation here.
	*/
    virtual void deliver(const Array<CIMIndication> & indications) = 0;

	/**
	Add documentation here.
	*/
    virtual void deliver(
        const OperationContext & context,
        const CIMIndication & indication) = 0;

	/**
	Add documentation here.
	*/
    virtual void deliver(
        const OperationContext & context,
        const Array<CIMIndication> & indications) = 0;
};


//
// ObjectResponseHandler
//
// NOTE: This class definition should not be considered complete until
// association provider and/or query provider support has been completed
// in Pegasus, as those are the only APIs that use this response handler
// type.  Implementation of support for those provider types may reveal
// a need for API changes in this class.
///
class PEGASUS_COMMON_LINKAGE ObjectResponseHandler : virtual public ResponseHandler
{
public:
    /** <p>The <tt>deliver</tt> function is used by providers to
        deliver results to the CIM Server. For operations that require a
        single element result (<tt>getInstance</tt>, for example),
        <tt>deliver</tt> should be called only once to deliver the
        entire result. For operations that involve
        enumeration, the single-element form shown here may be
        used, each iteration delivering an incremental element
        of the total result. The Array form below may be used
        to deliver a larger set of result elements.</p>
    */
    virtual void deliver(const CIMObject & object) = 0;

    /** <p>This form of the <tt>deliver</tt> function may be used
        to return a set of elements to the CIM Server. The set is not
        required to be complete, and the provider may invoke this
        function multiple times, if necessary. This form should only
        be used when the operation requires a result consisting
        of more than one element, such as an enumeration.</p>
    */
    virtual void deliver(const Array<CIMObject> & objects) = 0;
};

PEGASUS_NAMESPACE_END

#endif
