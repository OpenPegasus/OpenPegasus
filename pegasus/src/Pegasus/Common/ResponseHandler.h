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
    The ResponseHandler class allows a provider to report operation
    progress and results to the CIM Server.  Subclasses are defined
    for each type of result data.
*/
class PEGASUS_COMMON_LINKAGE ResponseHandler
{
public:

    /**
        Destructs the ResponseHandler.
    */
    virtual ~ResponseHandler();

    // This method is defined in subclasses, specialized for
    // the appropriate data type.
    //virtual void deliver(const T & object);

    // This method is defined in subclasses, specialized for
    // the appropriate data type.
    //virtual void deliver(const Array<T> & objects);

    /**
        Informs the CIM Server that delivery of results will begin.
        This method must be called before deliver() is called.
    */
    virtual void processing() = 0;

    /**
        Informs the CIM Server that delivery of results is complete.
        This method must be called when all the results have been delivered.
        The deliver() method must not be called after this method is called.
    */
    virtual void complete() = 0;

    /**
        Sets the context for operation responses delivered to the CIM Server.
        This method allows a provider to communicate context information
        (such as content language) along with an operation response.  The
        context information applies to all the operation response data
        delivered to this ResponseHandler object.  The context information
        is applied at the time the complete() method is called.
    */
    void setContext(const OperationContext & context);

protected:

    /**
        The default constructor is not available for the ResponseHandler class.
    */
    ResponseHandler();

    /**
        The copy constructor is not available for the ResponseHandler class.
    */
    ResponseHandler(const ResponseHandler& handler);

    /**
        The assignment operator is not available for the ResponseHandler class.
    */
    ResponseHandler& operator=(const ResponseHandler& handler);

    /**
        Gets the context for the results delivered to the CIM Server.
    */
    OperationContext getContext() const;
};

/**
    The InstanceResponseHandler class is a subclass of the ResponseHandler
    class which allows delivery of instance results.
*/
class PEGASUS_COMMON_LINKAGE InstanceResponseHandler
    : virtual public ResponseHandler
{
public:
    /**
        Delivers an instance result to the CIM Server.  This method may
        be called multiple times when more than one result needs to be
        delivered.  An Array form of this method is also available to
        deliver multiple results.
        @param instance The instance to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const CIMInstance & instance) = 0;

    /**
        Delivers multiple instance results to the CIM Server.  This method
        may be invoked multiple times, if necessary.
        @param instances The instances to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const Array<CIMInstance> & instances) = 0;
};


/**
    The ObjectPathResponseHandler class is a subclass of the ResponseHandler
    class which allows delivery of object path results.
*/
class PEGASUS_COMMON_LINKAGE ObjectPathResponseHandler
    : virtual public ResponseHandler
{
public:
    /**
        Delivers an object path result to the CIM Server.  This method may
        be called multiple times when more than one result needs to be
        delivered.  An Array form of this method is also available to
        deliver multiple results.
        @param objectPath The object path to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const CIMObjectPath & objectPath) = 0;

    /**
        Delivers multiple object path results to the CIM Server.  This method
        may be invoked multiple times, if necessary.
        @param objectPaths The object paths to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const Array<CIMObjectPath> & objectPaths) = 0;
};


/**
    The MethodResultResponseHandler class is a subclass of the ResponseHandler
    class which allows delivery of extrinsic method results.
*/
class PEGASUS_COMMON_LINKAGE MethodResultResponseHandler
    : virtual public ResponseHandler
{
public:
    /**
        Delivers extrinsic method output parameters to the CIM Server.
        This method may be called multiple times when more than one result
        needs to be delivered.  An Array form of this method is also
        available to deliver multiple results.
        @param outParamValue The output parameter to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliverParamValue(const CIMParamValue & outParamValue) = 0;

    /**
        Delivers a set of output parameters to the CIM Server.  This method
        may be invoked multiple times, if necessary.
        @param outParamValues An Array of method output parameters to deliver
        to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliverParamValue(
        const Array<CIMParamValue> & outParamValues) = 0;

    /**
        Delivers an extrinsic method return value to the CIM Server.
        @param returnValue The return value to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const CIMValue & returnValue) = 0;
};


/**
    The IndicationResponseHandler class is a subclass of the ResponseHandler
    class which allows delivery of generated indications.
*/
class PEGASUS_COMMON_LINKAGE IndicationResponseHandler
    : virtual public ResponseHandler
{
public:
    /**
        Delivers an indication to the CIM Server.  An Array form of this
        method is available to deliver multiple indications at once.
        Another form is also available to specify the context for the
        delivery.
        @param indication The indication instance to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const CIMIndication & indication) = 0;

    /**
        Delivers multiple indications to the CIM Server.  Another form of
        this method is available to specify the context for the delivery.
        @param indications An Array of indication instances to deliver to the
        CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const Array<CIMIndication> & indications) = 0;

    /**
        Delivers an indication to the CIM Server with a specified context.
        An Array form of this method is available to deliver multiple
        indications at once.  The context may include data to be associated
        with the indication, such as the content language.
        @param context A context associated with the indication delivery.
        @param indication The indication instance to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(
        const OperationContext & context,
        const CIMIndication & indication) = 0;

    /**
        Delivers multiple indications to the CIM Server with a specified
        context.  The context may include data to be associated with the
        indications, such as the content language.
        @param context A context associated with the indication delivery.
        @param indications An Array of indication instances to deliver to the
        CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(
        const OperationContext & context,
        const Array<CIMIndication> & indications) = 0;
};


/**
    The ObjectResponseHandler class is a subclass of the ResponseHandler
    class which allows delivery of object results.
*/
class PEGASUS_COMMON_LINKAGE ObjectResponseHandler
    : virtual public ResponseHandler
{
public:
    /**
        Delivers an object result to the CIM Server.  This method may
        be called multiple times when more than one result needs to be
        delivered.  An Array form of this method is also available to
        deliver multiple results.
        @param object The object to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const CIMObject & object) = 0;

    /**
        Delivers multiple object results to the CIM Server.  This method
        may be invoked multiple times, if necessary.
        @param objects The objects to deliver to the CIM Server.
        @exception Exception May be thrown if the data that is delivered is
        not consistent with the corresponding request or associated schema.
    */
    virtual void deliver(const Array<CIMObject> & objects) = 0;
};

PEGASUS_NAMESPACE_END

#endif
