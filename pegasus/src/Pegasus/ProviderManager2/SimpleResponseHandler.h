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

#ifndef Pegasus_SimpleResponseHandler_h
#define Pegasus_SimpleResponseHandler_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/Common/CIMResponseData.h>
#include <Pegasus/Common/SCMO.h>

#include <Pegasus/ProviderManager2/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

//
// ValueResponseHandler (used internally to implement property operations)
//
class PEGASUS_PPM_LINKAGE ValueResponseHandler : virtual public ResponseHandler
{
public:
    virtual void deliver(const CIMValue& value) = 0;

    virtual void deliver(const Array<CIMValue>& values) = 0;
};


//
// SimpleResponseHandler
//
class PEGASUS_PPM_LINKAGE SimpleResponseHandler : virtual public ResponseHandler
{
public:
    SimpleResponseHandler();
    virtual ~SimpleResponseHandler();

    virtual void processing();

    virtual void complete();

    // return the number of objects in this handler
    virtual Uint32 size() const;

    // clear any objects in this handler
    virtual void clear();

    ContentLanguageList getLanguages();

protected:
    virtual void send(Boolean isComplete);

};

class PEGASUS_PPM_LINKAGE SimpleInstanceResponseHandler :
    public SimpleResponseHandler, public InstanceResponseHandler
{
public:
    SimpleInstanceResponseHandler();
    ~SimpleInstanceResponseHandler();

    virtual void processing();

    virtual void complete();

    virtual Uint32 size() const;

    virtual void clear();

    virtual void deliver(const CIMInstance& instance);

    virtual void deliver(const SCMOInstance& instance);

    virtual void deliver(const Array<CIMInstance>& instances);

    const Array<CIMInstance> getObjects() const;
    const Array<SCMOInstance> getSCMOObjects() const;

private:
    Array<CIMInstance> _objects;
    Array<SCMOInstance> _scmoObjects;
    //CIMInstanceResponseData responseData;
};

class PEGASUS_PPM_LINKAGE SimpleObjectPathResponseHandler :
    public SimpleResponseHandler, public ObjectPathResponseHandler
{
public:
    SimpleObjectPathResponseHandler();
    ~SimpleObjectPathResponseHandler();

    virtual void processing();

    virtual void complete();

    virtual Uint32 size() const;

    virtual void clear();

    virtual void deliver(const CIMObjectPath& objectPath);

    virtual void deliver(const SCMOInstance& objectPath);

    virtual void deliver(const Array<CIMObjectPath>& objectPaths);

    const Array<CIMObjectPath> getObjects() const;
    const Array<SCMOInstance> getSCMOObjects() const;

private:
    Array<CIMObjectPath> _objects;
    Array<SCMOInstance> _scmoObjects;
    //CIMInstanceNamesResponseData responseData;
};

class PEGASUS_PPM_LINKAGE SimpleMethodResultResponseHandler :
    public SimpleResponseHandler, public MethodResultResponseHandler
{
public:
    SimpleMethodResultResponseHandler();

    virtual void processing();

    virtual void complete();

    // NOTE: this is the total size (count) of ALL parameters!
    virtual Uint32 size() const;

    virtual void clear();

    virtual void deliverParamValue(const CIMParamValue& outParamValue);

    virtual void deliverParamValue(const Array<CIMParamValue>& outParamValues);

    virtual void deliver(const CIMValue& returnValue);

    const Array<CIMParamValue> getParamValues() const;

    const CIMValue getReturnValue() const;

private:
    Array<CIMParamValue> _objects;

    CIMValue _returnValue;

};

class PEGASUS_PPM_LINKAGE SimpleIndicationResponseHandler :
    public SimpleResponseHandler, public IndicationResponseHandler
{
public:
    SimpleIndicationResponseHandler();

    virtual void processing();

    virtual void complete();

    virtual Uint32 size() const;

    virtual void clear();

    virtual void deliver(const CIMIndication& indication);

    virtual void deliver(const Array<CIMIndication>& indications);

    virtual void deliver(
        const OperationContext& context,
        const CIMIndication& indication);

    virtual void deliver(
        const OperationContext& context,
        const Array<CIMIndication>& indications);

    const Array<CIMIndication> getObjects() const;

    // ATTN: why is this variable public?
    CIMInstance _provider;

private:
    Array<CIMIndication> _objects;

};

class PEGASUS_PPM_LINKAGE SimpleObjectResponseHandler :
    public SimpleResponseHandler, public ObjectResponseHandler
{
public:
    SimpleObjectResponseHandler();

    virtual void processing();

    virtual void complete();

    virtual Uint32 size() const;

    virtual void clear();

    virtual void deliver(const CIMObject& object);

    virtual void deliver(const SCMOInstance& object);

    virtual void deliver(const CIMInstance& instance);

    virtual void deliver(const Array<CIMObject>& objects);

    const Array<CIMObject> getObjects() const;
    const Array<SCMOInstance> getSCMOObjects() const;

private:
    Array<CIMObject> _objects;
    Array<SCMOInstance> _scmoObjects;
};

class PEGASUS_PPM_LINKAGE SimpleInstance2ObjectResponseHandler :
    public SimpleResponseHandler, public InstanceResponseHandler
{
public:
    SimpleInstance2ObjectResponseHandler();

    virtual void processing();

    virtual void complete();

    virtual Uint32 size() const;

    virtual void clear();

    virtual void deliver(const CIMInstance& object);

    virtual void deliver(const SCMOInstance& object);

    virtual void deliver(const Array<CIMInstance>& objects);

    const Array<CIMObject> getObjects() const;
    const Array<SCMOInstance> getSCMOObjects() const;

private:
    Array<CIMObject> _objects;
    Array<SCMOInstance> _scmoObjects;
};

class PEGASUS_PPM_LINKAGE SimpleValueResponseHandler :
    public SimpleResponseHandler, public ValueResponseHandler
{
public:
    SimpleValueResponseHandler();

    virtual void processing();

    virtual void complete();

    virtual Uint32 size() const;

    virtual void clear();

    virtual void deliver(const CIMValue& value);

    virtual void deliver(const Array<CIMValue>& values);

    const Array<CIMValue> getObjects() const;

private:
    Array<CIMValue> _objects;

};

PEGASUS_NAMESPACE_END

#endif
