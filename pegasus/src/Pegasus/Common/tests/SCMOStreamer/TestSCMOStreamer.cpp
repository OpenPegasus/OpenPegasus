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

#include <Pegasus/Common/XmlReader.h>
#include <Pegasus/Common/XmlParser.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/SCMOClassCache.h>
#include <Pegasus/Common/CIMBuffer.h>
#include <Pegasus/Common/SCMOStreamer.h>


PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

#define VCOUT if (verbose) cout

static Boolean verbose;
static Boolean loadClassOnce;

const String TESTSCMO2XML("/src/Pegasus/Common/tests/SCMOStreamer/");

#define TEST_INSTANCE_ID "my_instance_id"
#define TEST_ERROR_SOURCE "my_error_source"
#define TEST_PRE_METHOD_NAME "my_pre_method_name"
#define TEST_POST_METHOD_NAME "my_post_method_name"

SCMOClass _scmoClassCache_GetClass(
    const CIMNamespaceName& nameSpace,
    const CIMName& className)
{
    CIMClass cl;
    Buffer text;

    VCOUT << endl << "Loading class " << className.getString() << endl;

    String filename (getenv("PEGASUS_ROOT"));
    filename.append(TESTSCMO2XML);
    filename.append(className.getString());
    filename.append(".xml");

    FileSystem::loadFileToMemory(text,(const char*)filename.getCString());

    XmlParser theParser((char*)text.getData());
    XmlReader::getObject(theParser,cl);

    // The class was loaded.
    loadClassOnce = true;

    VCOUT << endl << "Done." << endl;

    return SCMOClass(
        cl, (const char*)nameSpace.getString().getCString());
}


void SCMOInstanceConverterTest()
{

    CIMClass CIM_CSClass;
    CIMInstance CIM_CSInstance;
    Buffer text;
    CIMNamespaceName nm("root/cimv2");

    VCOUT << endl << "SCMOStreamer Test.." << endl;

    VCOUT << endl << "Loading classes" << endl;
    CIMName CIM_MethodResultName("CIM_MethodResult");
    SCMOClass CIM_MethodResult = _scmoClassCache_GetClass(
        nm,
        CIM_MethodResultName);

    CIMName CIM_InstMethodCallName("CIM_InstMethodCall");
    SCMOClass CIM_InstMethodCall = _scmoClassCache_GetClass(
        nm,
        CIM_InstMethodCallName);

    CIMName CIM_ErrorName("CIM_Error");
    SCMOClass CIM_Error = _scmoClassCache_GetClass(nm, CIM_ErrorName);

    VCOUT << endl << "Creating CIM_Error instance" << endl;
    SCMOInstance error = SCMOInstance(CIM_Error);
    SCMBUnion val;
    val.extString.pchar = (char *) TEST_ERROR_SOURCE;
    val.extString.length = strlen(val.extString.pchar);
    error.setPropertyWithOrigin("ErrorSource", CIMTYPE_STRING, &val);

    VCOUT << endl << "Creating PreCall instance" << endl;
    SCMOInstance preCall = SCMOInstance(CIM_InstMethodCall);
    val.extString.pchar  = (char *) TEST_PRE_METHOD_NAME;
    val.extString.length = strlen(val.extString.pchar);
    preCall.setPropertyWithOrigin("MethodName", CIMTYPE_STRING, &val);

    VCOUT << endl << "Creating PostCall instance" << endl;
    SCMOInstance postCall = SCMOInstance(CIM_InstMethodCall);
    val.extString.pchar  = TEST_POST_METHOD_NAME;
    val.extString.length = strlen(val.extString.pchar);
    postCall.setPropertyWithOrigin("MethodName", CIMTYPE_STRING, &val);
    val.extRefPtr = &error;
    postCall.setPropertyWithOrigin("Error", CIMTYPE_INSTANCE, &val, true, 1);

    VCOUT << endl << "Creating MethodResult instance" << endl;
    SCMOInstance methodResult = SCMOInstance(CIM_MethodResult);
    val.extString.pchar  = (char *) TEST_INSTANCE_ID;
    val.extString.length = strlen(val.extString.pchar);
    methodResult.setPropertyWithOrigin("InstanceID", CIMTYPE_STRING, &val);
    val.extRefPtr = &preCall;
    methodResult.setPropertyWithOrigin(
        "PreCallIndication",
        CIMTYPE_INSTANCE,
        &val);
    val.extRefPtr = &postCall;
    methodResult.setPropertyWithOrigin(
        "PostCallIndication",
        CIMTYPE_INSTANCE,
        &val);

    VCOUT << "serialize" << endl;

    Array<SCMOInstance> in;
    in.append(methodResult);

    CIMBuffer buf(4096);
    buf.putSCMOInstanceA(in);
    VCOUT << "serialize took " << buf.size() << endl;

    VCOUT << "deserialize" << endl;
    Array<SCMOInstance> out;
    buf.rewind();
    bool ret = buf.getSCMOInstanceA(out);
    VCOUT << "deserialize returned " << ret << endl;
    VCOUT << "out size " << out.size() << endl;

    SCMOInstance &methodResult2 = out[0];

    CIMType type;
    const SCMBUnion *pval;
    Boolean isArray;
    Uint32 size;

    //
    methodResult2.getProperty("InstanceID", type, &pval, isArray, size);
    PEGASUS_TEST_ASSERT(pval != NULL);
    VCOUT << "Got Instance ID: " << pval->extString.pchar << endl;
    PEGASUS_TEST_ASSERT(strcmp(pval->extString.pchar, TEST_INSTANCE_ID) == 0);

    // check PreCallIndication
    methodResult2.getProperty("PreCallIndication", type, &pval, isArray, size);
    PEGASUS_TEST_ASSERT(pval != NULL);
    SCMOInstance *preCall2 = pval->extRefPtr;

    preCall2->getProperty("MethodName", type, &pval, isArray, size);
    PEGASUS_TEST_ASSERT(pval != NULL);
    VCOUT << "Got PreCallIndication.MethodName: " <<
        pval->extString.pchar << endl;
    PEGASUS_TEST_ASSERT(
        strcmp(pval->extString.pchar, TEST_PRE_METHOD_NAME) == 0);

    // check PostCallIndication
    methodResult2.getProperty("PostCallIndication", type, &pval, isArray, size);
    PEGASUS_TEST_ASSERT(pval != NULL);
    SCMOInstance *postCall2 = pval->extRefPtr;

    postCall2->getProperty("MethodName", type, &pval, isArray, size);
    PEGASUS_TEST_ASSERT(pval != NULL);
    VCOUT << "Got PostCallIndication.MethodName: " << pval->extString.pchar <<
        endl;
    PEGASUS_TEST_ASSERT(
        strcmp(pval->extString.pchar, TEST_POST_METHOD_NAME) == 0);

    // check error
    postCall2->getProperty("Error", type, &pval, isArray, size);
    PEGASUS_TEST_ASSERT(pval != NULL);
    SCMOInstance *error2 = pval->extRefPtr;

    error2->getProperty("ErrorSource", type, &pval, isArray, size);
    PEGASUS_TEST_ASSERT(pval != NULL);
    VCOUT << "Got Error[0].ErrorSource: " << pval->extString.pchar << endl;
    PEGASUS_TEST_ASSERT(strcmp(pval->extString.pchar, TEST_ERROR_SOURCE) == 0);

    VCOUT << endl << "Done." << endl << endl;
}


int main (int argc, char *argv[])
{

    CIMClass CIM_TESTClass2;

    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

    try
    {
        // init the cache.
        SCMOClassCache* _thecache = SCMOClassCache::getInstance();
        _thecache->setCallBack(_scmoClassCache_GetClass);

        SCMOInstanceConverterTest();

         //destroy the cache.
        _thecache->destroy();
    }
    catch (CIMException& e)
    {
        cout << endl << "CIMException: " ;
        cout << e.getMessage() << endl << endl ;
        exit(-1);
    }

    catch (Exception& e)
    {
        cout << endl << "Exception: " ;
        cout << e.getMessage() << endl << endl ;
        exit(-1);
    }
    catch (...)
    {
        cout << endl << "Unkown exception!" << endl << endl;
        exit(-1);
    }

    cout << argv[0] << " +++++ passed all tests" << endl;
    return 0;

}
