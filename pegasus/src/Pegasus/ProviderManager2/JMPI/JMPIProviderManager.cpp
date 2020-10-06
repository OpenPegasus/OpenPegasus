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
#include "JMPIProviderManager.h"

#include "JMPIImpl.h"

#include <Pegasus/Common/CIMMessage.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/MessageLoader.h> //l10n
#include <Pegasus/Common/Constants.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/ArrayInternal.h>

#include <Pegasus/Config/ConfigManager.h>

#include <Pegasus/ProviderManager2/ProviderName.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProvider.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProviderModule.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

// request->localOnly is replaced with JMPI_LOCALONLY for
// getInstance () and enumerateInstances ()
#define JMPI_LOCALONLY false

/* Fix for 4092 */
// request->includeQualifiers is replaced with JMPI_INCLUDE_QUALIFIERS
// for getInstance (), setInstance (), enumerateInstances (), associators ()
// and references ()
#define JMPI_INCLUDE_QUALIFIERS false

#include "Convert.h"


//
//  The following tests were added to allow the provider to supply a
//  CIMObjectPath in the response.  Before this, the path was always
//  rebuild by the response functions from the instance.  This resulted
//  in cases where the key properties were not in the returned instance
//  (ex. excluded by the property list) so that the path could not be built.
//  The preferred behavior is for the provider to build the path on response.
//  However, to cover existing providers the alternate of allowing the
//  ProviderManager has been kept if the path in the instance is not considered
//  to exist.
//  PLEASE: provider writers build/set a proper path into returned objects
//  See bug 8062

/*
    local function to implement the CIMObjectPath required for a response
    object.
    If there is a path defined in the instance, we simply return this path.
    If not, we use the class to build a path from the instance.
    Note that we are depending on some value in the keybindings to determine
    if we have a path.
*/

void _fixCIMObjectPath(CIMInstance* instance, CIMClass& cls)
{
    CIMObjectPath op = instance->getPath();
    const Array<CIMKeyBinding> kb = op.getKeyBindings();
    if (kb.size() == 0)
    {
        CIMObjectPath iop = instance->buildPath(cls);
        /* Fix for 4237*/
        iop.setNameSpace(op.getNameSpace());
        instance->setPath(iop);
    }
}

void JMPIProviderManager::debugPrintMethodPointers (JNIEnv *env, jclass jc)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::debugPrintMethodPointers");

   static const char *methodNames[][3] = {
       // CIMProvider
      {"snia 2.0",
       "initialize",
       "(Lorg/pegasus/jmpi/CIMOMHandle;)V"},
      {"snia 2.0",
       "cleanup",
       "()V"},
      // InstanceProvider
      {"snia 2.0",
       "enumInstances",
       "(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;"
          "Z)Ljava/util/Vector;"},
      {"pegasus 2.4",
       "enumInstances",
       "(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ[Ljava/lang/String;"
          "Lorg/pegasus/jmpi/CIMClass;)[Lorg/pegasus/jmpi/CIMInstance;"},
      /* Begin Fix for 4189 */
      {"pegasus 2.5",
       "enumerateInstances",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;"
          "ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      /* End Fix for 4189 */
      {"snia 2.0",
       "enumInstances",
       "(Lorg/pegasus/jmpi/CIMObjectPath;ZLorg/pegasus/jmpi/CIMClass;)"
          "Ljava/util/Vector;"},
      {"pegasus 2.4",
       "enumerateInstanceNames",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;)"
          "[Lorg/pegasus/jmpi/CIMObjectPath;"},
      {"pegasus 2.5",
       "enumerateInstanceNames",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;)"
          "Ljava/util/Vector;"},
      {"snia 2.0",
       "getInstance",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;Z)"
          "Lorg/pegasus/jmpi/CIMInstance;"},
      {"pegasus 2.4",
       "getInstance",
       "(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ[Ljava/lang/String;"
          "Lorg/pegasus/jmpi/CIMClass;)Lorg/pegasus/jmpi/CIMInstance;"},
      /* Begin Fix for 4238 */
      {"pegasus 2.5",
       "getInstance",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;"
          "ZZ[Ljava/lang/String;)Lorg/pegasus/jmpi/CIMInstance;"},
      /* End Fix for 4238 */
      {"snia 2.0",
       "createInstance",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)"
          "Lorg/pegasus/jmpi/CIMObjectPath;"},
      {"snia 2.0",
       "setInstance",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMInstance;)V"},
      {"pegasus 2.4",
       "setInstance",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Z[Ljava/lang/String)V"},
      {"snia 2.0",
       "deleteInstance",
       "(Lorg/pegasus/jmpi/CIMObjectPath;)V"},
      {"snia 2.0",
       "execQuery",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
          "ILorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;"},
      {"pegasus 2.4",
       "execQuery",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
          "Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)"
              "[Lorg/pegasus/jmpi/CIMInstance;"},
      {"pegasus 2.5",
       "execQuery",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMClass;"
          "Ljava/lang/String;Ljava/lang/String;)Ljava/util/Vector;"},
      // MethodProvider
      {"snia 2.0",
       "invokeMethod",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
          "Ljava/util/Vector;Ljava/util/Vector;)Lorg/pegasus/jmpi/CIMValue;"},
      {"pegasus 2.4",
       "invokeMethod",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
          "[Lorg/pegasus/jmpi/CIMArgument;[Lorg/pegasus/jmpi/CIMArgument;)"
          "Lorg/pegasus/jmpi/CIMValue;"},
      // PropertyProvider
      {"snia 2.0",
       "getPropertyValue",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;)"
          "Lorg/pegasus/jmpi/CIMValue;"},
      {"snia 2.0",
       "setPropertyValue",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;"
          "Lorg/pegasus/jmpi/CIMValue;)V"},
      // AssociatorProvider
      {"snia 2.0",
       "associators",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;"
          "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
             "ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"pegasus 2.4",
       "associators",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;"
          "Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"snia 2.0",
       "associatorNames",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;"
          "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)"
             "Ljava/util/Vector;"},
      {"pegasus 2.4",
       "associatorNames",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;Ljava/lang/String;"
          "Ljava/lang/String;)Ljava/util/Vector;"},
      {"snia 2.0",
       "references",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;"
          "Ljava/lang/String;ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"pegasus 2.4",
       "references",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
          "ZZ[Ljava/lang/String;)Ljava/util/Vector;"},
      {"snia 2.0",
       "referenceNames",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Lorg/pegasus/jmpi/CIMObjectPath;"
          "Ljava/lang/String;)Ljava/util/Vector;"},
      {"pegasus 2.4",
       "referenceNames",
       "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)"
          "Ljava/util/Vector;"},
      // CIMProviderRouter
      // EventProvider
      {"snia 2.0",
       "activateFilter",
       "(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;"
          "Lorg/pegasus/jmpi/CIMObjectPath;Z)V"},
      {"snia 2.0",
       "deActivateFilter",
       "(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;"
          "Lorg/pegasus/jmpi/CIMObjectPath;Z)V"},
      // IndicationHandler
      // ProviderAdapter
      // JMPI_TestPropertyTypes
      {"JMPI_TestPropertyTypes",
       "findObjectPath",
       "(Lorg/pegasus/jmpi/CIMObjectPath;)I"},
      {"JMPI_TestPropertyTypes",
       "testPropertyTypesValue",
       "(Lorg/pegasus/jmpi/CIMInstance;)V"}
   };

   if (!env)
   {
      PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "No JNI environment specified. (JNIEnv==NULL)");
      PEG_METHOD_EXIT();
      return;
   }
   if (!jc)
   {
      PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "No Java class specified. (jc==NULL)");
      PEG_METHOD_EXIT();
      return;
   }

   for (int i = 0; i < (int)(sizeof (methodNames)/sizeof (methodNames[0])); i++)
   {
      jmethodID id = env->GetMethodID(jc,methodNames[i][1], methodNames[i][2]);
      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "Method: %s, %s, id = %p",
          methodNames[i][0],methodNames[i][1],id));

      env->ExceptionClear();
   }

   env->ExceptionClear();
   PEG_METHOD_EXIT();
   return;
}

void debugIntrospectJavaObject (JNIEnv *env, jobject jInst)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::debugIntrospectJavaObject");

   jclass       jInstClass             = env->GetObjectClass(jInst);
   jclass       jInstSuperClass        = env->GetSuperclass(jInstClass);
   jmethodID    jmidGetDeclaredMethods = env->GetMethodID(
                                             jInstClass,
                                             "getDeclaredMethods",
                                             "()[Ljava/lang/reflect/Method;");

   if (!jmidGetDeclaredMethods)
   {
      env->ExceptionClear();
      jmidGetDeclaredMethods = env->GetMethodID(
                                   jInstSuperClass,
                                   "getDeclaredMethods",
                                   "()[Ljava/lang/reflect/Method;");
   }

   if (jmidGetDeclaredMethods)
   {
      jobjectArray jarrayDeclaredMethods = (jobjectArray)env->CallObjectMethod(
                                               jInstClass,
                                               jmidGetDeclaredMethods);

      if (jarrayDeclaredMethods)
      {
         for (int i=0,iLen=env->GetArrayLength(jarrayDeclaredMethods);
               i < iLen;
               i++)
         {
            JMPIjvm::checkException(env);

            jobject jDeclaredMethod = env->GetObjectArrayElement(
                                               jarrayDeclaredMethods,
                                               i);
            jclass jDeclaredMethodClass = env->GetObjectClass(jDeclaredMethod);

            JMPIjvm::checkException(env);

            jmethodID   jmidToString  = env->GetMethodID(
                                            jDeclaredMethodClass,
                                            "toString",
                                            "()Ljava/lang/String;");
            jstring     jstringResult = (jstring)env->CallObjectMethod(
                                            jDeclaredMethod,
                                            jmidToString);
            const char *pszResult = env->GetStringUTFChars(jstringResult, 0);

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "Introspect java object: %s",pszResult));

            env->ReleaseStringUTFChars (jstringResult, pszResult);
         }
      }
   }

   env->ExceptionClear();
   PEG_METHOD_EXIT();
   return;
}

void debugDumpJavaObject (JNIEnv *env, jobject jInst)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
       "JMPIProviderManager::debugDumpJavaObject");
   jclass jInstClass = env->GetObjectClass(jInst);
   jclass jInstSuperClass = env->GetSuperclass(jInstClass);
   jmethodID jmidToString1 = env->GetMethodID(
                                jInstClass,
                                "toString",
                                "()Ljava/lang/String;");
   jmethodID jmidToString2 = env->GetMethodID(
                                jInstSuperClass,
                                "toString",
                                "()Ljava/lang/String;");
   if (!jmidToString1 || !jmidToString2)
   {
      PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
           "No class or super class description.");
      env->ExceptionClear();
      PEG_METHOD_EXIT();
      return;
   }
   jstring jstringResult1 = (jstring)env->CallObjectMethod(
                                jInstClass,
                                jmidToString1);
   jstring jstringResult2 = (jstring)env->CallObjectMethod(
                                jInstSuperClass,
                                jmidToString2);
   jstring jstringResult3 = (jstring)env->CallObjectMethod(
                                jInst,
                                jmidToString1);
   const char *pszResult1      = env->GetStringUTFChars(jstringResult1, 0);
   const char *pszResult2      = env->GetStringUTFChars(jstringResult2, 0);
   const char *pszResult3      = env->GetStringUTFChars(jstringResult3, 0);

   jmethodID jmidCInst = env->GetMethodID(
                             env->GetObjectClass(jInst),
                             JMPIjvm::jv.instanceMethodNames[22].methodName,
                             JMPIjvm::jv.instanceMethodNames[22].signature);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
       "Dumping class %p:\n "
           "    jInstSuperClass = %p\n"
           "    jClassShouldBe = %p\n"
           "    jmidCInst = %p\n"
           "    pszResult1 = %s\n"
           "    pszResult2 = %s\n"
           "    pszResult3 = %s",
       jInstClass,jInstSuperClass,JMPIjvm::jv.classRefs[18],jmidCInst,
       pszResult1,pszResult2,pszResult3 ));

   env->ReleaseStringUTFChars (jstringResult1, pszResult1);
   env->ReleaseStringUTFChars (jstringResult2, pszResult2);
   env->ReleaseStringUTFChars (jstringResult3, pszResult3);

   env->ExceptionClear();
   PEG_METHOD_EXIT();
   return;
}

bool JMPIProviderManager::getInterfaceType(
    ProviderIdContainer pidc,
    String& interfaceType,
    String& interfaceVersion)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::getInterfaceType");

   CIMInstance ciProviderModule = pidc.getModule ();
   Uint32      idx;
   bool        fRet             = true;

   idx = ciProviderModule.findProperty ("InterfaceType");

   if (idx != PEG_NOT_FOUND)
   {
      CIMValue itValue;

      itValue = ciProviderModule.getProperty (idx).getValue ();

      itValue.get (interfaceType);

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "interfaceType = %s",(const char*)interfaceType.getCString()));
   }
   else
   {
      PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
          "inferfaceType not found.");
      fRet = false;
   }

   idx = ciProviderModule.findProperty ("InterfaceVersion");

   if (idx != PEG_NOT_FOUND)
   {
      CIMValue itValue;

      itValue = ciProviderModule.getProperty (idx).getValue ();

      itValue.get (interfaceVersion);

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "interfaceVersion = %s",(const char*)interfaceVersion.getCString()));
   }
   else
   {
      PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
          "inferfaceVersion not found.");
      fRet = false;
   }
   PEG_METHOD_EXIT();
   return fRet;
}

bool JMPIProviderManager::interfaceIsUsed (JNIEnv  *env,
                                           jobject  jObject,
                                           String   searchInterfaceName)
{
   jobjectArray jInterfaces       = 0;
   jsize        jInterfacesLength = 0;
   bool         fFound            = false;

   if (!jObject)
   {
      return false;
   }

   jInterfaces = (jobjectArray)env->CallObjectMethod(
                     env->GetObjectClass (jObject),
                    JMPIjvm::jv.ClassGetInterfaces);

   if (!jInterfaces)
   {
      return false;
   }

   jInterfacesLength = env->GetArrayLength (jInterfaces);

   for (jsize i = 0; !fFound && i < jInterfacesLength; i++)
   {
      jobject jInterface = env->GetObjectArrayElement (jInterfaces, i);

      if (jInterface)
      {
         jstring jInterfaceName = (jstring)env->CallObjectMethod(
                                      jInterface,
                                      JMPIjvm::jv.ClassGetName);

         if (jInterfaceName)
         {
            const char *pszInterfaceName = env->GetStringUTFChars(
                                               jInterfaceName,
                                               0);
            String      interfaceName    = pszInterfaceName;

            if (String::equal (interfaceName, searchInterfaceName))
            {
               fFound = true;
            }

            env->ReleaseStringUTFChars (jInterfaceName, pszInterfaceName);
         }
      }
   }

   return fFound;
}

JMPIProviderManager::IndProvTab    JMPIProviderManager::provTab;
Mutex                              JMPIProviderManager::mutexProvTab;
JMPIProviderManager::IndSelectTab  JMPIProviderManager::selxTab;
Mutex                              JMPIProviderManager::mutexSelxTab;
JMPIProviderManager::ProvRegistrar JMPIProviderManager::provReg;
Mutex                              JMPIProviderManager::mutexProvReg;

JMPIProviderManager::JMPIProviderManager()
{
   _subscriptionInitComplete = false;
}

JMPIProviderManager::~JMPIProviderManager(void)
{
}

Boolean JMPIProviderManager::insertProvider(const ProviderName & name,
            const String &ns, const String &cn)
{
    PEG_METHOD_ENTER( TRC_PROVIDERMANAGER,
        "JMPIProviderManager::insertProvider");

    String key(ns + String("::") + cn);
    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
        "insertProvider: %s",(const char*)key.getCString()));

    Boolean ret = false;

    {
       AutoMutex lock (mutexProvReg);

       ret = provReg.insert(key,name);
    }

    PEG_METHOD_EXIT();
    return ret;
}

Message * JMPIProviderManager::processMessage(Message * request) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::processMessage()");

    Message * response = 0;

    // pass the request message to a handler method based on message type
    switch(request->getType())
    {
    case CIM_GET_INSTANCE_REQUEST_MESSAGE:
        response = handleGetInstanceRequest(request);
        break;

    case CIM_ENUMERATE_INSTANCES_REQUEST_MESSAGE:
        response = handleEnumerateInstancesRequest(request);
        break;

    case CIM_ENUMERATE_INSTANCE_NAMES_REQUEST_MESSAGE:
        response = handleEnumerateInstanceNamesRequest(request);
        break;

    case CIM_CREATE_INSTANCE_REQUEST_MESSAGE:
        response = handleCreateInstanceRequest(request);
        break;

    case CIM_MODIFY_INSTANCE_REQUEST_MESSAGE:
        response = handleModifyInstanceRequest(request);
        break;

    case CIM_DELETE_INSTANCE_REQUEST_MESSAGE:
        response = handleDeleteInstanceRequest(request);
        break;

    case CIM_EXEC_QUERY_REQUEST_MESSAGE:
        response = handleExecQueryRequest(request);
        break;

    case CIM_ASSOCIATORS_REQUEST_MESSAGE:
        response = handleAssociatorsRequest(request);
        break;

    case CIM_ASSOCIATOR_NAMES_REQUEST_MESSAGE:
        response = handleAssociatorNamesRequest(request);
        break;

    case CIM_REFERENCES_REQUEST_MESSAGE:
        response = handleReferencesRequest(request);
        break;

    case CIM_REFERENCE_NAMES_REQUEST_MESSAGE:
        response = handleReferenceNamesRequest(request);
        break;

    case CIM_GET_PROPERTY_REQUEST_MESSAGE:
        response = handleGetPropertyRequest(request);
        break;

    case CIM_SET_PROPERTY_REQUEST_MESSAGE:
        response = handleSetPropertyRequest(request);
        break;

    case CIM_INVOKE_METHOD_REQUEST_MESSAGE:
        response = handleInvokeMethodRequest(request);
        break;

    case CIM_CREATE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleCreateSubscriptionRequest(request);
        break;

/*  case CIM_MODIFY_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleModifySubscriptionRequest(request);
        break;
*/
    case CIM_DELETE_SUBSCRIPTION_REQUEST_MESSAGE:
        response = handleDeleteSubscriptionRequest(request);
        break;

/*  case CIM_EXPORT_INDICATION_REQUEST_MESSAGE:
        response = handleExportIndicationRequest(request);
        break;
*/
    case CIM_DISABLE_MODULE_REQUEST_MESSAGE:
        response = handleDisableModuleRequest(request);
        break;

    case CIM_ENABLE_MODULE_REQUEST_MESSAGE:
        response = handleEnableModuleRequest(request);
        break;

    case CIM_STOP_ALL_PROVIDERS_REQUEST_MESSAGE:
        response = handleStopAllProvidersRequest(request);
        break;

    case CIM_SUBSCRIPTION_INIT_COMPLETE_REQUEST_MESSAGE:
        response = handleSubscriptionInitCompleteRequest (request);
        break;

    case CIM_INDICATION_SERVICE_DISABLED_REQUEST_MESSAGE:
        response = handleIndicationServiceDisabledRequest (request);
        break;

    default:
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "Unsupported Request %d",
                   request->getType()
                  ));

        response = handleUnsupportedRequest(request);
        break;
    }

    PEG_METHOD_EXIT();

    return(response);
}

Boolean JMPIProviderManager::hasActiveProviders()
{
     return providerManager.hasActiveProviders();
}

void JMPIProviderManager::unloadIdleProviders()
{
     providerManager.unloadIdleProviders();
}

#define STRDUPA(s,o) \
   if (s) { \
      o=(const char*)alloca(strlen(s)); \
      strcpy((char*)(o),(s)); \
   } \
   else o=NULL;

#define CHARS(cstring) (char*)(strlen(cstring)?(const char*)cstring:NULL)


#define HandlerIntroBase(type,type1,message,request,response,handler) \
    CIM##type##RequestMessage * request = \
        dynamic_cast<CIM##type##RequestMessage *>(const_cast<Message *> \
        (message)); \
    PEGASUS_ASSERT(request != 0); \
    CIM##type##ResponseMessage * response = \
        dynamic_cast<CIM##type##ResponseMessage*>(request->buildResponse()); \
    PEGASUS_ASSERT(response != 0); \
    type1##ResponseHandler handler(request, response, _responseChunkCallback);

#define HandlerIntroInd(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler)

#define HandlerIntroInit(type,message,request,response,handler) \
     HandlerIntroBase(type,Operation,message,request,response,handler)

#define HandlerIntro(type,message,request,response,handler) \
     HandlerIntroBase(type,type,message,request,response,handler)

#define HandlerCatch(handler) \
    catch(CIMException & e)  \
    {   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"CIMException: %s", \
            (const char*)e.getMessage().getCString())); \
        handler.setStatus(e.getCode(), e.getContentLanguages(), \
        e.getMessage()); \
    } \
    catch(Exception & e) \
    {   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,"Exception: %s", \
            (const char*)e.getMessage().getCString())); \
        handler.setStatus(CIM_ERR_FAILED, e.getContentLanguages(), \
        e.getMessage()); \
    } \
    catch(...) \
    { PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1, \
                "Exception: Unknown"); \
        handler.setStatus(CIM_ERR_FAILED, "Unknown error."); \
    }

static jobjectArray getList(JvmVector *jv, JNIEnv *env, CIMPropertyList &list)
{
    jobjectArray pl= NULL;
    if (!list.isNull())
    {
        Uint32 s=list.size();
        jstring initial=env->NewString(NULL,0);
        pl=(jobjectArray)env->NewObjectArray(s,jv->StringClassRef,initial);
        for (Uint32 i=0; i<s; i++)
        {
            env->SetObjectArrayElement
                (pl,i,env->NewStringUTF(list[i].getString().getCString()));
        }
    }
    return pl;
}

Message * JMPIProviderManager::handleGetInstanceRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleGetInstanceRequest");

    HandlerIntro(GetInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2, // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
             "handleGetInstanceRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                  instanceName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->instanceName.getClassName(),
                                        request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                name.getPhysicalName(),
                                                name.getLogicalName());
        OperationContext context;

        context.insert(
            request->operationContext.get(IdentityContainer::NAME));
        context.insert(
            request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(
            request->operationContext.get(ContentLanguageListContainer::NAME));

        if (request->operationContext.contains(UserRoleContainer::NAME))
        {
            context.insert(
                request->operationContext.get(UserRoleContainer::NAME));
        }

        // forward request
        JMPIProvider &pr=ph.GetProvider();

        PEG_TRACE(( TRC_PROVIDERMANAGER, Tracer::LEVEL3,
            "handleGetInstanceRequest:Calling provider instance: %s" ,
            (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleGetInstanceRequest:"
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "getInstance",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;"
                         "Lorg/pegasus/jmpi/CIMClass;Z)"
                             "Lorg/pegasus/jmpi/CIMInstance;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleGetInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "getInstance",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ["
                            "Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)"
                                "Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                       "handleGetInstanceRequest: "
                           "Found METHOD_CIMINSTANCEPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "getInstance",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMClass;ZZ[Ljava/lang/String;)"
                                "Lorg/pegasus/jmpi/CIMInstance;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER2.");
           }
           /* Fix for 4238 */
        }

        if (id == NULL)
        {
           PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
               "handleGetInstanceRequest: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on "
                       "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong jopRef=DEBUG_ConvertCToJava(CIMObjectPath*,jlong,objectPath);
            jobject jop=env->NewObject(
                            jv->CIMObjectPathClassRef,
                            jv->CIMObjectPathNewJ,
                            jopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetInstanceRequest: "
                       "enter(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->
                       instanceName.getClassName().getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(
                         context,
                         request->nameSpace,
                         request->instanceName.getClassName(),
                         false,
                         true,
                         true,
                         CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetInstanceRequest: "
                   "exit(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->
                       instanceName.getClassName().getString().getCString()
                    ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleGetInstanceRequest: "
                   "Caught CIMExcetion(METHOD_CIMINSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->
                      instanceName.getClassName().getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcimClassRef = DEBUG_ConvertCToJava(
                                     CIMClass*,
                                     jlong,
                                     pcls);
            jobject jcimClass = env->NewObject(
                                    jv->CIMClassClassRef,
                                    jv->CIMClassNewJ,
                                    jcimClassRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobject jciRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                   id,
                                                   jop,
                                                   JMPI_LOCALONLY,
                                                   JMPI_INCLUDE_QUALIFIERS,
                                                   request->includeClassOrigin,
                                                   jPropertyList,
                                                   jcimClass);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jciRet) {
               jlong jciRetRef = env->CallLongMethod(
                                     jciRet,
                                     JMPIjvm::jv.CIMInstanceCInst);
               CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                        jlong,
                                        CIMInstance*,
                                        jciRetRef);

               handler.deliver(*ciRet);
            }
            handler.complete();
            break;
        }

        /* Fix for 4238 */
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jopRef = DEBUG_ConvertCToJava(
                               CIMObjectPath*,
                               jlong,
                               objectPath);
            jobject jop = env->NewObject(
                              jv->CIMObjectPathClassRef,
                              jv->CIMObjectPathNewJ,
                              jopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetInstanceRequest: "
                   "enter(METHOD_INSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->
                       instanceName.getClassName().getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(
                         context,
                         request->nameSpace,
                         request->instanceName.getClassName(),
                         false,
                         true,
                         true,
                         CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetInstanceRequest: "
                   "exit(METHOD_INSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->
                       instanceName.getClassName().getString().getCString()
                   ));
            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleGetInstanceRequest: "
                   "Caught CIMExcetion (METHOD_INSTANCEPROVIDER2) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->
                       instanceName.getClassName().getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));
               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass = env->NewObject(
                                    jv->CIMClassClassRef,
                                    jv->CIMClassNewJ,
                                    jcimClassRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobject jciRet = env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jop,
                                 jcimClass,
                                 JMPI_INCLUDE_QUALIFIERS,
                                 request->includeClassOrigin,
                                 jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(
                   joc,
                   JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            if (jciRet) {
               jlong jciRetRef = env->CallLongMethod(
                                     jciRet,
                                     JMPIjvm::jv.CIMInstanceCInst);
               CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                        jlong,
                                        CIMInstance*,
                                        jciRetRef);

               handler.deliver(*ciRet);
            }
            handler.complete();
            break;
        }
        /* Fix for 4238 */

        case METHOD_INSTANCEPROVIDER:
        {
            jlong jopRef = DEBUG_ConvertCToJava(
                               CIMObjectPath*,
                               jlong,
                               objectPath);
            jobject jop = env->NewObject(
                              jv->CIMObjectPathClassRef,
                              jv->CIMObjectPathNewJ,jopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
                PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                    "handleGetInstanceRequest: "
                    "enter(METHOD_INSTANCEPROVIDER): "
                        "cimom_handle->getClass(%s).",
                    (const char*)request->
                        instanceName.getClassName().getString().getCString()
                    ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(
                         context,
                         request->nameSpace,
                         request->instanceName.getClassName(),
                         false,
                         true,
                         true,
                         CIMPropertyList());
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetInstanceRequest: "
                   "exit(METHOD_INSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->
                       instanceName.getClassName().getString().getCString()
                   ));
            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleGetInstanceRequest: "
                   "Caught CIMExcetion (METHOD_INSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->
                       instanceName.getClassName().getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcimClassRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcimClass = env->NewObject(
                                    jv->CIMClassClassRef,
                                    jv->CIMClassNewJ,
                                    jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            // Modified for Bugzilla# 3679
            jobject jciRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                   id,
                                                   jop,
                                                   jcimClass,
                                                   JMPI_LOCALONLY);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jciRet) {
               jlong jciRetRef = env->CallLongMethod(
                                     jciRet,
                                     JMPIjvm::jv.CIMInstanceCInst);
               CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                        jlong,
                                        CIMInstance*,
                                        jciRetRef);

               handler.deliver(*ciRet);
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleGetInstanceRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnumerateInstancesRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleEnumerateInstanceRequest");

    HandlerIntro(EnumerateInstances,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_CIMINSTANCEPROVIDER2,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try
    {
       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
            "handleEnumerateInstanceRequest: "
                "name space = %s class name = %s",
            (const char*)request->nameSpace.getString().getCString(),
            (const char*)request->className.getString().getCString()
            ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->className);

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                name.getPhysicalName(),
                                                name.getLogicalName(),
                                                String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(
            request->operationContext.get(IdentityContainer::NAME));
        context.insert(
            request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(
            request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleEnumerateInstanceRequest: "
            "Calling provider: %s", (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleEnumerateInstanceRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get (ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "enumInstances",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;Z"
                         "Lorg/pegasus/jmpi/CIMClass;Z)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstances: Found METHOD_INSTANCEPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "enumerateInstances",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;ZZZ["
                            "Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)["
                                "Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleEnumerateInstances: "
                           "Found METHOD_CIMINSTANCEPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "enumerateInstances",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMClass;ZZ[Ljava/lang/String;)"
                                "Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstances: "
                       "Found METHOD_INSTANCEPROVIDER2.");
           }
           if (id == NULL)
           {
               env->ExceptionClear();
               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "enumerateInstances",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;"
                                "Lorg/pegasus/jmpi/CIMClass;ZZ["
                                    "Ljava/lang/String;)["
                                        "Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleEnumerateInstances: "
                           "Found METHOD_CIMINSTANCEPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleEnumerateInstances: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on "
                       "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstances: "
                       "enter(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstances: "
                       "exit(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstances: "
                   "Caught CIMExcetion(METHOD_CIMINSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc = env->NewObject(
                              jv->CIMClassClassRef,
                              jv->CIMClassNewJ,
                              jccRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr  = (jobjectArray)env->CallObjectMethod(
                                    (jobject)pr.jProvider,
                                    id,
                                    jcop,
                                    JMPI_LOCALONLY,
                                    JMPI_INCLUDE_QUALIFIERS,
                                    request->includeClassOrigin,
                                    jPropertyList,
                                    jcc);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    /* Fix for 4237 */
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleEnumerateInstances: "
                               "enter: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleEnumerateInstances: "
                               "exit: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                           "handleEnumerateInstances: "
                           "Caught CIMExcetion "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                                getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstances: "
                       "enter(METHOD_CIMINSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstances: "
                       "exit(METHOD_CIMINSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));
            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstances: "
                   "Caught CIMExcetion(METHOD_CIMINSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc = env->NewObject(
                              jv->CIMClassClassRef,
                              jv->CIMClassNewJ,
                              jccRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod(
                                   (jobject)pr.jProvider,
                                   id,
                                   joc,
                                   jcop,
                                   jcc,
                                   JMPI_INCLUDE_QUALIFIERS,
                                   request->includeClassOrigin,
                                   jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    /* Fix for 4237 */
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleEnumerateInstancesRequest: "
                               "enter: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                                getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleEnumerateInstancesRequest: "
                               "exit: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                                getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                           "handleEnumerateInstancesRequest: "
                           "Caught CIMExcetion "
                              "cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        /* Fix for 4189 */
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
                PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                    "handleEnumerateInstancesRequest: "
                        "enter(METHOD_INSTANCEPROVIDER2): "
                        "cimom_handle->getClass(%s).",
                    (const char*)request->className.getString().getCString()
                    ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass (context,
                                                 request->nameSpace,
                                                 request->className,
                                                 false,
                                                 true,
                                                 true,
                                                 CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstancesRequest: "
                       "enter(METHOD_INSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstancesRequest: "
                   "Caught CIMExcetion(METHOD_INSTANCEPROVIDER2) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc = env->NewObject(
                              jv->CIMClassClassRef,
                              jv->CIMClassNewJ,
                              jccRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 joc,
                                                 jcop,
                                                 jcc,
                                                 JMPI_INCLUDE_QUALIFIERS,
                                                 request->includeClassOrigin,
                                                 jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                      i<m;
                      i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    /* Fix for 4237 */
                    CIMClass             cls;

                    try
                    {
                        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                            "handleEnumerateInstancesRequest: "
                                "enter: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                            ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleEnumerateInstancesRequest: "
                               "exit: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                           "handleEnumerateInstancesRequest: "
                           "Caught CIMExcetion "
                              "cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                              getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    _fixCIMObjectPath(ciRet, cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }
        /* Fix for 4189 */

        case METHOD_INSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstancesRequest: "
                       "enter(METHOD_INSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstancesRequest: "
                       "exit(METHOD_INSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));
            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstancesRequest: "
                   "Caught CIMExcetion(METHOD_INSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong   jccRef = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);
            jobject jcc = env->NewObject(
                              jv->CIMClassClassRef,
                              jv->CIMClassNewJ,
                              jccRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            // Modified for Bugzilla# 3679
            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 jcop,
                                                 request->deepInheritance,
                                                 jcc,
                                                 JMPI_LOCALONLY);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                      i<m;
                      i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    /* Fix for 4237 */
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleEnumerateInstancesRequest: "
                               "enter: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());

                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleEnumerateInstancesRequest: "
                               "exit: cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                           "handleEnumerateInstancesRequest: "
                           "Caught CIMExcetion "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }
                    _fixCIMObjectPath(ciRet, cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleEnumerateInstancesRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnumerateInstanceNamesRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleEnumerateInstanceNamesRequest");

    HandlerIntro(EnumerateInstanceNames,message,request,response, handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_CIMINSTANCEPROVIDER2,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
             "handleEnumerateInstanceNamesRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->className.getString().getCString()));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->className);

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                name.getPhysicalName(),
                                                name.getLogicalName());

        // convert arguments
        OperationContext context;

        context.insert(
            request->operationContext.get(IdentityContainer::NAME));
        context.insert(
            request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(
            request->operationContext.get(ContentLanguageListContainer::NAME));

        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleEnumerateInstanceNamesRequest: "
            "Calling provider: %s",(const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleEnumerateInstanceNamesRequest: "
                     "Could not initialize the JVM (Java Virtual Machine) "
                     "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                         "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "enumInstances",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;"
                        "ZLorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "Found METHOD_INSTANCEPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                       (jclass)pr.jProviderClass,
                       "enumerateInstanceNames",
                       "(Lorg/pegasus/jmpi/CIMObjectPath;"
                           "Lorg/pegasus/jmpi/CIMClass;)["
                               "Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleEnumerateInstanceNamesRequest: "
                           "Found METHOD_CIMINSTANCEPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "enumerateInstanceNames",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "Found METHOD_INSTANCEPROVIDER2.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "enumerateInstanceNames",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                             "Lorg/pegasus/jmpi/CIMObjectPath;"
                                 "Lorg/pegasus/jmpi/CIMClass;)"
                                     "[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleEnumerateInstanceNamesRequest: "
                           "Found METHOD_CIMINSTANCEPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleEnumerateInstanceNamesRequest: "
                    "No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on "
                       "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "enter(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "exit(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstanceNamesRequest: "
                   "Caught CIMExcetion(METHOD_CIMINSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcimClassRef = DEBUG_ConvertCToJava(
                                     CIMClass*,
                                     jlong,
                                     pcls);
            jobject jcimClass = env->NewObject(
                                    jv->CIMClassClassRef,
                                    jv->CIMClassNewJ,
                                    jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod(
                                   (jobject)pr.jProvider,
                                   id,
                                   jcop,
                                   jcimClass);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "enter(METHOD_CIMINSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "exit(METHOD_CIMINSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstanceNamesRequest: "
                   "Caught CIMExcetion(METHOD_CIMINSTANCEPROVIDER2) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcimClassRef = DEBUG_ConvertCToJava(CIMClass*, jlong, pcls);
            jobject jcimClass = env->NewObject(
                                    jv->CIMClassClassRef,
                                    jv->CIMClassNewJ,
                                    jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod(
                                   (jobject)pr.jProvider,
                                   id,
                                   joc,
                                   jcop,
                                   jcimClass);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "enter(METHOD_INSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "exit(METHOD_INSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstanceNamesRequest: "
                   "Caught CIMExcetion(METHOD_INSTANCEPROVIDER2) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcimClassRef = DEBUG_ConvertCToJava(
                                     CIMClass*,
                                     jlong,
                                     pcls);
            jobject jcimClass = env->NewObject(
                                    jv->CIMClassClassRef,
                                    jv->CIMClassNewJ,
                                    jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 joc,
                                                 jcop,
                                                 jcimClass);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                      i<m;
                      i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(
                                          jVec,
                                          JMPIjvm::jv.VectorElementAt,
                                          i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "exit(METHOD_INSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleEnumerateInstanceNamesRequest: "
                       "exit(METHOD_INSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));
            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                   "handleEnumerateInstanceNamesRequest: "
                   "Caught CIMExcetion(METHOD_INSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcimClassRef = DEBUG_ConvertCToJava(
                                     CIMClass*,
                                     jlong,
                                     pcls);
            jobject jcimClass = env->NewObject(
                                    jv->CIMClassClassRef,
                                    jv->CIMClassNewJ,
                                    jcimClassRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 jcop,
                                                 true,
                                                 jcimClass);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                      i<m;
                      i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(
                                          jVec,
                                          JMPIjvm::jv.VectorElementAt,
                                          i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleEnumerateInstanceNamesRequest: "
                    "Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleCreateInstanceRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleCreateInstanceRequest");

    HandlerIntro(CreateInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_INSTANCEPROVIDER, // same as METHOD_CIMINSTANCEPROVIDER
       METHOD_INSTANCEPROVIDER2 // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleCreateInstanceRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 newInstance.getPath().getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath =
            new CIMObjectPath(
                System::getHostName(),
                request->nameSpace,
                request->newInstance.getPath().getClassName(),
                request->newInstance.getPath().getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleCreateInstanceRequest: "
            "Calling provider: %s", (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleCreateInstanceRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(
                ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "createInstance",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;"
                        "Lorg/pegasus/jmpi/CIMInstance;)"
                            "Lorg/pegasus/jmpi/CIMObjectPath;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleCreateInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER.");
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "createInstance",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMInstance;)"
                                "Lorg/pegasus/jmpi/CIMObjectPath;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleCreateInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER2.");
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleCreateInstanceRequest: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on "
                        "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_INSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci     = new CIMInstance (request->newInstance);
            jlong jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject jci = env->NewObject(
                              jv->CIMInstanceClassRef,
                              jv->CIMInstanceNewJ,
                              jciRef);

            JMPIjvm::checkException(env);

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                 "handleCreateInstanceRequest: "
                     "id = %p, jcop = %p, jci = %p",
                       id,jcop,jci));

            StatProviderTimeMeasurement providerTime(response);

            jobject jcopRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                    id,
                                                    jcop,
                                                    jci);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jcopRet) {
                jlong jCopRetRef = env->CallLongMethod(
                                       jcopRet,
                                       JMPIjvm::jv.CIMObjectPathCInst);
                CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                            jlong,
                                            CIMObjectPath*,
                                            jCopRetRef);

                handler.deliver(*copRet);
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci = new CIMInstance (request->newInstance);
            jlong jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject jci = env->NewObject(
                              jv->CIMInstanceClassRef,
                              jv->CIMInstanceNewJ,
                              jciRef);

            JMPIjvm::checkException(env);

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                 "handleCreateInstanceRequest: "
                     "id = %p, jcop = %p, jci = %p",
                       id,jcop,jci));

            StatProviderTimeMeasurement providerTime(response);

            jobject jcopRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                    id,
                                                    joc,
                                                    jcop,
                                                    jci);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            if (jcopRet) {
                jlong jCopRetRef = env->CallLongMethod(
                                       jcopRet,
                                       JMPIjvm::jv.CIMObjectPathCInst);
                CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                            jlong,
                                            CIMObjectPath*,
                                            jCopRetRef);

                handler.deliver(*copRet);
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleCreateInstanceRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleModifyInstanceRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleModifyInstanceRequest");

    HandlerIntro(ModifyInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2, // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleModifyInstanceRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->modifiedInstance.getPath().getClassName()
                 .getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath =
           new CIMObjectPath(
               System::getHostName(),
               request->nameSpace,
               request->modifiedInstance.getPath().getClassName(),
               request->modifiedInstance.getPath ().getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
             "handleModifyInstanceRequest: "
                 "provider name physical = %s, logical = %s",
              (const char*)name.getPhysicalName().getCString(),
              (const char*)name.getLogicalName().getCString()
              ));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleModifyInstanceRequest: "
            "Calling provider: %s", (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL1,
                "handleModifyInstanceRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(
                ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "setInstance",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;"
                        "Lorg/pegasus/jmpi/CIMInstance;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleModifyInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "setInstance",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Z[Ljava/lang/String)V");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleModifyInstanceRequest: "
                           "Found METHOD_CIMINSTANCEPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                   (jclass)pr.jProviderClass,
                   "setInstance",
                   "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMInstance;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleModifyInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER2.");
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleModifyInstanceRequest: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on "
                       "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);

            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci = new CIMInstance (request->modifiedInstance);
            jlong jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject jci = env->NewObject(
                              jv->CIMInstanceClassRef,
                              jv->CIMInstanceNewJ,
                              jciRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                joc,
                                jcop,
                                jci);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci     = new CIMInstance (request->modifiedInstance);
            jlong jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject jci = env->NewObject(
                              jv->CIMInstanceClassRef,
                              jv->CIMInstanceNewJ,
                              jciRef);

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                jcop,
                                jci,
                                JMPI_INCLUDE_QUALIFIERS,
                                jPropertyList);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            CIMInstance *ci = new CIMInstance (request->modifiedInstance);
            jlong jciRef = DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
            jobject jci = env->NewObject(
                              jv->CIMInstanceClassRef,
                              jv->CIMInstanceNewJ,
                              jciRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                jcop,
                                jci);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleModifyInstanceRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDeleteInstanceRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleDeleteInstanceRequest");

    HandlerIntro(DeleteInstance,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_INSTANCEPROVIDER,  // same as METHOD_CIMINSTANCEPROVIDER
       METHOD_INSTANCEPROVIDER2, // same as METHOD_CIMINSTANCEPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleDeleteInstanceRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 instanceName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->instanceName.getClassName(),
                                        request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleDeleteInstanceRequest: "
            "Calling provider: %s", (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleDeleteInstanceRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID((jclass)pr.jProviderClass,
                                 "deleteInstance",
                                 "(Lorg/pegasus/jmpi/CIMObjectPath;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleDeleteInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER.");
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "deleteInstance",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleDeleteInstanceRequest: "
                       "Found METHOD_INSTANCEPROVIDER2.");
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleDeleteInstanceRequest: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on "
                       "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_INSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                joc,
                                jcop);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod((jobject)pr.jProvider,
                                id,
                                jcop);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleDeleteInstanceRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleExecQueryRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleExecQueryRequest");

    HandlerIntro(ExecQuery,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMINSTANCEPROVIDER,
       METHOD_CIMINSTANCEPROVIDER2,
       METHOD_INSTANCEPROVIDER,
       METHOD_INSTANCEPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleExecQueryRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->className.getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath (System::getHostName(),
                                                       request->nameSpace,
                                                       request->className);

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(
            request->operationContext.get(IdentityContainer::NAME));
        context.insert(
            request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(
            request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleExecQueryRequest: "
                "Calling provider: %s, queryLanguage: %s, query: %s",
            (const char*)pr.getName().getCString(),
            (const char*)request->queryLanguage.getCString(),
            (const char*)request->query.getCString()
            ));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleExecQueryRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(
                ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                   (jclass)pr.jProviderClass,
                   "execQuery",
                   "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                       "ILorg/pegasus/jmpi/CIMClass;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: Found METHOD_INSTANCEPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "execQuery",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                            "Ljava/lang/String;Lorg/pegasus/jmpi/CIMClass;)"
                                "[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleExecQueryRequest: "
                           "Found METHOD_CIMINSTANCEPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "execQuery",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                         "Lorg/pegasus/jmpi/CIMObjectPath;"
                             "Lorg/pegasus/jmpi/CIMClass;Ljava/lang/String;"
                                 "Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_INSTANCEPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: Found METHOD_INSTANCEPROVIDER2.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "execQuery",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;"
                                "Lorg/pegasus/jmpi/CIMClass;Ljava/lang/String;"
                                    "Ljava/lang/String;)"
                                        "[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMINSTANCEPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleExecQueryRequest: "
                           "Found METHOD_CIMINSTANCEPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleExecQueryRequest: No method provider found!");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                    "Could not find a method for the provider based on "
                         "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMINSTANCEPROVIDER:
        {
            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(
                                         request->queryLanguage.getCString());
            jstring jquery         = env->NewStringUTF(
                                         request->query.getCString());

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "enter(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "exit(METHOD_CIMINSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                   "handleExecQueryRequest: "
                   "Caught CIMExcetion(METHOD_CIMINSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(
                            jv->CIMClassClassRef,
                            jv->CIMClassNewJ,
                            jcls);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod(
                                   (jobject)pr.jProvider,
                                   id,
                                   jcop,
                                   jquery,
                                   jqueryLanguage,
                                   jCc);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMINSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(
                                         request->queryLanguage.getCString());
            jstring jquery         = env->NewStringUTF(
                                         request->query.getCString());

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "enter(METHOD_CIMINSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "exit(METHOD_CIMINSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                   "handleExecQueryRequest: "
                   "Caught CIMExcetion(METHOD_CIMINSTANCEPROVIDER2) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(
                            jv->CIMClassClassRef,
                            jv->CIMClassNewJ,
                            jcls);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr = (jobjectArray)env->CallObjectMethod(
                                   (jobject)pr.jProvider,
                                   id,
                                   joc,
                                   jcop,
                                   jquery,
                                   jqueryLanguage,
                                   jCc);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(
                                         request->queryLanguage.getCString());
            jstring jquery         = env->NewStringUTF(
                                         request->query.getCString());

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "enter(METHOD_INSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "exit(METHOD_INSTANCEPROVIDER2): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                   "handleExecQueryRequest: "
                   "Caught CIMExcetion(METHOD_INSTANCEPROVIDER2) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(
                            jv->CIMClassClassRef,
                            jv->CIMClassNewJ,
                            jcls);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod ((jobject)pr.jProvider,
                                                  id,
                                                  joc,
                                                  jcop,
                                                  jCc,
                                                  jquery,
                                                  jqueryLanguage);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec)
            {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i < m;
                     i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);
                    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                        "handleExecQueryRequest: jciRet = %p",jciRet));

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "handleExecQueryRequest: done!");

            handler.complete();
            break;
        }

        case METHOD_INSTANCEPROVIDER:
        {
            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring jqueryLanguage = env->NewStringUTF(
                                         request->queryLanguage.getCString());
            jstring jquery = env->NewStringUTF(
                                 request->query.getCString());

            CIMClass cls;

            try
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "enter(METHOD_INSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

               AutoMutex lock (pr._cimomMutex);

               cls = pr._cimom_handle->getClass(context,
                                                request->nameSpace,
                                                request->className,
                                                false,
                                                true,
                                                true,
                                                CIMPropertyList());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleExecQueryRequest: "
                       "exit(METHOD_INSTANCEPROVIDER): "
                       "cimom_handle->getClass(%s).",
                   (const char*)request->className.getString().getCString()
                   ));

            }
            catch (CIMException e)
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                   "handleExecQueryRequest: "
                   "Caught CIMExcetion(METHOD_INSTANCEPROVIDER) "
                       "during cimom_handle->getClass(%s): %s ",
                   (const char*)request->className.getString().getCString(),
                   (const char*)e.getMessage().getCString()
                   ));

               PEG_METHOD_EXIT();
               throw;
            }

            CIMClass *pcls = new CIMClass (cls);

            JMPIjvm::checkException(env);

            jlong jcls = DEBUG_ConvertCToJava (CIMClass*, jlong, pcls);

            jobject jCc=env->NewObject(
                            jv->CIMClassClassRef,
                            jv->CIMClassNewJ,
                            jcls);

            JMPIjvm::checkException(env);

            jlong jql = 0; // @BUG - how to convert?

            StatProviderTimeMeasurement providerTime(response);

            jobject jVec = env->CallObjectMethod((jobject)pr.jProvider,
                                                 id,
                                                 jcop,
                                                 jquery,
                                                 jql,
                                                 jCc);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m;
                     i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleExecQueryRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleAssociatorsRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleAssociatorsRequest");

    HandlerIntro(Associators,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleAssociatorsRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 objectName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->objectName.getClassName(),
                                        request->objectName.getKeyBindings());
        CIMObjectPath *assocPath  = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->assocClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(
            request->operationContext.get(IdentityContainer::NAME));
        context.insert(
            request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(
            request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleAssociatorsRequest: "
                "Calling provider: %s, role: %s, aCls: %s",
            (const char*)pr.getName().getCString(),
            (const char*)request->role.getCString(),
            (const char*)request->assocClass.getString().getCString()
            ));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleAssociatorsRequest: "
                "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(
                ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                   (jclass)pr.jProviderClass,
                   "associators",
                   "(Lorg/pegasus/jmpi/CIMObjectPath;"
                       "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                           "Ljava/lang/String;Ljava/lang/String;"
                               "ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleAssociatorsRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "associators",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                                "Ljava/lang/String;Ljava/lang/String;"
                                    "ZZ[Ljava/lang/String;)"
                                        "[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleAssociatorsRequest: "
                           "Found METHOD_ASSOCIATORPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "associators",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;"
                                "Ljava/lang/String;Ljava/lang/String;"
                                    "Ljava/lang/String;ZZ[Ljava/lang/String;)"
                                        "Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleAssociatorsRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER2.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();
               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "associators",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;"
                                "Lorg/pegasus/jmpi/CIMObjectPath;"
                                    "Ljava/lang/String;Ljava/lang/String;"
                                       "Ljava/lang/String;ZZ[Ljava/lang/String;"
                                          ")[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleAssociatorsRequest: "
                           "Found METHOD_CIMASSOCIATORPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleAssociatorsRequest: No method found!");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                    "Could not find a method for the provider based on "
                        "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong   jAssociationNameRef = DEBUG_ConvertCToJava(
                                              CIMObjectPath*,
                                              jlong,
                                              assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava(
                                       CIMObjectPath*,
                                       jlong,
                                       objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole =
                env->NewStringUTF(
                    request->role.getCString());
            jstring jResultRole =
                env->NewStringUTF(
                    request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeClassOrigin = false");
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 jAssociationName,
                                 jPathName,
                                 jResultClass,
                                 jRole,
                                 jResultRole,
                                 JMPI_INCLUDE_QUALIFIERS,
                                 request->includeClassOrigin,
                                 jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleAssociatorsRequest: "
                               "enter(METHOD_CIMASSOCIATORPROVIDER): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());

                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleAssociatorsRequest: "
                               "exit(METHOD_CIMASSOCIATORPROVIDER): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                                  getClassName().getString().getCString()
                           ));

                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleAssociatorsRequest: "
                           "Caught CIMExcetion(METHOD_CIMASSOCIATORPROVIDER) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                                  getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    _fixCIMObjectPath(ciRet, cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole =
                env->NewStringUTF(request->role.getCString());
            jstring jResultRole =
                env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeClassOrigin = false");
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jAssociationName,
                                 jPathName,
                                 jResultClass,
                                 jRole,
                                 jResultRole,
                                 JMPI_INCLUDE_QUALIFIERS,
                                 request->includeClassOrigin,
                                 jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass cls;

                    try
                    {
                        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                            "handleAssociatorsRequest: "
                                "enter(METHOD_CIMASSOCIATORPROVIDER2): "
                                "cimom_handle->getClass(%s).",
                            (const char*)ciRet->
                                getClassName().getString().getCString()
                            ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleAssociatorsRequest: "
                               "exit(METHOD_CIMASSOCIATORPROVIDER2): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleAssociatorsRequest: "
                           "Caught CIMExcetion(METHOD_CIMASSOCIATORPROVIDER2) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    _fixCIMObjectPath(ciRet, cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole =
                env->NewStringUTF(request->role.getCString());
            jstring jResultRole =
                env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeClassOrigin = false");
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  joc,
                                  jAssociationName,
                                  jPathName,
                                  jResultClass,
                                  jRole,
                                  jResultRole,
                                  JMPI_INCLUDE_QUALIFIERS,
                                  request->includeClassOrigin,
                                  jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m;
                     i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleAssociatorsRequest: "
                               "enter(METHOD_ASSOCIATORPROVIDER2): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(
                                 context,
                                 request->nameSpace,
                                 ciRet->getClassName(),
                                 false,
                                 true,
                                 true,
                                 CIMPropertyList());

                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleAssociatorsRequest: "
                               "exit(METHOD_ASSOCIATORPROVIDER2): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleAssociatorsRequest: "
                           "Caught CIMExcetion(METHOD_ASSOCIATORPROVIDER2) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    _fixCIMObjectPath(ciRet, cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole =
                env->NewStringUTF(request->role.getCString());
            jstring jResultRole =
                env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorsRequest: includeClassOrigin = false");
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  jAssociationName,
                                  jPathName,
                                  jResultClass,
                                  jRole,
                                  jResultRole,
                                  JMPI_INCLUDE_QUALIFIERS,
                                  request->includeClassOrigin,
                                  jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m;
                     i++)
                {
                    JMPIjvm::checkException(env);
                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleAssociatorsRequest: "
                               "enter(METHOD_ASSOCIATORPROVIDER): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());

                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleAssociatorsRequest: "
                               "exit(METHOD_ASSOCIATORPROVIDER): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleAssociatorsRequest: "
                           "Caught CIMExcetion(METHOD_ASSOCIATORPROVIDER) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    _fixCIMObjectPath(ciRet, cls);

                    JMPIjvm::checkException(env);

                    handler.deliver(*ciRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleAssociatorsRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleAssociatorNamesRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleAssociatorNamesRequest");

    HandlerIntro(AssociatorNames,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleAssociatorNamesRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 objectName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->objectName.getClassName(),
                                        request->objectName.getKeyBindings());
        CIMObjectPath *assocPath  = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->assocClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleAssociatorNamesRequest: "
                "Calling provider: %s, role: %s, aCls: %s",
            (const char*)pr.getName().getCString(),
            (const char*)request->role.getCString(),
            (const char*)request->assocClass.getString().getCString()
            ));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleAssociatorNamesRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "associatorNames",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                            "Ljava/lang/String;Ljava/lang/String;)"
                                 "Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleAssociatorNamesRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();
               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "associatorNames",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                                "Ljava/lang/String;Ljava/lang/String;)"
                                    "[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleAssociatorNamesRequest: "
                           "Found METHOD_CIMASSOCIATORPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "associatorNames",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                                "Ljava/lang/String;Ljava/lang/String;)"
                                    "Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleAssociatorNamesRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER2.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "associatorNames",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;"
                                "Lorg/pegasus/jmpi/CIMObjectPath;"
                                    "Ljava/lang/String;Ljava/lang/String;"
                                        "Ljava/lang/String;)["
                                            "Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleAssociatorNamesRequest: "
                           "Found METHOD_CIMASSOCIATORPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleAssociatorNames: No method found!");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                    "Could not find a method for the provider based on "
                        "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole        =
                env->NewStringUTF(request->role.getCString());
            jstring jResultRole  =
                env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 jAssociationName,
                                 jPathName,
                                 jResultClass,
                                 jRole,
                                 jResultRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole =
                env->NewStringUTF(request->role.getCString());
            jstring jResultRole =
                env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jAssociationName,
                                 jPathName,
                                 jResultClass,
                                 jRole,
                                 jResultRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole =
                env->NewStringUTF(request->role.getCString());
            jstring jResultRole =
                env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  jAssociationName,
                                  jPathName,
                                  jResultClass,
                                  jRole,
                                  jResultRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m; i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(
                                          jVec,
                                          JMPIjvm::jv.VectorElementAt,
                                          i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            assocPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jResultClass =
                env->NewStringUTF(
                    request->resultClass.getString().getCString());
            jstring jRole =
                env->NewStringUTF(request->role.getCString());
            jstring jResultRole =
                env->NewStringUTF(request->resultRole.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: assocName = %s ",
                (const char*)assocPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultClass = %s ",
                (const char*)request->resultClass.getString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleAssociatorNamesRequest: resultRole = %s ",
                (const char*)request->resultRole.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  joc,
                                  jAssociationName,
                                  jPathName,
                                  jResultClass,
                                  jRole,
                                  jResultRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m; i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(
                                          jVec,
                                          JMPIjvm::jv.VectorElementAt,
                                          i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    JMPIjvm::checkException(env);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleAssociatorNamesRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleReferencesRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleReferencesRequest");

    HandlerIntro(References,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleReferencesRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 objectName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->objectName.getClassName(),
                                        request->objectName.getKeyBindings());
        CIMObjectPath *resultPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->resultClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                name.getPhysicalName(),
                name.getLogicalName(),
                String::EMPTY);

        // convert arguments
        OperationContext context;

        context.insert(
            request->operationContext.get(IdentityContainer::NAME));
        context.insert(
            request->operationContext.get(AcceptLanguageListContainer::NAME));
        context.insert(
            request->operationContext.get(ContentLanguageListContainer::NAME));

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleReferencesRequest: "
                "Calling provider: %s, role: %s, refCls: %s",
            (const char*)pr.getName().getCString(),
            (const char*)request->role.getCString(),
            (const char*)request->resultClass.getString().getCString()
            ));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleReferencesRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "references",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;"
                         "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                             "ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleReferencesRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();
               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "references",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                                "ZZ[Ljava/lang/String;)"
                                    "[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleReferencesRequest: "
                           "Found METHOD_CIMASSOCIATORPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "references",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                                "ZZ[Ljava/lang/String;)Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleReferencesRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER2.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();
               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "references",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                             "Lorg/pegasus/jmpi/CIMObjectPath;"
                                 "Lorg/pegasus/jmpi/CIMObjectPath;"
                                     "Ljava/lang/String;ZZ[Ljava/lang/String;)"
                                         "[Lorg/pegasus/jmpi/CIMInstance;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleReferencesRequest: "
                           "Found METHOD_CIMASSOCIATORPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleReferencesRequest: No method provider found!");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                    "Could not find a method for the provider based on "
                        "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong   jPathNameRef = DEBUG_ConvertCToJava(
                                       CIMObjectPath*,
                                       jlong,
                                       objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: assocName = %s ",
                (const char*)resultPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeClassOrigin = false");
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 jAssociationName,
                                 jPathName,
                                 jRole,
                                 JMPI_INCLUDE_QUALIFIERS,
                                 request->includeClassOrigin,
                                 jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleReferencesRequest: "
                               "enter(METHOD_CIMASSOCIATORPROVIDER): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());

                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleReferencesRequest: "
                               "exit(METHOD_CIMASSOCIATORPROVIDER): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleReferencesRequest: "
                           "Caught CIMExcetion(METHOD_CIMASSOCIATORPROVIDER) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    _fixCIMObjectPath(ciRet, cls);

                    JMPIjvm::checkException(env);

                    CIMObject coRet(*ciRet);
                    handler.deliver(coRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: assocName = %s ",
                (const char*)resultPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeClassOrigin = false");
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jAssociationName,
                                 jPathName,
                                 jRole,
                                 JMPI_INCLUDE_QUALIFIERS,
                                 request->includeClassOrigin,
                                 jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);

                    JMPIjvm::checkException(env);

                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass             cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleReferencesRequest: "
                               "enter(METHOD_CIMASSOCIATORPROVIDER2): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());

                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleReferencesRequest: "
                               "exit(METHOD_CIMASSOCIATORPROVIDER2): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleReferencesRequest: "
                           "Caught CIMExcetion(METHOD_CIMASSOCIATORPROVIDER2) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    const CIMObjectPath& op    = ciRet->getPath();
                    CIMObjectPath        iop   = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    CIMObject coRet(*ciRet);
                    handler.deliver(coRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: assocName = %s ",
                (const char*)resultPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeClassOrigin = false");
#endif
            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  jAssociationName,
                                  jPathName,
                                  jRole,
                                  JMPI_INCLUDE_QUALIFIERS,
                                  request->includeClassOrigin,
                                  jPropertyList);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m; i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass cls;

                    try
                    {
                      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                          "handleReferencesRequest: "
                              "enter(METHOD_ASSOCIATORPROVIDER): "
                              "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                          ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleReferencesRequest: "
                               "exit(METHOD_ASSOCIATORPROVIDER): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));
                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleReferencesRequest: "
                           "Caught CIMExcetion(METHOD_ASSOCIATORPROVIDER) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    const CIMObjectPath& op        = ciRet->getPath();
                    CIMObjectPath        iop       = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    CIMObject coRet(*ciRet);
                    handler.deliver(coRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

            jobjectArray jPropertyList = getList(jv,env,request->propertyList);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: assocName = %s ",
                (const char*)resultPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: pathName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: role = %s ",
                (const char*)request->role.getCString()));
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeQualifiers = false");
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferencesRequest: includeClassOrigin = false");
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  joc,
                                  jAssociationName,
                                  jPathName,
                                  jRole,
                                  JMPI_INCLUDE_QUALIFIERS,
                                  request->includeClassOrigin,
                                  jPropertyList);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m; i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jciRet = env->CallObjectMethod(
                                         jVec,
                                         JMPIjvm::jv.VectorElementAt,
                                         i);

                    JMPIjvm::checkException(env);

                    jlong jciRetRef = env->CallLongMethod(
                                          jciRet,
                                          JMPIjvm::jv.CIMInstanceCInst);
                    CIMInstance *ciRet = DEBUG_ConvertJavaToC(
                                             jlong,
                                             CIMInstance*,
                                             jciRetRef);
                    CIMClass cls;

                    try
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleReferencesRequest: "
                               "enter(METHOD_ASSOCIATORPROVIDER2): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                       AutoMutex lock (pr._cimomMutex);

                       cls = pr._cimom_handle->getClass(context,
                                                        request->nameSpace,
                                                        ciRet->getClassName(),
                                                        false,
                                                        true,
                                                        true,
                                                        CIMPropertyList());
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                           "handleReferencesRequest: "
                               "exit(METHOD_ASSOCIATORPROVIDER2): "
                               "cimom_handle->getClass(%s).",
                           (const char*)ciRet->
                               getClassName().getString().getCString()
                           ));

                    }
                    catch (CIMException e)
                    {
                       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                           "handleReferencesRequest: "
                           "Caught CIMExcetion(METHOD_ASSOCIATORPROVIDER2) "
                               "during cimom_handle->getClass(%s): %s ",
                           (const char*)ciRet->
                               getClassName().getString().getCString(),
                           (const char*)e.getMessage().getCString()
                           ));

                       PEG_METHOD_EXIT();
                       throw;
                    }

                    const CIMObjectPath& op        = ciRet->getPath();
                    CIMObjectPath        iop       = ciRet->buildPath(cls);

                    JMPIjvm::checkException(env);

                    iop.setNameSpace(op.getNameSpace());
                    ciRet->setPath(iop);

                    CIMObject coRet(*ciRet);
                    handler.deliver(coRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleReferencesRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleReferenceNamesRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleReferenceNamesRequest");

    HandlerIntro(ReferenceNames,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMASSOCIATORPROVIDER,
       METHOD_CIMASSOCIATORPROVIDER2,
       METHOD_ASSOCIATORPROVIDER,
       METHOD_ASSOCIATORPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleReferenceNamesRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 objectName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->objectName.getClassName(),
                                        request->objectName.getKeyBindings());
        CIMObjectPath *resultPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->resultClass.getString());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                name.getPhysicalName(),
                                                name.getLogicalName(),
                                                String::EMPTY);

        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleReferenceNamesRequest: "
                "Calling provider: %s, role: %s, refCls: %s",
            (const char*)pr.getName().getCString(),
            (const char*)request->role.getCString(),
            (const char*)request->resultClass.getString().getCString()
            ));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleReferenceNamesRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                   (jclass)pr.jProviderClass,
                   "referenceNames",
                   "(Lorg/pegasus/jmpi/CIMObjectPath;"
                       "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;)"
                           "Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleReferencesRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "referenceNames",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                                ")[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleReferenceNamesRequest: "
                           "Found METHOD_CIMASSOCIATORPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "referenceNames",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                                ")Ljava/util/Vector;");

           if (id != NULL)
           {
               eMethodFound = METHOD_ASSOCIATORPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleReferenceNamesRequest: "
                       "Found METHOD_ASSOCIATORPROVIDER2.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();
               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "referenceNames",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;"
                                "Lorg/pegasus/jmpi/CIMObjectPath;"
                                    "Ljava/lang/String;)"
                                        "[Lorg/pegasus/jmpi/CIMObjectPath;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMASSOCIATORPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleReferenceNamesRequest: "
                           "Found METHOD_CIMASSOCIATORPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleReferenceNames: No method found!");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                    "Could not find a method for the provider based on "
                        "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMASSOCIATORPROVIDER:
        {
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: assocName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 jPathName,
                                 jAssociationName,
                                 jRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);

                    JMPIjvm::checkException(env);

                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_CIMASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: assocName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jAr=(jobjectArray)env->CallObjectMethod(
                                 (jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jPathName,
                                 jAssociationName,
                                 jRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jAr) {
                for (int i=0,m=env->GetArrayLength(jAr); i<m; i++) {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->GetObjectArrayElement(jAr,i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);

                    JMPIjvm::checkException(env);

                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER:
        {
            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: assocName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: pathName = %s ",
                (const char*)resultPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  jAssociationName,
                                  jPathName,
                                  jRole);

            JMPIjvm::checkException(env);

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m; i++)
                {
                    JMPIjvm::checkException(env);

                    jobject jcopRet = env->CallObjectMethod(
                                          jVec,
                                          JMPIjvm::jv.VectorElementAt,
                                          i);

                    JMPIjvm::checkException(env);

                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);

                    JMPIjvm::checkException(env);

                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);

                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_ASSOCIATORPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jAssociationNameRef = DEBUG_ConvertCToJava(
                                            CIMObjectPath*,
                                            jlong,
                                            resultPath);
            jobject jAssociationName = env->NewObject(
                                           jv->CIMObjectPathClassRef,
                                           jv->CIMObjectPathNewJ,
                                           jAssociationNameRef);

            JMPIjvm::checkException(env);

            jlong jPathNameRef = DEBUG_ConvertCToJava(
                                     CIMObjectPath*,
                                     jlong,
                                     objectPath);
            jobject jPathName = env->NewObject(
                                    jv->CIMObjectPathClassRef,
                                    jv->CIMObjectPathNewJ,
                                    jPathNameRef);

            JMPIjvm::checkException(env);

            jstring jRole = env->NewStringUTF(request->role.getCString());

            JMPIjvm::checkException(env);

#ifdef PEGASUS_DEBUG
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: assocName = %s ",
                (const char*)objectPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: pathName = %s ",
                (const char*)resultPath->toString().getCString()));
            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleReferenceNamesRequest: role = %s ",
                (const char*)request->role.getCString()));
#endif

            StatProviderTimeMeasurement providerTime(response);

            jobjectArray jVec=(jobjectArray)env->CallObjectMethod(
                                  (jobject)pr.jProvider,
                                  id,
                                  joc,
                                  jAssociationName,
                                  jPathName,
                                  jRole);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();
            if (jVec) {
                for (int i=0,m=env->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);
                     i<m; i++)
                {
                    JMPIjvm::checkException(env);
                    jobject jcopRet = env->CallObjectMethod(
                                          jVec,
                                          JMPIjvm::jv.VectorElementAt,
                                          i);
                    JMPIjvm::checkException(env);
                    jlong jcopRetRef = env->CallLongMethod(
                                           jcopRet,
                                           JMPIjvm::jv.CIMObjectPathCInst);
                    JMPIjvm::checkException(env);
                    CIMObjectPath *copRet = DEBUG_ConvertJavaToC(
                                                jlong,
                                                CIMObjectPath*,
                                                jcopRetRef);
                    handler.deliver(*copRet);
                }
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleReferenceNamesRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleGetPropertyRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleGetPropertyRequest");

    HandlerIntro(GetProperty,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_PROPERTYPROVIDER,
       METHOD_PROPERTYPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleGetPropertyRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 instanceName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->instanceName.getClassName(),
                                        request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                name.getPhysicalName(),
                                                name.getLogicalName(),
                                                String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleReferenceNamesRequest: "
                "Calling provider: %s",
            (const char*)pr.getName().getCString()
            ));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleReferenceNamesRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "getPropertyValue",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                        "Ljava/lang/String;)Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetPropertyRequest: "
                       "Found METHOD_PROPERTYPROVIDER.");
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "getPropertyValue",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                            "Ljava/lang/String;)Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleGetPropertyRequest: "
                       "Found METHOD_PROPERTYPROVIDER2.");
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleGetPropertyRequest: No method provider found!");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                    "Could not find a method for the provider based on "
                        "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_PROPERTYPROVIDER:
        {
            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring joclass =
              env->NewStringUTF(
                 request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName =
                env->NewStringUTF(
                    request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jvalRet = env->CallObjectMethod ((jobject)pr.jProvider,
                                                     id,
                                                     jcop,
                                                     joclass,
                                                     jpName);

            JMPIjvm::checkException(env);

            handler.processing();

            if (jvalRet)
            {
               jlong jvalRetRef = env->CallLongMethod(
                                      jvalRet,
                                      JMPIjvm::jv.CIMValueCInst);
               CIMValue *valRet = DEBUG_ConvertJavaToC(
                                      jlong,
                                      CIMValue*,
                                      jvalRetRef);

               JMPIjvm::checkException(env);

               handler.deliver(*valRet);
            }
            handler.complete();
            break;
        }

        case METHOD_PROPERTYPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring joclass = env->NewStringUTF(
                request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName = env->NewStringUTF(
                request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jvalRet = env->CallObjectMethod ((jobject)pr.jProvider,
                                                     id,
                                                     joc,
                                                     jcop,
                                                     joclass,
                                                     jpName);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            if (jvalRet)
            {
               jlong jvalRetRef = env->CallLongMethod(
                                      jvalRet,
                                      JMPIjvm::jv.CIMValueCInst);
               CIMValue *valRet = DEBUG_ConvertJavaToC(
                                      jlong,
                                      CIMValue*,
                                      jvalRetRef);

               JMPIjvm::checkException(env);

               handler.deliver(*valRet);
            }
            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleGetPropertyRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleSetPropertyRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleSetPropertyRequest");

    HandlerIntro(SetProperty,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_PROPERTYPROVIDER,
       METHOD_PROPERTYPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try
    {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleSetPropertyRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 instanceName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->instanceName.getClassName(),
                                        request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                name.getPhysicalName(),
                                                name.getLogicalName(),
                                                String::EMPTY);

        // forward request
        JMPIProvider &pr = ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleSetPropertyRequest: "
            "Calling provider. setPropertyValue: %s",
            (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "setPropertyValue",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                        "Ljava/lang/String;Lorg/pegasus/jmpi/CIMValue;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleSetPropertyRequest: "
                       "Found METHOD_PROPERTYPROVIDER.");
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "setPropertyValue",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                            "Ljava/lang/String;Lorg/pegasus/jmpi/CIMValue;)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_PROPERTYPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleSetPropertyRequest: "
                       "Found METHOD_PROPERTYPROVIDER2.");
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleSetPropertyRequest: No method provider found!");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                    "Could not find a method for the provider based on "
                        "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_PROPERTYPROVIDER:
        {
            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring joclass =
              env->NewStringUTF(
                 request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName =
                env->NewStringUTF(
                    request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            CIMValue *val = new CIMValue (request->newValue);

            JMPIjvm::checkException(env);

            jlong jvalref = DEBUG_ConvertCToJava(CIMValue*, jlong, val);
            jobject jval = env->NewObject(
                               jv->CIMValueClassRef,
                               jv->CIMValueNewJ,
                               jvalref);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 jcop,
                                 joclass,
                                 jpName,
                                 jval);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_PROPERTYPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopref = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopref);

            JMPIjvm::checkException(env);

            jstring joclass =
               env->NewStringUTF(
                 request->instanceName.getClassName().getString().getCString());

            JMPIjvm::checkException(env);

            jstring jpName =
                env->NewStringUTF(
                    request->propertyName.getString().getCString());

            JMPIjvm::checkException(env);

            CIMValue *val = new CIMValue (request->newValue);

            JMPIjvm::checkException(env);

            jlong jvalref = DEBUG_ConvertCToJava (CIMValue*, jlong, val);
            jobject jval = env->NewObject(
                               jv->CIMValueClassRef,
                               jv->CIMValueNewJ,
                               jvalref);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jcop,
                                 joclass,
                                 jpName,
                                 jval);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleSetPropertyRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleInvokeMethodRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleInvokeMethodRequest");

    HandlerIntro(InvokeMethod,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_CIMMETHODPROVIDER,
       METHOD_CIMMETHODPROVIDER2,
       METHOD_METHODPROVIDER,
       METHOD_METHODPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION   eMethodFound  = METHOD_UNKNOWN;
    JNIEnv          *env           = NULL;

    try {
        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleInvokeMethodRequest: "
                 "name space = %s class name = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)request->
                 instanceName.getClassName().getString().getCString()
             ));

        // make target object path
        CIMObjectPath *objectPath = new CIMObjectPath(
                                        System::getHostName(),
                                        request->nameSpace,
                                        request->instanceName.getClassName(),
                                        request->instanceName.getKeyBindings());

        // resolve provider name
        ProviderName name = _resolveProviderName(
            request->operationContext.get(ProviderIdContainer::NAME));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                name.getPhysicalName(),
                                                name.getLogicalName(),
                                                String::EMPTY);

        JMPIProvider &pr=ph.GetProvider();

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleInvokeMethodRequest: "
                "Calling provider: %s",
            (const char*)pr.getName().getCString()
            ));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleInvokeMethodRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "invokeMethod",
                    "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                        "Ljava/util/Vector;Ljava/util/Vector;)"
                            "Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_METHODPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleInvokeMethodRequest: "
                       "Found METHOD_PROPERTYPROVIDER.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();
               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "invokeMethod",
                        "(Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                            "[Lorg/pegasus/jmpi/CIMArgument;"
                                "[Lorg/pegasus/jmpi/CIMArgument;)"
                                    "Lorg/pegasus/jmpi/CIMValue;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMMETHODPROVIDER;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleInvokeMethodRequest: "
                           "Found METHOD_CIMMETHODPROVIDER.");
               }
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "invokeMethod",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;Ljava/lang/String;"
                            "Ljava/util/Vector;Ljava/util/Vector;)"
                                "Lorg/pegasus/jmpi/CIMValue;");

           if (id != NULL)
           {
               eMethodFound = METHOD_METHODPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleInvokeMethodRequest: "
                       "Found METHOD_METHODPROVIDER2.");
           }

           if (id == NULL)
           {
               env->ExceptionClear();

               id = env->GetMethodID(
                        (jclass)pr.jProviderClass,
                        "invokeMethod",
                        "(Lorg/pegasus/jmpi/OperationContext;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;"
                                "Ljava/lang/String;"
                                    "[Lorg/pegasus/jmpi/CIMArgument;["
                                        "Lorg/pegasus/jmpi/CIMArgument;)"
                                            "Lorg/pegasus/jmpi/CIMValue;");

               if (id != NULL)
               {
                   eMethodFound = METHOD_CIMMETHODPROVIDER2;
                   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                       "handleInvokeMethodRequest: "
                           "Found METHOD_CIMMETHODPROVIDER2.");
               }
           }
        }

        if (id == NULL)
        {
           PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
               "handleInvokeMethod: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on "
                       "InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_CIMMETHODPROVIDER:
        {
            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod =
                env->NewStringUTF(
                    request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            Uint32 m=request->inParameters.size();

            jobjectArray jArIn=(jobjectArray)env->NewObjectArray(
                                   m,
                                   jv->CIMArgumentClassRef,
                                   NULL);

            for (Uint32 i=0; i<m; i++) {
              CIMParamValue *parm = new CIMParamValue(request->inParameters[i]);
              jlong jArgRef = DEBUG_ConvertCToJava(CIMParamValue*, jlong, parm);
              jobject jArg = env->NewObject(
                                 jv->CIMArgumentClassRef,
                                 jv->CIMArgumentNewJ,
                                 jArgRef);

              env->SetObjectArrayElement(jArIn,i,jArg);
            }

            jobjectArray jArOut=(jobjectArray)env->NewObjectArray(
                                    24,
                                    jv->CIMArgumentClassRef,
                                    NULL);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      jcop,
                                                      jMethod,
                                                      jArIn,
                                                      jArOut);
            JMPIjvm::checkException(env);

            handler.processing();

            jlong jValueRetRef = env->CallLongMethod(
                                     jValueRet,
                                     JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet = DEBUG_ConvertJavaToC(
                                     jlong,
                                     CIMValue*,
                                     jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0; i<24; i++) {
                jobject jArg = env->GetObjectArrayElement(jArOut,i);

                JMPIjvm::checkException(env);

                if (jArg==NULL)
                   break;

                jlong jpRef = env->CallLongMethod(
                                  jArg,
                                  JMPIjvm::jv.CIMArgumentCInst);
                CIMParamValue *p = DEBUG_ConvertJavaToC(
                                       jlong,
                                       CIMParamValue*,
                                       jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(*p);
            }

            handler.complete();
            break;
        }

        case METHOD_CIMMETHODPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,
                              jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod =
                env->NewStringUTF(request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            Uint32 m=request->inParameters.size();

            jobjectArray jArIn=(jobjectArray)env->NewObjectArray(
                                   m,
                                   jv->CIMArgumentClassRef,
                                   NULL);

            for (Uint32 i=0; i<m; i++) {
              CIMParamValue *parm = new CIMParamValue(request->inParameters[i]);
              jlong jArgRef = DEBUG_ConvertCToJava(CIMParamValue*, jlong, parm);
              jobject jArg = env->NewObject(
                                 jv->CIMArgumentClassRef,
                                 jv->CIMArgumentNewJ,
                                 jArgRef);

              env->SetObjectArrayElement(jArIn,i,jArg);
            }

            jobjectArray jArOut=(jobjectArray)env->NewObjectArray(
                                    24,
                                    jv->CIMArgumentClassRef,
                                    NULL);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      joc,
                                                      jcop,
                                                      jMethod,
                                                      jArIn,
                                                      jArOut);
            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            jlong     jValueRetRef = env->CallLongMethod(
                                         jValueRet,
                                         JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet = DEBUG_ConvertJavaToC(
                                     jlong,
                                     CIMValue*,
                                     jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0; i<24; i++) {
                jobject jArg = env->GetObjectArrayElement(jArOut,i);

                JMPIjvm::checkException(env);

                if (jArg==NULL)
                   break;

                jlong jpRef = env->CallLongMethod(
                                  jArg,
                                  JMPIjvm::jv.CIMArgumentCInst);
                CIMParamValue *p = DEBUG_ConvertJavaToC(
                                       jlong,
                                       CIMParamValue*,
                                       jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(*p);
            }

            handler.complete();
            break;
        }

        case METHOD_METHODPROVIDER:
        {
            jlong   jcopRef = DEBUG_ConvertCToJava(
                                  CIMObjectPath*,
                                  jlong,
                                  objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod = env->NewStringUTF(
                                  request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            jobject jVecIn = env->NewObject(jv->VectorClassRef,jv->VectorNew);

            JMPIjvm::checkException(env);

            for (int i=0,m=request->inParameters.size(); i<m; i++)
            {
                const CIMParamValue &parm = request->inParameters[i];
                const CIMValue v = parm.getValue();
                CIMProperty *p = new CIMProperty(
                                     parm.getParameterName(),
                                     v,
                                     v.getArraySize());
                jlong jpRef = DEBUG_ConvertCToJava (CIMProperty*, jlong, p);
                jobject jp = env->NewObject(
                                 jv->CIMPropertyClassRef,
                                 jv->CIMPropertyNewJ,
                                 jpRef);

                env->CallVoidMethod(jVecIn,jv->VectorAddElement,jp);
             }

            jobject jVecOut=env->NewObject(jv->VectorClassRef,jv->VectorNew);
            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      jcop,
                                                      jMethod,
                                                      jVecIn,
                                                      jVecOut);
            JMPIjvm::checkException(env);

            handler.processing();

            jlong jValueRetRef = env->CallLongMethod(
                                     jValueRet,
                                     JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet = DEBUG_ConvertJavaToC(
                                     jlong,
                                     CIMValue*,
                                     jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0,m=env->CallIntMethod(jVecOut,JMPIjvm::jv.VectorSize);
                  i<m;
                  i++)
            {
                JMPIjvm::checkException(env);

                jobject jProp = env->CallObjectMethod(
                                    jVecOut,
                                    JMPIjvm::jv.VectorElementAt,
                                    i);

                JMPIjvm::checkException(env);

                jlong jpRef = env->CallLongMethod(
                                  jProp,
                                  JMPIjvm::jv.CIMPropertyCInst);
                CIMProperty *p = DEBUG_ConvertJavaToC(
                                     jlong,
                                     CIMProperty*,
                                     jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(
                    CIMParamValue(p->getName().getString(),
                                  p->getValue()));
            }

            handler.complete();
            break;
        }

        case METHOD_METHODPROVIDER2:
        {
            jlong   jocRef = DEBUG_ConvertCToJava(
                                 OperationContext*,
                                 jlong,
                                 &request->operationContext);
            jobject joc    = env->NewObject(
                                 jv->OperationContextClassRef,
                                 jv->OperationContextNewJ,
                                 jocRef);

            jlong jcopRef = DEBUG_ConvertCToJava(
                                CIMObjectPath*,
                                jlong,
                                objectPath);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,
                               jcopRef);

            JMPIjvm::checkException(env);

            jstring jMethod = env->NewStringUTF(
                                  request->methodName.getString().getCString());

            JMPIjvm::checkException(env);

            jobject jVecIn = env->NewObject(jv->VectorClassRef,jv->VectorNew);

            JMPIjvm::checkException(env);

            for (int i=0,m=request->inParameters.size(); i<m; i++)
            {
                const CIMParamValue &parm  = request->inParameters[i];
                const CIMValue       v     = parm.getValue();
                CIMProperty *p = new CIMProperty(
                                     parm.getParameterName(),
                                     v,
                                     v.getArraySize());
                jlong jpRef = DEBUG_ConvertCToJava(CIMProperty*, jlong, p);
                jobject jp = env->NewObject(
                                 jv->CIMPropertyClassRef,
                                 jv->CIMPropertyNewJ,
                                 jpRef);

                env->CallVoidMethod(jVecIn,jv->VectorAddElement,jp);
             }

            jobject jVecOut=env->NewObject(jv->VectorClassRef,jv->VectorNew);
            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            jobject jValueRet = env->CallObjectMethod((jobject)pr.jProvider,
                                                      id,
                                                      joc,
                                                      jcop,
                                                      jMethod,
                                                      jVecIn,
                                                      jVecOut);
            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }

            handler.processing();

            jlong jValueRetRef = env->CallLongMethod(
                                     jValueRet,
                                     JMPIjvm::jv.CIMValueCInst);
            CIMValue *valueRet = DEBUG_ConvertJavaToC(
                                     jlong,
                                     CIMValue*,
                                     jValueRetRef);

            handler.deliver(*valueRet);

            for (int i=0,m=env->CallIntMethod(jVecOut,JMPIjvm::jv.VectorSize);
                 i<m;
                 i++)
            {
                JMPIjvm::checkException(env);

                jobject jProp = env->CallObjectMethod(
                                    jVecOut,
                                    JMPIjvm::jv.VectorElementAt,
                                    i);

                JMPIjvm::checkException(env);

                jlong jpRef = env->CallLongMethod(
                                  jProp,
                                  JMPIjvm::jv.CIMPropertyCInst);
                CIMProperty *p = DEBUG_ConvertJavaToC(
                                     jlong,
                                     CIMProperty*,
                                     jpRef);

                JMPIjvm::checkException(env);

                handler.deliverParamValue(
                    CIMParamValue(p->getName().getString(),
                                  p->getValue()));
            }

            handler.complete();
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleInvokeMethodRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

int LocateIndicationProviderNames(
    const CIMInstance& pInstance,
    const CIMInstance& pmInstance,
    String& providerName,
    String& location)
{
    Uint32 pos = pInstance.findProperty(PEGASUS_PROPERTYNAME_NAME);
    pInstance.getProperty(pos).getValue().get(providerName);

    pos = pmInstance.findProperty(CIMName ("Location"));
    pmInstance.getProperty(pos).getValue().get(location);
    return 0;
}

WQLSelectStatement *
newSelectExp (String& query,
              String& queryLanguage)
{
   WQLSelectStatement *stmt = new WQLSelectStatement (queryLanguage, query);

   try
   {
      WQLParser::parse (query, *stmt);
   }
   catch (const Exception &e)
   {
      cerr << "Error: newSelectExp caught: " << e.getMessage () << endl;
   }

   return stmt;
}

Message * JMPIProviderManager::handleCreateSubscriptionRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleCreateSubscriptionRequest");

    HandlerIntroInd(CreateSubscription,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_EVENTPROVIDER,
       METHOD_EVENTPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION  eMethodFound = METHOD_UNKNOWN;
    JNIEnv         *env          = NULL;

    try {
        String               fileName,
                             providerName,
                             providerLocation;
        CIMInstance          req_provider,
                             req_providerModule;
        ProviderIdContainer  pidc =
            (ProviderIdContainer) request->operationContext.get(
                ProviderIdContainer::NAME);

        req_provider = pidc.getProvider ();
        req_providerModule = pidc.getModule ();

        LocateIndicationProviderNames(
            req_provider,
            req_providerModule,
            providerName,
            providerLocation);

        fileName = resolveFileName(providerLocation);

        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleCreateSubscriptionRequest: "
                 "name space = %s provider name = %s provider filename = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)providerName.getCString(),
             (const char*)fileName.getCString()
             ));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph =
            providerManager.getProvider(
                fileName,
                providerName,
                String::EMPTY);

        //
        //  Save the provider instance from the request
        //
        ph.GetProvider ().setProviderInstance (req_provider);

        JMPIProvider &pr = ph.GetProvider ();

        //
        //  Increment count of current subscriptions for this provider
        //
        pr.testIfZeroAndIncrementSubscriptions ();

        SubscriptionFilterConditionContainer sub_cntr =
            request->operationContext.get(
                SubscriptionFilterConditionContainer::NAME);
        indProvRecord *prec = NULL;
        bool fNewPrec = false;

        {
           AutoMutex lock (mutexProvTab);

           provTab.lookup (providerName, prec);

           if (!prec)
           {
               fNewPrec = true;

               prec = new indProvRecord ();

               // convert arguments
               prec->ctx = new OperationContext ();

               prec->ctx->insert(
                   request->operationContext.get(
                       IdentityContainer::NAME));
               prec->ctx->insert(
                   request->operationContext.get(
                       AcceptLanguageListContainer::NAME));
               prec->ctx->insert(
                   request->operationContext.get(
                       ContentLanguageListContainer::NAME));
               prec->ctx->insert(
                   request->operationContext.get(
                       SubscriptionInstanceContainer::NAME));
               prec->ctx->insert(
                   request->operationContext.get(
                       SubscriptionFilterConditionContainer::NAME));

               prec->enabled = true;

               prec->handler = new EnableIndicationsResponseHandler(
                                   0,
                                   0,
                                   req_provider,
                                   _indicationCallback,
                                   _responseChunkCallback);

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                             "handleCreateSubscriptionRequest: "
                       "Adding %s to provTab.",
                   (const char*)providerName.getCString()));

               provTab.insert (providerName, prec);
           }
        }

        {
           AutoMutex lock (prec->mutex);

           prec->count++;
        }

        // Add a selection record for JNI CIMOMHandle deliverEvent calls
        indSelectRecord *srec = new indSelectRecord ();

        {
           srec->query         = request->query;
           srec->queryLanguage = sub_cntr.getQueryLanguage ();
           srec->propertyList  = request->propertyList;

           CIMOMHandleQueryContext *qContext =
               new CIMOMHandleQueryContext(
                   CIMNamespaceName(
                       request->nameSpace.getString()),
                   *pr._cimom_handle);
           srec->qContext = qContext;

           CIMObjectPath sPath = request->subscriptionInstance.getPath();
           Array<CIMKeyBinding> kb;

           // Technically we only need Name and Handler for uniqueness
           kb = sPath.getKeyBindings ();

           // Add an entry for every provider.
           kb.append (CIMKeyBinding ("Provider",
                                     pr.getName (),
                                     CIMKeyBinding::STRING));

           sPath.setKeyBindings (kb);

           AutoMutex lock (mutexSelxTab);

           PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
               "handleCreateSubscriptionRequest: "
                   "Adding %s to selxTab.",
               (const char*)sPath.toString().getCString()));

           selxTab.insert (sPath.toString (), srec);

           PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                 "handleCreateSubscriptionRequest: "
                     "For selxTab %s , srec = %p, qContext = %p",
                 (const char*)sPath.toString().getCString(),
                 srec,qContext));
        }

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
            "handleCreateSubscriptionRequest: Calling provider: %s",
            (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleCreateSubscriptionRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType (pidc,
                          interfaceType,
                          interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "activateFilter",
                    "(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleCreateSubscriptionRequest: "
                       "Found METHOD_EVENTPROVIDER.");
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "activateFilter",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                        "Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;"
                            "Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleCreateSubscriptionRequest: "
                       "Found METHOD_EVENTPROVIDER2.");
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleCreateSubscriptionRequest: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on"
                       " InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_EVENTPROVIDER:
        {
            WQLSelectStatement *stmt = newSelectExp(
                                           srec->query,
                                           srec->queryLanguage);
            jlong jStmtRef = DEBUG_ConvertCToJava(
                                 WQLSelectStatement *,
                                 jlong,
                                 stmt);
            jobject jSelectExp = env->NewObject(
                                     jv->SelectExpClassRef,
                                     jv->SelectExpNewJ,
                                     jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(
                                request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop     = new CIMObjectPath (System::getHostName(),
                                                        request->nameSpace,
                                                        request->classNames[0]);
            jlong jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fNewPrec);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_EVENTPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,jocRef);

            WQLSelectStatement *stmt = newSelectExp(srec->query,
                                                    srec->queryLanguage);
            jlong   jStmtRef   = DEBUG_ConvertCToJava(
                                     WQLSelectStatement *,
                                     jlong,
                                     stmt);
            jobject jSelectExp = env->NewObject(
                                     jv->SelectExpClassRef,
                                     jv->SelectExpNewJ,
                                     jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(
                                request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop = new CIMObjectPath (System::getHostName(),
                                                    request->nameSpace,
                                                    request->classNames[0]);
            jlong jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fNewPrec);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(joc,JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleCreateSubscriptionRequest: Unknown method provider!");
            break;
        }
        }
    }
    HandlerCatch(handler);

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDeleteSubscriptionRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleDeleteSubscriptionRequest");

    HandlerIntroInd(DeleteSubscription,message,request,response,handler);

    typedef enum {
       METHOD_UNKNOWN = 0,
       METHOD_EVENTPROVIDER,
       METHOD_EVENTPROVIDER2,
    } METHOD_VERSION;
    METHOD_VERSION           eMethodFound = METHOD_UNKNOWN;
    JNIEnv                  *env          = NULL;
    bool                     fFreePrec    = false;
    indProvRecord           *prec         = NULL;
    indSelectRecord         *srec         = NULL;

    try {
        String              fileName,
                            providerName,
                            providerLocation;
        CIMInstance         req_provider,
                            req_providerModule;
        ProviderIdContainer pidc = (ProviderIdContainer)
                                       request->operationContext.get(
                                           ProviderIdContainer::NAME);

        req_provider       = pidc.getProvider ();
        req_providerModule = pidc.getModule ();

        LocateIndicationProviderNames (req_provider,
                                       req_providerModule,
                                       providerName,
                                       providerLocation);

        fileName = resolveFileName (providerLocation);

        PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
             "handleDeleteSubscriptionRequest: "
                 "name space = %s provider name = %s provider filename = %s",
             (const char*)request->nameSpace.getString().getCString(),
             (const char*)providerName.getCString(),
             (const char*)fileName.getCString()
             ));

        // get cached or load new provider module
        JMPIProvider::OpProviderHolder ph = providerManager.getProvider(
                                                fileName,
                                                providerName,
                                                String::EMPTY);

        JMPIProvider &pr = ph.GetProvider ();

        {
           AutoMutex lock (mutexProvTab);

           provTab.lookup (providerName, prec);
        }

        {
           AutoMutex lock (prec->mutex);

           if (--prec->count <= 0)
           {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                    "handleDeleteSubscriptionRequest: "
                       "Removing %s from provTab.",
                    (const char*)providerName.getCString()));

               provTab.remove (providerName);

               fFreePrec = true;
           }
        }

        {
           CIMObjectPath sPath = request->subscriptionInstance.getPath();
           Array<CIMKeyBinding> kb;

           // Technically we only need Name and Handler for uniqueness
           kb = sPath.getKeyBindings ();

           // Add an entry for every provider.
           kb.append (CIMKeyBinding ("Provider",
                                     pr.getName (),
                                     CIMKeyBinding::STRING));

           sPath.setKeyBindings (kb);

           String sPathString = sPath.toString ();

           AutoMutex lock (mutexSelxTab);

           PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "handleDeleteSubscriptionRequest: "
                   "Removing %s from selxTab.",
                (const char*)sPathString.getCString()));

           if (!selxTab.lookup (sPathString, srec))
           {
               PEGASUS_ASSERT(0);
           }

           PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                 "handleDeleteSubscriptionRequest: "
                     "For selxTab %s , srec = %p, qContext = %p",
                 (const char*)sPathString.getCString(),
                 srec,srec->qContext));

           selxTab.remove (sPathString);
        }

        PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL3,
            "handleDeleteSubscriptionRequest: "
                "Calling provider: %s",
            (const char*)pr.getName().getCString()));

        JvmVector *jv = 0;

        env = JMPIjvm::attachThread(&jv);

        if (!env)
        {
            PEG_TRACE_CSTRING( TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleDeleteSubscriptionRequest: "
                    "Could not initialize the JVM (Java Virtual Machine) "
                    "runtime environment.");

            PEG_METHOD_EXIT();

            throw PEGASUS_CIM_EXCEPTION_L(
                CIM_ERR_FAILED,
                MessageLoaderParms(
                    "ProviderManager.JMPI.JMPIProviderManager.INIT_JVM_FAILED",
                    "Could not initialize the JVM (Java Virtual Machine) "
                        "runtime environment."));
        }

        JMPIProvider::pm_service_op_lock op_lock(&pr);

        jmethodID id               = NULL;
        String    interfaceType;
        String    interfaceVersion;

        getInterfaceType(
            request->operationContext.get(ProviderIdContainer::NAME),
            interfaceType,
            interfaceVersion);

        if (interfaceType == "JMPI")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "deActivateFilter",
                    "(Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;"
                        "Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleDeleteSubscriptionRequest: "
                       "Found METHOD_EVENTPROVIDER.");
           }
        }
        else if (interfaceType == "JMPIExperimental")
        {
           id = env->GetMethodID(
                    (jclass)pr.jProviderClass,
                    "deActivateFilter",
                    "(Lorg/pegasus/jmpi/OperationContext;"
                         "Lorg/pegasus/jmpi/SelectExp;Ljava/lang/String;"
                             "Lorg/pegasus/jmpi/CIMObjectPath;Z)V");

           if (id != NULL)
           {
               eMethodFound = METHOD_EVENTPROVIDER2;
               PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                   "handleDeleteSubscriptionRequest: "
                       "Found METHOD_EVENTPROVIDER2.");
           }
        }

        if (id == NULL)
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleDeleteSubscriptionRequest: No method provider found!");

           PEG_METHOD_EXIT();

           throw PEGASUS_CIM_EXCEPTION_L(
               CIM_ERR_FAILED,
               MessageLoaderParms(
                   "ProviderManager.JMPI.JMPIProviderManager.METHOD_NOT_FOUND",
                   "Could not find a method for the provider based on"
                        " InterfaceType."));
        }

        JMPIjvm::checkException(env);

        switch (eMethodFound)
        {
        case METHOD_EVENTPROVIDER:
        {
            WQLSelectStatement *stmt = newSelectExp(
                                           srec->query,
                                           srec->queryLanguage);
            jlong jStmtRef = DEBUG_ConvertCToJava(
                                 WQLSelectStatement *,
                                 jlong,
                                 stmt);
            jobject jSelectExp = env->NewObject(
                                     jv->SelectExpClassRef,
                                     jv->SelectExpNewJ,jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(
                                request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop = new CIMObjectPath(
                                     System::getHostName(),
                                     request->nameSpace,
                                     request->classNames[0]);
            jlong jcopRef = DEBUG_ConvertCToJava(CIMObjectPath*, jlong, cop);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fFreePrec);

            JMPIjvm::checkException(env);
            break;
        }

        case METHOD_EVENTPROVIDER2:
        {
            jlong jocRef = DEBUG_ConvertCToJava(
                               OperationContext*,
                               jlong,
                               &request->operationContext);
            jobject joc = env->NewObject(
                              jv->OperationContextClassRef,
                              jv->OperationContextNewJ,jocRef);

            WQLSelectStatement *stmt       = newSelectExp (srec->query,
                                                           srec->queryLanguage);
            jlong jStmtRef   = DEBUG_ConvertCToJava(
                                   WQLSelectStatement *,
                                   jlong,
                                   stmt);
            jobject jSelectExp = env->NewObject(
                                     jv->SelectExpClassRef,
                                     jv->SelectExpNewJ,
                                     jStmtRef);

            JMPIjvm::checkException(env);

            jstring jType = env->NewStringUTF(
                                request->nameSpace.getString().getCString());

            JMPIjvm::checkException(env);

            CIMObjectPath *cop = new CIMObjectPath (System::getHostName(),
                                                    request->nameSpace,
                                                    request->classNames[0]);
            jlong jcopRef = DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
            jobject jcop = env->NewObject(
                               jv->CIMObjectPathClassRef,
                               jv->CIMObjectPathNewJ,jcopRef);

            JMPIjvm::checkException(env);

            StatProviderTimeMeasurement providerTime(response);

            env->CallVoidMethod ((jobject)pr.jProvider,
                                 id,
                                 joc,
                                 jSelectExp,
                                 jType,
                                 jcop,
                                 (jboolean)fFreePrec);

            JMPIjvm::checkException(env);

            if (joc)
            {
               env->CallVoidMethod(
                   joc,
                   JMPIjvm::jv.OperationContextUnassociate);

               JMPIjvm::checkException(env);
            }
            break;
        }

        case METHOD_UNKNOWN:
        {
            PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL2,
                "handleDeleteSubscriptionRequest: Unknown method provider!");
            break;
        }
        }

        //
        //  Decrement count of current subscriptions for this provider
        //
        pr.decrementSubscriptionsAndTestIfZero ();
    }
    HandlerCatch(handler);

    if (srec)
    {
       delete srec->qContext;
    }
    delete srec;

    if (fFreePrec)
    {
       delete prec->ctx;
       delete prec->handler;
       delete prec;
    }

    if (env) JMPIjvm::detachThread();

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleDisableModuleRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleDisableModuleRequest");

    CIMDisableModuleRequestMessage * request =
        dynamic_cast<CIMDisableModuleRequestMessage *>
        (const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    // get provider module name
    String moduleName;
    CIMInstance mInstance = request->providerModule;
    Uint32 pos = mInstance.findProperty(PEGASUS_PROPERTYNAME_NAME);

    if(pos != PEG_NOT_FOUND)
    {
        mInstance.getProperty(pos).getValue().get(moduleName);
    }

    Boolean disableProviderOnly = request->disableProviderOnly;

    Array<Uint16> operationalStatus;
    // Assume success.
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_STOPPED);

    //
    // Unload providers
    //
    Array<CIMInstance> _pInstances = request->providers;

    CIMDisableModuleResponseMessage* response =
        dynamic_cast<CIMDisableModuleResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);
    response->operationalStatus = operationalStatus;

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleEnableModuleRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleEnableModuleRequest");

    CIMEnableModuleRequestMessage * request =
        dynamic_cast<CIMEnableModuleRequestMessage *>
            (const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    Array<Uint16> operationalStatus;
    operationalStatus.append(CIM_MSE_OPSTATUS_VALUE_STOPPED);

    CIMEnableModuleResponseMessage* response =
        dynamic_cast<CIMEnableModuleResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);
    response->operationalStatus = operationalStatus;

    PEG_METHOD_EXIT();

    return(response);
}

Message * JMPIProviderManager::handleStopAllProvidersRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleStopAllProvidersRequest");

    CIMStopAllProvidersRequestMessage * request =
        dynamic_cast<CIMStopAllProvidersRequestMessage *>
            (const_cast<Message *>(message));

    PEGASUS_ASSERT(request != 0);

    CIMStopAllProvidersResponseMessage* response =
        dynamic_cast<CIMStopAllProvidersResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    // tell the provider manager to shutdown all the providers
    providerManager.shutdownAllProviders();

    PEG_METHOD_EXIT();

    return(response);
}

Message* JMPIProviderManager::handleIndicationServiceDisabledRequest(
    Message* message)
{
    PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,
        "JMPIProviderManager::_handleIndicationServiceDisabledRequest");

    CIMIndicationServiceDisabledRequestMessage* request =
        dynamic_cast<CIMIndicationServiceDisabledRequestMessage*>(message);
    PEGASUS_ASSERT(request != 0);

    CIMIndicationServiceDisabledResponseMessage* response =
        dynamic_cast<CIMIndicationServiceDisabledResponseMessage*>(
            request->buildResponse());
    PEGASUS_ASSERT(response != 0);

    _subscriptionInitComplete = false;

    PEG_METHOD_EXIT ();
    return response;
}

Message * JMPIProviderManager::handleSubscriptionInitCompleteRequest(
    const Message * message)
{
    PEG_METHOD_ENTER (TRC_PROVIDERMANAGER,
     "JMPIProviderManager::handleSubscriptionInitCompleteRequest");

    CIMSubscriptionInitCompleteRequestMessage * request =
        dynamic_cast <CIMSubscriptionInitCompleteRequestMessage *>
            (const_cast <Message *> (message));

    PEGASUS_ASSERT (request != 0);

    CIMSubscriptionInitCompleteResponseMessage * response =
        dynamic_cast <CIMSubscriptionInitCompleteResponseMessage *>
            (request->buildResponse ());

    PEGASUS_ASSERT (response != 0);

    //
    //  Set indicator
    //
    _subscriptionInitComplete = true;

    //
    //  For each provider that has at least one subscription, call
    //  provider's enableIndications method
    //
    Array <JMPIProvider *> enableProviders;

    enableProviders = providerManager.getIndicationProvidersToEnable ();

    Uint32 numProviders = enableProviders.size ();

    PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "handleSubscriptionInitCompleteRequest: numProviders = %d ",
        numProviders));

    PEG_METHOD_EXIT ();
    return (response);
}

Message * JMPIProviderManager::handleUnsupportedRequest(
    const Message * message) throw()
{
    PEG_METHOD_ENTER(
        TRC_PROVIDERMANAGER,
        "JMPIProviderManager::handleUnsupportedRequest");

    CIMRequestMessage* request =
        dynamic_cast<CIMRequestMessage *>(const_cast<Message *>(message));
    PEGASUS_ASSERT(request != 0 );

    CIMResponseMessage* response = request->buildResponse();
    response->cimException =
        PEGASUS_CIM_EXCEPTION(CIM_ERR_NOT_SUPPORTED, String::EMPTY);

    PEG_METHOD_EXIT();
    return response;
}

ProviderName JMPIProviderManager::_resolveProviderName(
    const ProviderIdContainer & providerId)
{
    String providerName;
    String fileName;
    String moduleName;
    CIMValue genericValue;

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue();
    genericValue.get(moduleName);

    genericValue = providerId.getProvider().getProperty(
        providerId.getProvider().findProperty(
            PEGASUS_PROPERTYNAME_NAME)).getValue();
    genericValue.get(providerName);

    genericValue = providerId.getModule().getProperty(
        providerId.getModule().findProperty("Location")).getValue();
    genericValue.get(fileName);
    fileName = resolveFileName(fileName);

    return ProviderName(moduleName, providerName, fileName);
}

String JMPIProviderManager::resolveFileName(String fileName)
{
    String name = ConfigManager::getHomedPath(
        ConfigManager::getInstance()->getCurrentValue("providerDir"));
    // physfilename = everything up to the delimiter pointing at class start
    // in case there is no delimiter anymore, it takes the entire filename
    String physfilename = fileName.subString(0, fileName.find(":"));
    // look in all(multiple) homed pathes for the physical file
    name = FileSystem::getAbsoluteFileName(name, physfilename);
    // construct back the fully specified jar:<classname> provider name
    name = FileSystem::extractFilePath(name) + fileName;
    return name;
}

PEGASUS_NAMESPACE_END
