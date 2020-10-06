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

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <Pegasus/Common/Config.h>
#include "ParserTestProvider.h"

PEGASUS_NAMESPACE_BEGIN
PEGASUS_USING_STD;

ParserTestProvider::ParserTestProvider(void)
{
}

ParserTestProvider::~ParserTestProvider(void)
{
}

Boolean ParserTestProvider::tryTerminate(void) {

    //DEBUG("tryTerminate called");
    return false;
}

void ParserTestProvider::initialize(CIMOMHandle & cimom)
{
    cout << "ParserTestProvider::initialize" << endl;

    CIMInstance instance1("Sample_PClassOne");
    CIMObjectPath reference1("Sample_PClassOne.CName=ClassOne");
    // keys
    instance1.addProperty(CIMProperty("CName", String("Sample_PClassOne")));
    instance1.addProperty(CIMProperty("CNum", Uint16(1)));
    instance1.addProperty(CIMProperty("BadStringName", String(BADSTR_1)));

    //other
    instance1.addProperty(CIMProperty("BadStringNum", Uint16(1)));

    _instances.append(instance1);
    _instanceNames.append(reference1);

    CIMInstance instance2("Sample_PClassOne");
    CIMObjectPath reference2("Sample_PClassOne.CName=ClassTwo");

    // keys
    instance2.addProperty(CIMProperty("CName", String("Sample_PClassOne")));
    instance2.addProperty(CIMProperty("CNum", Uint16(2)));
    instance2.addProperty(CIMProperty("BadStringName", String(BADSTR_2)));

    // other
    instance2.addProperty(CIMProperty("BadStringNum", Uint16(2)));

    _instances.append(instance2);
    _instanceNames.append(reference2);
}

void ParserTestProvider::terminate(void)
{
    //DEBUG("terminte()");
}

void ParserTestProvider::getInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    String Value;
    char csname[256];
    int cnum;
    char badstrname[256];

    cout << "ParserTestProvider::getInstance" << endl;

    CIMName className = instanceReference.getClassName();
    CIMNamespaceName nameSpace = instanceReference.getNameSpace();

    Array<CIMKeyBinding> keys = instanceReference.getKeyBindings();

    for (int i = 0; i < (int)keys.size(); i++)
    {
        CIMName cimName = keys[i].getName();
        Value = keys[i].getValue();

        if (cimName.equal("BadStringName")) {
                strncpy(badstrname, Value.getCString(), 256);
        }
    }
    CIMInstance instance(CLASS_NAME);
    instance.setPath(CIMObjectPath(String::EMPTY, // hostname
                                      nameSpace,
                                      CLASS_NAME,
                                      keys));
    instance.addProperty(CIMProperty("CName",
                         String(CLASS_NAME)));
    instance.addProperty(CIMProperty("BadStringName",
                         String(badstrname)));

    //begin processing the request
    handler.processing();

    handler.deliver(instance);
    // complete processing the request
    handler.complete();
}

void ParserTestProvider::enumerateInstances(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    const Boolean includeQualifiers,
    const Boolean includeClassOrigin,
    const CIMPropertyList & propertyList,
    InstanceResponseHandler & handler)
{
    // begin processing the request

    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();
    //cout "[className: "<<className<<"], [in namespace: " << nameSpace << "]";

    handler.processing();
    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("CName",
                              String(CLASS_NAME),
                              CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("BadStringName",
                              String(BADSTR_1),
                              CIMKeyBinding::STRING));
    CIMInstance instance(CLASS_NAME);

    instance.setPath(CIMObjectPath(String::EMPTY,
                                   nameSpace,
                                   CLASS_NAME,
                                   keys));
    // keys
    instance.addProperty(CIMProperty("CName",
                                      String(CLASS_NAME)));
    instance.addProperty(CIMProperty("BadStringName",
                                      String(BADSTR_1)));

    for(Uint32 i = 0, n = _instances.size(); i < n; i++)
        handler.deliver(_instances[i]);
                        //
    handler.deliver(instance);
    handler.complete();
}

void ParserTestProvider::enumerateInstanceNames(
    const OperationContext & context,
    const CIMObjectPath & classReference,
    ObjectPathResponseHandler & handler)
{
    //DEBUG("enumerateInstanceNames()");

    // begin processing the request
    CIMName className = classReference.getClassName();
    CIMNamespaceName nameSpace = classReference.getNameSpace();
    //DEBUG("[className: "<<className<<"], [in namespace: "<<nameSpace<<"]");

    handler.processing();

    Array<CIMKeyBinding> keys;

    keys.append(CIMKeyBinding("CName",
                    String(CLASS_NAME),
                    CIMKeyBinding::STRING));
    keys.append(CIMKeyBinding("BadStr",
                    String(BADSTR_1),
                    CIMKeyBinding::STRING));

    CIMObjectPath obj_path = CIMObjectPath(String(),
                                nameSpace,
                                className,
                                keys);
    handler.deliver(obj_path);
    // complete processing the request
    handler.complete();
}

void ParserTestProvider::modifyInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    const Boolean includeQualifiers,
    const CIMPropertyList & propertyList,
    ResponseHandler & handler)
{
    //DEBUG("modifyInstance()");
    throw CIMNotSupportedException(String::EMPTY);
}

void ParserTestProvider::createInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    const CIMInstance & instanceObject,
    ObjectPathResponseHandler & handler)
{
    //DEBUG("createInstance");
    throw CIMNotSupportedException(String::EMPTY);

}

void ParserTestProvider::deleteInstance(
    const OperationContext & context,
    const CIMObjectPath & instanceReference,
    ResponseHandler & handler)
{
    //DEBUG("deleteInstance()");
    throw CIMNotSupportedException(String::EMPTY);

}

//const CIMDateTime *
//ParserTestProvider::_convert(time_t inTime)
//{
//   CIMDateTime *dt=new CIMDateTime();
//   char strTime[256];
//   char utcOffset[20];
//   //char usTime[32];
//   struct tm tmTime;

//   localtime_r(&inTime,&tmTime);
//   if (strftime(strTime,256,"%Y%m%d%H%M%S.",&tmTime)) {
//      strcat(strTime,"000000");
//      snprintf(utcOffset,20,"%+4.3ld",tmTime.tm_gmtoff/60);
//      strcat(strTime,utcOffset);
//      //DEBUG("Time is "<<strTime);
//      *dt=String(strTime);
//   }

//   return dt;
//}

PEGASUS_NAMESPACE_END
