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

#include "JMPIImpl.h"

#if defined(PEGASUS_OS_TYPE_WINDOWS)
#include <Pegasus/General/DynamicLibrary.h>
#else
#include <dlfcn.h>
#endif
#include <iostream>
#include <sstream>

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/OperationContext.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Provider/CIMOMHandle.h>
#include <Pegasus/Client/CIMClient.h>
#include <Pegasus/ProviderManager2/JMPI/JMPIProviderManager.h>
#include <Pegasus/WQL/WQLSelectStatement.h>
#include <Pegasus/WQL/WQLParser.h>
#define CALL_SIGN_WQL "WQL"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

JavaVM *JMPIjvm::jvm=NULL;
JvmVector JMPIjvm::jv;

#include "Convert.h"

JMPIjvm::ClassTable  JMPIjvm::_classTable;
JMPIjvm::ObjectTable JMPIjvm::_objectTable;

const char* classNames[]={
/*00*/ "java/util/Vector" ,
/*01*/ "java/lang/Boolean",
/*02*/ "java/lang/Byte",
/*03*/ "java/lang/Short",
/*04*/ "java/lang/Integer",
/*05*/ "java/lang/Long",
/*06*/ "java/lang/Float",
/*07*/ "java/lang/Double",
/*08*/ "org/pegasus/jmpi/UnsignedInt8",
/*09*/ "org/pegasus/jmpi/UnsignedInt16",
/*10*/ "org/pegasus/jmpi/UnsignedInt32",
/*11*/ "org/pegasus/jmpi/UnsignedInt64",
/*12*/ "org/pegasus/jmpi/CIMObjectPath",
/*13*/ "org/pegasus/jmpi/CIMException",
/*14*/ "java/math/BigInteger",
/*15*/ "org/pegasus/jmpi/CIMProperty",
/*16*/ "org/pegasus/jmpi/CIMOMHandle",
/*17*/ "org/pegasus/jmpi/CIMClass",
/*18*/ "org/pegasus/jmpi/CIMInstance",
/*19*/ "org/pegasus/jmpi/CIMValue",
/*20*/ "java/lang/Object",
/*21*/ "java/lang/Throwable",
/*22*/ "java/lang/String",
/*23*/ "org/pegasus/jmpi/JarClassLoader",
/*24*/ "org/pegasus/jmpi/CIMDateTime",
/*25*/ "org/pegasus/jmpi/SelectExp",
/*26*/ "org/pegasus/jmpi/CIMQualifier",
/*27*/ "org/pegasus/jmpi/CIMQualifierType",
/*28*/ "org/pegasus/jmpi/CIMFlavor",
/*29*/ "org/pegasus/jmpi/CIMArgument",
/*30*/ "org/pegasus/jmpi/CIMInstanceException",
/*31*/ "org/pegasus/jmpi/CIMObject",
/*32*/ "java/lang/Character",
/*33*/ "org/pegasus/jmpi/OperationContext",
/*34*/ "java/lang/Class",
/*35*/ "java/io/ByteArrayOutputStream",
/*36*/ "java/io/PrintStream"
};

const METHOD_STRUCT instanceMethodNames[]={
/*00 VectorNew                      */
{ /*Vector                */
    0,
    "<init>",
    "()V" },
/*01 BooleanNewZ                    */
{ /*Boolean               */
    1,
    "<init>",
    "(Z)V" },
/*02 ByteNewB                       */
{ /*Byte                  */
    2,
    "<init>",
    "(B)V" },
/*03 ShortNewS                      */
{ /*Short                 */
    3,
    "<init>",
    "(S)V" },
/*04 IntegerNewI                    */
{ /*Integer               */
    4,
    "<init>",
    "(I)V" },
/*05 LongNewJ                       */
{ /*Long                  */
    5,
    "<init>",
    "(J)V" },
/*06 FloatNewF                      */
{ /*Float                 */
    6,
    "<init>",
    "(F)V" },
/*07 DoubleNewD                     */
{ /*Double                */
    7,
    "<init>",
    "(D)V" },
/*08 UnsignedInt8NewS               */
{ /*UnsignedInt8          */
    8,
    "<init>",
    "(S)V" },
/*09 UnsignedInt16NewI              */
{ /*UnsignedInt16         */
    9,
    "<init>",
    "(I)V" },
/*10 UnsignedInt32NewJ              */
{ /*UnsignedInt32         */
    10,
    "<init>",
    "(J)V" },
/*11 UnsignedInt64NewBi             */
{ /*UnsignedInt64         */
    11,
    "<init>",
    "(Ljava/math/BigInteger;)V" },
/*12 CIMObjectPathNewJ              */
{ /*CIMObjectPath         */
    12,
    "<init>",
    "(J)V" },
/*13 CIMExceptionNewSt              */
{ /*CIMException          */
    13,
    "<init>",
    "(Ljava/lang/String;)V" },
/*14 CIMPropertyNewJ                */
{ /*CIMProperty           */
    15,
    "<init>",
    "(J)V" },
/*15 VectorAddElement               */
{ /*Vector                */
    0,
    "addElement",
    "(Ljava/lang/Object;)V" },
/*16 VectorElementAt                */
{ /*Vector                */
    0,
    "elementAt",
    "(I)Ljava/lang/Object;" },
/*17 CIMOMHandleNewJSt              */
{ /*CIMOMHandle           */
    16,
    "<init>",
    "(JLjava/lang/String;)V" },
/*18 CIMExceptionNewI               */
{ /*CIMException          */
    13,
    "<init>",
    "(I)V" },
/*19 CIMClassNewJ                   */
{ /*CIMClass              */
    17,
    "<init>",
    "(J)V" },
/*20 CIMInstanceNewJ                */
{ /*CIMInstance           */
    18,
    "<init>",
    "(J)V" },
/*21 CIMObjectPathCInst             */
{ /*CIMObjectPath         */
    12,
    "cInst",
    "()J" },
/*22 CIMInstanceCInst               */
{ /*CIMInstance           */
    18,
    "cInst",
    "()J" },
/*23 CIMClassCInst                  */
{ /*CIMClass              */
    17,
    "cInst",
    "()J" },
/*24 ObjectToString                 */
{ /*Object                */
    20,
    "toString",
    "()Ljava/lang/String;" },
/*25 ThrowableGetMessage            */
{ /*Throwable             */
    21,
    "getMessage",
    "()Ljava/lang/String;" },
/*26 CIMExceptionGetID              */
{ /*CIMException          */
    13,
    "getID",
    "()Ljava/lang/String;" },
/*27 VectorSize                     */
{ /*Vector                */
    0,
    "size",
    "()I" },
/*28 CIMPropertyCInst               */
{ /*CIMProperty           */
    15,
    "cInst",
    "()J" },
/*29 CIMOMHandleGetClass            */
{ /*CIMOMHandle           */
    16,
    "getClass",
    "(Lorg/pegasus/jmpi/CIMObjectPath;Z)Lorg/pegasus/jmpi/CIMClass;" },
/*30 VectorRemoveElementAt          */
{ /*Vector                */
    0,
    "removeElementAt",
    "(I)V" },
/*31 CIMValueCInst                  */
{ /*CIMValue              */
    19,
    "cInst",
    "()J" },
/*32 CIMExceptionNewISt             */
{ /*CIMException          */
    13,
    "<init>",
    "(ILjava/lang/String;)V" },
/*33 CIMExceptionGetCode            */
{ /*CIMException          */
    13,
    "getCode",
    "()I" },
/*34 CIMDateTimeNewJ                */
{ /*CIMDateTime           */
    24,
    "<init>",
    "(J)V" },
/*35 SelectExpNewJ                  */
{ /*SelectExp             */
    25,
    "<init>",
    "(J)V" },
/*36 CIMQualifierNewJ               */
{ /*CIMQualifier          */
    26,
    "<init>",
    "(J)V" },
/*37 CIMFlavorNewI                  */
{ /*CIMFlavor             */
    28,
    "<init>",
    "(I)V" },
/*38 CIMFlavorGetFlavor             */
{ /*CIMFlavor             */
    28,
    "getFlavor",
    "()I" },
/*39 CIMArgumentCInst               */
{ /*CIMArgument           */
    29,
    "cInst",
    "()J" },
/*40 CIMArgumentNewJ                */
{ /*CIMArgument           */
    29,
    "<init>",
    "(J)V" },
/*41 CIMExceptionNew                */
{ /*CIMException          */
    13,
    "<init>",
    "()V" },
/*42 CIMExceptionNewStOb            */
{ /*CIMException          */
    13,
    "<init>",
    "(Ljava/lang/String;Ljava/lang/Object;)V" },
/*43 CIMExceptionNewStObOb          */
{ /*CIMException          */
    13,
    "<init>",
    "(Ljava/lang/String;Ljava/lang/Object;Ljava/lang/Object;)V" },
/*44 CIMExceptionNewStObObOb        */
{ /*CIMException          */
    13,
    "<init>",
    "(Ljava/lang/String;Ljava/lang/Object;"
        "Ljava/lang/Object;Ljava/lang/Object;)V" },
/*45 CIMValueNewJ                   */
{ /*CIMValue              */
    19,
    "<init>",
    "(J)V" },
/*46 CIMObjectNewJZ                 */
{ /*CIMObject             */
    31,
    "<init>",
    "(JZ)V" },
/*47 CharacterNewC                  */
{ /*Character             */
    32,
    "<init>",
    "(C)V" },
/*48 OperationContextNewJ           */
{ /*OperationContext      */
    33,
    "<init>",
    "(J)V" },
/*49 OperationContextUnassociate    */
{ /*OperationContext      */
    33,
    "unassociate",
    "()V" },
/*50 ClassGetInterfaces             */
{ /*Class                 */
    34,
    "getInterfaces",
    "()[Ljava/lang/Class;" },
/*51 ClassGetName                   */
{ /*Class                 */
    34,
    "getName",
    "()Ljava/lang/String;" },
/*52 UnsignedInt64NewStr            */
{ /*UnsignedInt64         */
    11,
    "<init>",
    "(Ljava/lang/String;)V" },
/*53 ByteArrayOutputStreamNew       */
{ /*ByteArrayOutputStream */
    35,
    "<init>",
    "()V" },
/*54 PrintStreamNewOb               */
{ /*PrintStreamNew        */
    36,
    "<init>",
    "(Ljava/io/OutputStream;)V" },
/*55 ThrowablePrintStackTrace       */
{ /*Throwable             */
    21,
    "printStackTrace",
    "(Ljava/io/PrintStream;)V" },
/*56 ByteArrayOutputStreamToString  */
{ /*ByteArrayOutputStream */
    35,
    "toString",
    "()Ljava/lang/String;" }
};

const METHOD_STRUCT staticMethodNames[]={
/*00*/ { 14,
         "valueOf",
         "(J)Ljava/math/BigInteger;" },
/*01*/ { 23,
         "load",
         "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Class;" },
};

static int methodInitDone=0;

jclass    classRefs[sizeof(classNames)/sizeof(classNames[0])];
jmethodID instanceMethodIDs[sizeof(instanceMethodNames) /
                                sizeof(instanceMethodNames[0])];
jmethodID staticMethodIDs[sizeof(staticMethodNames) /
                                sizeof(staticMethodNames[0])];

jclass JMPIjvm::getGlobalClassRef(JNIEnv *env, const char* name)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIjvm::getGlobalClassRef");

  jclass localRefCls = env->FindClass(name);

  if (localRefCls == NULL)
  {
     PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
         "No local Class reference found. (localRefCls==NULL)");
     PEG_METHOD_EXIT();
     return JNI_FALSE;
  }


  jclass globalRefCls = (jclass) env->NewGlobalRef(localRefCls);

#if 0
  jmethodID   jmidToString   = env->GetMethodID(globalRefCls,
                                                "toString",
                                                "()Ljava/lang/String;");
  jstring     jstringResult  = (jstring)env->CallObjectMethod(globalRefCls,
                                                              jmidToString);
  const char *pszResult      = env->GetStringUTFChars(jstringResult, 0);

  PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
      "globalRefCls = %p, name = %s, pszResult = %s",
      globalRefCls,name,pszResult));

  env->ReleaseStringUTFChars (jstringResult, pszResult);
#else

  PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
      "globalRefCls = %p, name = %s",globalRefCls,name));

#endif

  env->DeleteLocalRef(localRefCls);

  PEG_METHOD_EXIT();
  return globalRefCls;
}

JMPIjvm::JMPIjvm()
{
   initJVM ();
}

JMPIjvm::~JMPIjvm()
{
}

int JMPIjvm::cacheIDs(JNIEnv *env)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIjvm::cacheIDs");

   if (methodInitDone == 1)
   {
      PEG_METHOD_EXIT();
      return JNI_TRUE;
   }

   if (methodInitDone == -1)
   {
      PEG_METHOD_EXIT();
      return JNI_FALSE;
   }

   methodInitDone = -1;

   for (unsigned i = 0; i<(sizeof(classNames)/sizeof(classNames[0])); i++)
   {
      if ((classRefs[i] = getGlobalClassRef(env,classNames[i])) == NULL)
      {
         PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
             "Error: Count not find global class ref for %s",classNames[i]));

         PEG_METHOD_EXIT();
         return JNI_FALSE;
      }
   }

   unsigned instanceMethodNamesSize =
       sizeof(instanceMethodNames)/sizeof(instanceMethodNames[0]);

   for (unsigned j = 0; j<instanceMethodNamesSize; j++)
   {
       instanceMethodIDs[j] = env->GetMethodID(
                                  classRefs[instanceMethodNames[j].clsIndex],
                                  instanceMethodNames[j].methodName,
                                  instanceMethodNames[j].signature);
       if (instanceMethodIDs[j] == NULL)
       {
           PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
               "Error: Could not get instance method id for %s:%s",
               classNames[instanceMethodNames[j].clsIndex],
               instanceMethodNames[j].methodName));

           PEG_METHOD_EXIT();
         return 0;
      }
   }
   unsigned staticMethodNamesSize =
       sizeof(staticMethodNames) / sizeof(staticMethodNames[0]);
   for (unsigned k = 0; k<staticMethodNamesSize; k++)
   {
       staticMethodIDs[k] = env->GetStaticMethodID(
                                classRefs[staticMethodNames[k].clsIndex],
                                staticMethodNames[k].methodName,
                                staticMethodNames[k].signature);

       if (staticMethodIDs[k] == NULL)
       {
           PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
               "Error: Could not get static method id for %s:%s",
               classNames[staticMethodNames[k].clsIndex],
               staticMethodNames[k].methodName));

           PEG_METHOD_EXIT();
           return 0;
       }
   }

   jv.env = env;
   jv.classRefs = classRefs;
   jv.instMethodIDs = instanceMethodIDs;
   jv.staticMethodIDs = staticMethodIDs;
   jv.instanceMethodNames = instanceMethodNames;

   methodInitDone = 1;

   PEG_METHOD_EXIT();
   return JNI_TRUE;
}

static void throwCIMException (JNIEnv *env, char *e)
{
   JMPIjvm::cacheIDs (env);

   env->ThrowNew (JMPIjvm::jv.CIMExceptionClassRef, e);
}

int JMPIjvm::destroyJVM ()
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JPIjvm::destroyJVM");

#ifdef JAVA_DESTROY_VM_WORKS
   if (jvm!= NULL)
   {
      JvmVector *jv;

      attachThread(&jv);

      jvm->DestroyJavaVM();

      jvm = NULL;

      PEG_METHOD_EXIT();
      return 0;
   }
#endif

   PEG_METHOD_EXIT();
   return -1;
}

Mutex JMPIjvm::_initMutex;

int JMPIjvm::initJVM ()
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JPIjvm::initJVM");

   AutoMutex lock (_initMutex);

   if (jvm != NULL)
   {
      return 0;
   }

   JavaVMInitArgs vm_args;
   jint res;
   char *envstring;
   JNIEnv *env;
   JavaVMOption *poptions = 0;
   int maxoption = 0;
   typedef struct _JVMOptions {
      const char *pszEnvName;
      const char *pszPrefix;
      bool        fSplit;
   } JVMOPTIONS;
   Array<std::string> JNIoptions;
   static JVMOPTIONS aEnvOptions[] = {
      { "CLASSPATH",                           "-Djava.class.path=", false },
      { "PEGASUS_JMPI_MAX_HEAP",               "-Xmx"              , false },
      { "PEGASUS_JMPI_INITIAL_HEAP",           "-Xms"              , false },
      { "PEGASUS_JMPI_JAVA_THREAD_STACK_SIZE", "-Xss"              , false },
      { "PEGASUS_JMPI_VERBOSE",                "-verbose:"         , true  }
   };
   std::ostringstream oss;

   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL4,
       "Start to initialize the JVM.");

   jv.initRc = 0;

   envstring = getenv("CLASSPATH");
   if (envstring == NULL)
   {
      jv.initRc = 1;

      PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
          "No CLASSPATH environment variable found.");

      PEG_METHOD_EXIT();

      throw PEGASUS_CIM_EXCEPTION_L(
          CIM_ERR_FAILED,
          MessageLoaderParms(
              "ProviderManager2.JMPI.JMPIImpl.GET_CLASSPATH_FAILED.STANDARD",
              "Could not get CLASSPATH from environment.")
          );

      return -1;
   }

///JNIoptions.append ("-Djava.compiler=NONE");
///maxoption++;

   unsigned int aEnvOptionsSize=sizeof(aEnvOptions)/sizeof(aEnvOptions[0]);
   for (Uint32 i = 0; i < aEnvOptionsSize; i++)
   {
      JVMOPTIONS *pEnvOption = &aEnvOptions[i];

      envstring = getenv(pEnvOption->pszEnvName);
      if (envstring)
      {
         if (pEnvOption->fSplit)
         {
            bool              fCommaFound  = true;
            string            stringValues = envstring;
            string::size_type posStart     = 0;
            string::size_type posEnd       = stringValues.length() - 1;

            while (fCommaFound)
            {
               string            stringValue;
               string::size_type posComma    = stringValues.find(',',
                                                                 posStart);

               if (posComma != string::npos)
               {
                  fCommaFound=true;
                  stringValue=stringValues.substr(posStart, posComma);
                  posStart=posComma + 1;
               }
               else
               {
                  fCommaFound=false;
                  stringValue=stringValues.substr(posStart,posEnd-posStart+1);
               }

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
                   "fCommaFound = %d, posStart = %d, "
                       "posComma =  %d, posEnd = %d",
                   fCommaFound,posStart,posComma,posEnd));

               maxoption++;

               oss.str ("");
               oss << pEnvOption->pszPrefix << stringValue;

               JNIoptions.append (oss.str ());

               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
                   "%s found! Specifying \"%s\"",
                   pEnvOption->pszEnvName, (const char*)oss.str().c_str()));
            }
         }
         else
         {
            maxoption++;

            oss.str ("");
            oss << pEnvOption->pszPrefix << envstring;

            JNIoptions.append (oss.str ());

            PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                "%s found! Specifying \"%s\"",
                pEnvOption->pszEnvName, (const char*)oss.str().c_str()));

         }
      }
   }

   poptions = (JavaVMOption *)calloc (maxoption, sizeof (JavaVMOption));
   if (!poptions)
   {
      jv.initRc = 1;

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL2,
          "Could not allocate %d structures of size %d.",
          maxoption,sizeof (JavaVMOption)));

      PEG_METHOD_EXIT();
      return -1;
   }

   for (Uint32 i = 0; i < JNIoptions.size(); i++)
   {
      poptions[i].optionString = (char *)JNIoptions[i].c_str ();

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL3,
          "Setting option %d to \"%s\".",
          i ,poptions[i].optionString));
   }

   vm_args.version = JNI_VERSION_1_2;
   vm_args.options = poptions;
   vm_args.nOptions = maxoption;
   vm_args.ignoreUnrecognized = JNI_TRUE;

   res = JNI_CreateJavaVM(&jvm,(void**)&env,&vm_args);

   if (poptions)
   {
      free (poptions);
   }

   if (res!= 0)
   {
      jv.initRc = 1;

      PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL3,
          "Can not create Java VM !");

      PEG_METHOD_EXIT();
      return -1;
   }

   cacheIDs(env);

   if (env->ExceptionOccurred())
   {
      jv.initRc = 1;

      env->ExceptionDescribe();

#ifdef JAVA_DESTROY_VM_WORKS

      JvmVector *jv = NULL;

      attachThread (&jv);

      jvm->DestroyJavaVM ();

      jvm = NULL;
#endif

      PEG_METHOD_EXIT();
      return -1;
   }

   jv.initRc = 1;
   jv.jvm = jvm;

   PEG_METHOD_EXIT();
   return res;
}

JNIEnv* JMPIjvm::attachThread (JvmVector **jvp)
{
   JNIEnv* env = NULL;
   int rc;

   if (jvm == NULL)
   {
      rc = initJVM ();

      if ((jvm == NULL) || (rc != 0))
         return NULL;
   }

   jvm->AttachCurrentThread ((void**)&env,NULL);

   *jvp = &jv;

   return env;
}

void JMPIjvm::detachThread ()
{
   jvm->DetachCurrentThread ();
}

jobject JMPIjvm::getProvider (JNIEnv     *env,
                              String      jarName,
                              String      className,
                              const char *pszProviderName,
                              jclass     *pjClass)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIjvm::getProvider");

   jobject   jProviderInstance      = 0;
   jclass    jClassLoaded           = 0;
   jmethodID jId                    = 0;
   jobject   jProviderInstanceLocal = 0;

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
       "jarName = %s, className = %s, pszProviderName = %s, pjClass = %p",
       (const char*)jarName.getCString(),
       (const char*)className.getCString(),
       pszProviderName,pjClass));

   // CASE #1
   //    className has been loaded previously.
   //    Return saved instance.
   _objectTable.lookup (className, jProviderInstance);
   _classTable.lookup (className, jClassLoaded);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
       "jProviderInstance = %p, jClassLoaded = %p",
       jProviderInstance,jClassLoaded));

   if (  jProviderInstance
      && jClassLoaded
      )
   {
      if (pjClass)
      {
         *pjClass = jClassLoaded;
      }
      PEG_METHOD_EXIT();
      return jProviderInstance;
   }

   // CASE #2
   //    className can be loaded via getGlobalClassRef ().
   //    Load and return the instance.
   // NOTE:
   //    According to
   //        http://java.sun.com/j2se/1.5.0/docs/guide/jni/spec/functions.html
   //    In JDK 1.1, FindClass searched only local classes in CLASSPATH.

   jClassLoaded = getGlobalClassRef (env,
                                     (const char*)className.getCString ());

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
       "jClassLoaded = %p",jClassLoaded));

   if (env->ExceptionCheck ())
   {
      // CASE #3
      //    see if the className can be loaded via JarClassLoader.load ().
      //    Load and return the instance.
      jstring jJarName          = 0;
      jstring jClassName        = 0;
      jclass  jClassLoadedLocal = 0;

      env->ExceptionClear ();

      // NOTE: Instances of "packageName/className" will not work with the jar
      //       class loader.  Change the '/' to a '.'.
      String fixedClassName(className);
      static Char16 slash=Char16('/');

      for (Uint32 i=0; i<className.size(); i++)
      {
          if (fixedClassName[i]==slash)
          {
              fixedClassName[i]=Char16('.');
          }
      };

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "fixedClassName = %s",(const char*)fixedClassName.getCString()));

      jJarName = env->NewStringUTF((const char*)jarName.getCString());
      jClassName = env->NewStringUTF((const char*)fixedClassName.getCString());

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "jJarName = %p, jClassName = %p",
          jJarName,jClassName));

      jClassLoadedLocal = (jclass)env->CallStaticObjectMethod(
                              JMPIjvm::jv.JarClassLoaderClassRef,
                              JMPIjvm::jv.JarClassLoaderLoad,
                              jJarName,
                              jClassName);

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "jClassLoadedLocal = %p",jClassLoadedLocal));

      if (env->ExceptionCheck ())
      {
         env->ExceptionDescribe();

         PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
              "Unable to instantiate provider %s: "
              "Can not load Java class %s from jar %s.",
              pszProviderName,
              (const char*)fixedClassName.getCString(),
              (const char*)jarName.getCString()));

         PEG_METHOD_EXIT();
         return 0;
      }

      jClassLoaded = (jclass)env->NewGlobalRef (jClassLoadedLocal);

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
          "jClassLoaded = %p",jClassLoaded));

      env->DeleteLocalRef (jClassLoadedLocal);
   }

   if (pjClass)
   {
      *pjClass = jClassLoaded;
   }

   if (!jClassLoaded)
   {
      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
           "Unable to instantiate provider %s: "
           "Can not load Java class.",pszProviderName));
      PEG_METHOD_EXIT();
      return 0;
   }

   jId = env->GetMethodID (jClassLoaded,"<init>","()V");

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "GetMethodID() jID = %p ",jId));

   jProviderInstanceLocal = env->NewObject (jClassLoaded,
                                            jId);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "NewObject() jProviderInstanceLocal = %p ",
        jProviderInstanceLocal));

   if (!jProviderInstanceLocal)
   {
      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
           "Unable to instantiate provider %s: "
           "No new Java object of provider.",pszProviderName));
      PEG_METHOD_EXIT();
      return 0;
   }

   jProviderInstance = (jobject)env->NewGlobalRef (jProviderInstanceLocal);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "NewGlobalRef() jProviderInstance = %p ",jProviderInstance));

   if (!jProviderInstance)
   {
       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
            "Unable to instantiate provider %s: "
            "No global reference to provider object.",pszProviderName));
      PEG_METHOD_EXIT();
      return 0;
   }

   _classTable.insert (className, jClassLoaded);
   _objectTable.insert (className, jProviderInstance);

   PEG_METHOD_EXIT();
   return jProviderInstance;
}

jobject JMPIjvm::getProvider (JNIEnv *env, const char *cn, jclass *cls)
{
   String cln = cn;
   jobject gProv = NULL;
   jclass scls = NULL;

   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIjvm::getProvider");

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "cn = %s, cls = %p",cn,cls));

   _objectTable.lookup(cln,gProv);
   _classTable.lookup(cln,scls);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "gProv = %p, scls = %p",gProv,scls));

   if (gProv)
   {
      *cls = scls;
      return gProv;
   }

   scls = getGlobalClassRef(env,cn);
   if (env->ExceptionCheck())
   {
      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
           "Provider %s not found: No global reference.",cn));
      PEG_METHOD_EXIT();
      return NULL;
   }
   *cls = scls;

   if (scls)
   {
       PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
            "Inserting global reference %p into class table.",scls));
      _classTable.insert(cln,scls);
   }

   jmethodID id = env->GetMethodID(*cls,"<init>","()V");
   jobject lProv = env->NewObject(*cls,id);
   gProv = (jobject)env->NewGlobalRef(lProv);
   if (env->ExceptionCheck())
   {
      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
           "Unable to instantiate provider %s.",cn));
      PEG_METHOD_EXIT();
      return NULL;
   }

   if (gProv)
   {
      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
           "Inserting provider reference %p into object table.",gProv));
      _objectTable.insert(cln,gProv);
   }

   PEG_METHOD_EXIT();
   return gProv;
}

#if 0

// Java 1.4 version of programmatically accessting the backtrace stack
// NOTE: Renumber to follow current content
// add to:
// const char* classNames[]={
///*37*/ "java/lang/StackTraceElement"
// add to:
// const METHOD_STRUCT instanceMethodNames[]={
///*57 ThrowableGetStackTrace         */ { /*Throwable        */
///21, "getStackTrace",   "()Ljava/lang/StackTraceElement;" },
///*58 StackTraceElementGetClassName  */ { /*StackTraceElement*/
///37, "getClassName",    "()Ljava/lang/String;" },
///*59 StackTraceElementGetFileName   */ { /*StackTraceElement*/
///37, "getFileName",     "()Ljava/lang/String;" },
///*60 StackTraceElementGetLineNumber */ { /*StackTraceElement*/
///37, "getLineNumber",   "()I" },
///*61 StackTraceElementGetMethodName */ { /*StackTraceElement*/
///37, "getMethodName",   "()Ljava/lang/String;" }

String getExceptionInfo (JNIEnv *env)
{
   jthrowable   err        = env->ExceptionOccurred ();
   jobjectArray stackTrace = 0;
   String       rc;

   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"getExceptionInfo");

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "err =  %ld ",(long)(jlong)err));

   if (!err)
   {
       PEG_METHOD_EXIT();
      return rc;
   }


   stackTrace = (jobjectArray)env->CallObjectMethod(
                    err,
                    JMPIjvm::jv.ThrowableGetStackTrace);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "stackTrace =  %p ",stackTrace));

   if (!stackTrace)
   {
       PEG_METHOD_EXIT();
      return rc;
   }

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "stackTrace length =  %d",(jlong)env->GetArrayLength(stackTrace)));

   jobject jFirstST = 0;
   jstring jClass   = 0;
   jstring jFile    = 0;
   jstring jMethod  = 0;
   jint    jLine    = 0;

   jFirstST = env->GetObjectArrayElement (stackTrace, 0);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "jFirstST = %p",jFirstST));

   if (!jFirstST)
   {
       PEG_METHOD_EXIT();
      return rc;
   }

   jClass  = (jstring)env->CallObjectMethod(
                 jFirstST,
                 JMPIjvm::jv.StackTraceElementGetClassName);
   jFile   = (jstring)env->CallObjectMethod(
                 jFirstST,
                 JMPIjvm::jv.StackTraceElementGetFileName);
   jMethod = (jstring)env->CallObjectMethod(
                 jFirstST,
                 JMPIjvm::jv.StackTraceElementGetMethodName);
   jLine   = env->CallIntMethod(
                 jFirstST,
                 JMPIjvm::jv.StackTraceElementGetLineNumber);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
       "jClass = %p, jFile = %p, jMethod = %p, jLine = %p",
       jClass,jFile,jMethod,jLine));

   const char *pszClass  = 0;
   const char *pszFile   = 0;
   const char *pszMethod = 0;

   pszClass  = env->GetStringUTFChars (jClass, NULL);
   pszFile   = env->GetStringUTFChars (jFile, NULL);
   pszMethod = env->GetStringUTFChars (jMethod, NULL);

   std::ostringstream oss;
   String             exceptionInfo;

   if (pszFile)
   {
      oss << "File: " << pszFile;

      env->ReleaseStringUTFChars (jFile, pszFile);
   }
   if (jLine)
   {
      oss << ", Line: " << jLine;
   }
   if (pszClass)
   {
      oss << ", Class: " << pszClass;

      env->ReleaseStringUTFChars (jClass, pszClass);
   }
   if (pszMethod)
   {
      oss << ", Method: " << pszMethod;

      env->ReleaseStringUTFChars (jMethod, pszMethod);
   }

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "oss = %s",(const char*)oss.str().c_str()));

   rc = oss.str ().c_str ();

   return rc;
}

#else

String getExceptionInfo (JNIEnv *env)
{
   jthrowable err = env->ExceptionOccurred ();
   String     rc;

   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"getExceptionInfo");

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
        "err =  %ld ",(long)(jlong)err));

   if (!err)
   {
       PEG_METHOD_EXIT();
      return rc;
   }


   // ByteArrayOutputStream baos = new ByteArrayOutputStream ();
   // PrintStream           ps   = new PrintStream (baos);
   // e.printStackTrace (ps);
   // result = baos.toString ();

   jobject jBAOS = 0;
   jobject jPS   = 0;

   jBAOS = env->NewObject (JMPIjvm::jv.ByteArrayOutputStreamClassRef,
                           JMPIjvm::jv.ByteArrayOutputStreamNew);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4, "jBAOS = %p ",jBAOS));

   if (!jBAOS)
   {
      env->ExceptionDescribe ();

      PEG_METHOD_EXIT();
      return rc;
   }

   jPS = env->NewObject (JMPIjvm::jv.PrintStreamClassRef,
                         JMPIjvm::jv.PrintStreamNewOb,
                         jBAOS);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4, "jPS = %p ",jPS));

   if (!jPS)
   {
       PEG_METHOD_EXIT();
      return rc;
   }

   env->CallVoidMethod (err,
                        JMPIjvm::jv.ThrowablePrintStackTrace,
                        jPS);

   jstring jST = 0;

   jST = (jstring)env->CallObjectMethod(
             jBAOS,
             JMPIjvm::jv.ByteArrayOutputStreamToString);

   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,"jST = %p ",jST));

   const char *pszST = 0;

   pszST = env->GetStringUTFChars (jST, NULL);

   if (pszST)
   {
      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
           "pszST = %s ",pszST));

      rc = pszST;

      env->ReleaseStringUTFChars (jST, pszST);
   }

   PEG_METHOD_EXIT();
   return rc;
}

#endif

void JMPIjvm::checkException (JNIEnv *env)
{
   PEG_METHOD_ENTER(TRC_PROVIDERMANAGER,"JMPIjvm::checkException");

   if (!env->ExceptionCheck ())
   {
       PEG_METHOD_EXIT();
      return;
   }

   jstring     jMsg = NULL,
               jId  = NULL;
   int         code;
   const char *cp;
   String      msg;
   String      id;
   jthrowable  err  = env->ExceptionOccurred ();

//   PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
//        "err =  %ld ",(long)(jlong)err));

   if (!err)
   {
       PEG_METHOD_EXIT();
      return;
   }

   PEG_TRACE_CSTRING(TRC_PROVIDERMANAGER, Tracer::LEVEL1,
        "Provider caused an exception!");

   env->ExceptionDescribe ();

   if (env->IsInstanceOf (err, JMPIjvm::jv.CIMExceptionClassRef))
   {
      env->ExceptionClear ();

      jMsg = (jstring)env->CallObjectMethod(
                 err,
                 JMPIjvm::jv.ThrowableGetMessage);
      code = (int)env->CallIntMethod(
                 err,
                 JMPIjvm::jv.CIMExceptionGetCode);
      jId  = (jstring)env->CallObjectMethod(
                 err,
                 JMPIjvm::jv.CIMExceptionGetID);

      if (jId)
      {
         cp = env->GetStringUTFChars (jId, NULL);
         id = String (cp);
         env->ReleaseStringUTFChars (jId, cp);
      }

      if (jMsg)
      {
         cp  = env->GetStringUTFChars (jMsg, NULL);
         msg = String (cp);
         env->ReleaseStringUTFChars (jMsg, cp);
      }

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
           "throwing Pegasus exception: %d %s (%s)",
           code,(const char*)id.getCString(),(const char*)msg.getCString()));

      PEG_METHOD_EXIT();
      throw CIMException ((CIMStatusCode)code, id+" ("+msg+")");
   }
   else
   {
      String info = getExceptionInfo (env);

      env->ExceptionClear ();

      PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL1,
           "Java caused an exception: %s",(const char*)info.getCString()));

      PEG_METHOD_EXIT();

      throw PEGASUS_CIM_EXCEPTION_L(
          CIM_ERR_FAILED,
          MessageLoaderParms(
              "ProviderManager2.JMPI.JMPIImpl.JAVA_CAUSED_EXCEPTION.STANDARD",
              "Java caused an exception: $0",
              info));
   }
}

/**************************************************************************
 * name         - NewPlatformString
 * description  - Returns a new Java string object for the specified
 *                platform string.
 * parameters   - env
 *                s     Platform encoded string
 * returns      - Java string object pointer or null (0)
 **************************************************************************/
jstring JMPIjvm::NewPlatformString (JNIEnv *env, char *s)
{
    size_t     len = strlen(s);
    jclass     cls;
    jmethodID  mid;
    jbyteArray ary;

    NULL_CHECK0(cls = (*env).FindClass("java/lang/String"));
    NULL_CHECK0(mid = (*env).GetMethodID(cls, "<init>", "([B)V"));

    ary = (*env).NewByteArray((jsize)len);
    if (ary != 0)
    {
        jstring str = 0;
        (*env).SetByteArrayRegion (ary,
                                   0,
                                   (jsize)len,
                                   (jbyte *)s);
        if (!(*env).ExceptionOccurred ())
        {
            str = (jstring)(*env).NewObject (cls, mid, ary);
        }
        (*env).DeleteLocalRef (ary);

        return str;
    }
    return 0;
}

/**************************************************************************
 * name         - NewPlatformStringArray
 * description  - Returns a new array of Java string objects for the specified
 *                array of platform strings.
 * parameters   - env
 *                strv      Platform encoded string array
 *                strc      Number of strings in strv
 * returns      - Java string array object pointer
 **************************************************************************/
jobjectArray JMPIjvm::NewPlatformStringArray(JNIEnv *env, char **strv, int strc)
{
    jarray cls;
    jarray ary;
    int i;

    NULL_CHECK0(cls = (jarray)(*env).FindClass("java/lang/String"));
    NULL_CHECK0(ary = (*env).NewObjectArray(strc, (jclass)cls, 0));
    for(i = 0; i < strc; i++)
    {
        jstring str = NewPlatformString (env, *strv++);

        NULL_CHECK0(str);

        (*env).SetObjectArrayElement ((jobjectArray)ary, i, str);
        (*env).DeleteLocalRef (str);
    }

    return (jobjectArray)ary;
}

CIMPropertyList getList (JNIEnv *jEnv, jobjectArray l)
{
   CIMPropertyList pl;

   if (l)
   {
      Array<CIMName> n;

      for (jsize i = 0, s = jEnv->GetArrayLength (l); i <s; i++)
      {
         jstring     jObj = (jstring)jEnv->GetObjectArrayElement (l, i);
         const char *pn   = jEnv->GetStringUTFChars (jObj, NULL);

         n.append (pn);

         jEnv->ReleaseStringUTFChars (jObj, pn);
      }

      pl.set (n);
   }

   return pl;
}

extern "C" {

void throwCimException (JNIEnv *jEnv, CIMException &e)
{
      JMPIjvm::cacheIDs(jEnv);
      jobject ev = jEnv->NewObject(
          JMPIjvm::jv.CIMExceptionClassRef,
          JMPIjvm::jv.CIMExceptionNewI,
          (jint)e.getCode());
      jEnv->Throw((jthrowable)ev);
}

void throwFailedException (JNIEnv *jEnv)
{
      JMPIjvm::cacheIDs(jEnv);
      jobject ev = jEnv->NewObject(
          JMPIjvm::jv.CIMExceptionClassRef,
          JMPIjvm::jv.CIMExceptionNewI,
          1);
      jEnv->Throw((jthrowable)ev);
}

void throwNotSupportedException (JNIEnv *jEnv)
{
      JMPIjvm::cacheIDs(jEnv);
      jobject ev = jEnv->NewObject(
          JMPIjvm::jv.CIMExceptionClassRef,
          JMPIjvm::jv.CIMExceptionNewI,
          7);
      jEnv->Throw((jthrowable)ev);
}


// -------------------------------------
// ---
// -        CIMArgument
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMArgument__1new
      (JNIEnv *jEnv, jobject jThs)
{
   CIMParamValue *p = new CIMParamValue (String::EMPTY,CIMValue ());

   return DEBUG_ConvertCToJava (CIMParamValue*, jlong, p);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMArgument__1newS
      (JNIEnv *jEnv, jobject jThs, jstring jN)
{
   const char    *str = jEnv->GetStringUTFChars (jN, NULL);
   CIMParamValue *p   = new CIMParamValue (str, CIMValue ());

   jEnv->ReleaseStringUTFChars (jN, str);

   return DEBUG_ConvertCToJava (CIMParamValue*, jlong, p);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMArgument__1newSV
      (JNIEnv *jEnv, jobject jThs, jstring jN, jlong jV)
{
   const char    *str = jEnv->GetStringUTFChars (jN, NULL);
   CIMValue      *cv  = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);
   CIMParamValue *p   = 0;

   if (  str
      && cv
      )
   {
      p = new CIMParamValue (str, *cv);
   }

   jEnv->ReleaseStringUTFChars (jN, str);

   return DEBUG_ConvertCToJava (CIMParamValue*, jlong, p);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMArgument__1getType
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMParamValue *cp       = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jP);
   const CIMValue cv       = cp->getValue ();
   String         ref;
   bool           fSuccess = false;
   int            iJType   = 0;
   _dataType     *type     = 0;

   iJType = _dataType::convertCTypeToJavaType (cv.getType (), &fSuccess);

   if (fSuccess)
   {
      type = new _dataType (iJType,
                            cv.getArraySize (),
                            false,
                            false,
                            cv.isArray (),
                            ref,
                            true);
   }

   return DEBUG_ConvertCToJava (_dataType*, jlong, type);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMArgument__1setValue
      (JNIEnv *jEnv, jobject jThs, jlong jP,jlong jV)
{
   CIMParamValue *cp = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jP);
   CIMValue      *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);

   cp->setValue (*cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMArgument__1getValue
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMParamValue *cp = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jP);
   CIMValue      *cv = new CIMValue (cp->getValue ());

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMArgument__1getName
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMParamValue *cp  = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jP);
   const String  &n   = cp->getParameterName ();
   jstring        str = jEnv->NewStringUTF (n.getCString ());

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMArgument__1setName
      (JNIEnv *jEnv, jobject jThs, jlong jP, jstring jN)
{
   CIMParamValue *cp  = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jP);
   const char    *str = jEnv->GetStringUTFChars (jN, NULL);
   String         n (str);

   cp->setParameterName (n);

   jEnv->ReleaseStringUTFChars (jN, str);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMArgument__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMParamValue *cp = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jP);

   delete cp;

   DEBUG_ConvertCleanup (jlong, jP);
}


// -------------------------------------
// ---
// -        CIMClass
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClass__1newInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCls)
{
   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);

   if (cls)
   {
      try
      {
         CIMInstance   *ci     = new CIMInstance (cls->getClassName ());
         CIMObjectPath  copNew = ci->getPath ();

         copNew.setNameSpace (cls->getPath ().getNameSpace ());
         ci->setPath (copNew);
         for (int i = 0, m = cls->getQualifierCount (); i < m; i++)
         {
            try
            {
               ci->addQualifier (cls->getQualifier (i).clone ());
            }
            catch(Exception e)
            {
            }
         }
         for (int i = 0, m = cls->getPropertyCount (); i < m; i++)
         {
            CIMProperty cp = cls->getProperty (i);

            ci->addProperty (cp.clone ());

            for (int j = 0, s = cp.getQualifierCount (); j < s; j++)
            {
               try
               {
                  ci->getProperty (i).addQualifier (cp.getQualifier (j));
               }
               catch (Exception e)
               {
               }
            }
         }
         return DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
      }
      Catch (jEnv);
   }

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClass__1filterProperties
      (JNIEnv *jEnv,
       jobject jThs,
       jlong jInst,
       jobjectArray jPl,
       jboolean iq,
       jboolean ic,
       jboolean lo)
{
   CIMClass *cc   = DEBUG_ConvertJavaToC (jlong, CIMClass*, jInst);
   CIMClass *cf   = 0;
   CIMName   clsn = cc->getClassName ();

   if (lo)
   {
      cf = new CIMClass (cc->clone ());

      CIMName clsn = cc->getClassName ();

      for (int i = cf->getPropertyCount () - 1; i>= 0; i--)
      {
         if (cf->getProperty (i).getClassOrigin () == clsn)
            cf->removeProperty (i);
      }

      return DEBUG_ConvertCToJava (CIMClass*, jlong, cf);
   }
   else if (jPl)
   {
      CIMPropertyList pl = getList (jEnv,jPl);
      Array<CIMName>  n  = pl.getPropertyNameArray ();

      cf = new CIMClass (clsn, cc->getSuperClassName ());

      for (int i = 0, s = n.size (); i < s; i++)
      {
         Uint32 pos = cc->findProperty (n[i]);

         if (pos != PEG_NOT_FOUND)
         {
            if (iq)
            {
               CIMProperty cp = cc->getProperty (pos).clone ();

               if (!ic)
                  cp.setClassOrigin (CIMName ());

               cf->addProperty (cp);
            }
            else
            {
               CIMProperty cp = cc->getProperty (pos);
               CIMName     co;

               if (ic)
                  co = cp.getClassOrigin ();

               CIMProperty np (cp.getName (),
                              cp.getValue (),
                              cp.getArraySize (),
                              cp.getReferenceClassName (),
                              co,
                              cp.getPropagated ());

               cf->addProperty (np);
            }
         }
      }
      if (iq)
      {
         for (Uint32 i = 0, s = cc->getQualifierCount (); i < s; i++)
         {
            cf->addQualifier (cc->getQualifier (i));
         }
      }
   }
   else if (iq)
   {
      cf = new CIMClass (cc->clone ());

      if (ic)
         return DEBUG_ConvertCToJava (CIMClass*, jlong, cf);

      for (int i = cf->getPropertyCount () - 1; i >= 0; i--)
      {
         CIMProperty cp = cf->getProperty (i);

         cp.setClassOrigin (CIMName ());
         cf->removeProperty (i);
         cf->addProperty (cp);
      }
   }
   else
   {
      cf = new CIMClass (clsn, cc->getSuperClassName ());

      for (int i = cc->getPropertyCount () - 1; i >= 0; i--)
      {
         CIMProperty cp = cc->getProperty (i);
         CIMName     co;

         if (ic)
            co = cp.getClassOrigin ();

         CIMProperty np (cp.getName (),
                        cp.getValue (),
                        cp.getArraySize (),
                        cp.getReferenceClassName (),
                        co,
                        cp.getPropagated ());

         cf->addProperty (np);
      }
   }

   return DEBUG_ConvertCToJava (CIMClass*, jlong, cf);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMClass__1getName
   (JNIEnv *jEnv, jobject jThs, jlong jCls)
{
   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);

   try {
      const String &cn = cls->getClassName ().getString ();
      jstring str = jEnv->NewStringUTF (cn.getCString ());

      return str;
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClass__1getQualifier
   (JNIEnv *jEnv, jobject jThs, jlong jCls, jstring jN)
{
   CIMClass   *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   const char *str = jEnv->GetStringUTFChars (jN, NULL);
   jlong       rv  = 0;
   Uint32      pos = cls->findQualifier (String (str));

   if (pos != PEG_NOT_FOUND)
      rv = DEBUG_ConvertCToJava (CIMQualifier*,
                                 jlong,
                                 new CIMQualifier (cls->getQualifier (pos)));

   jEnv->ReleaseStringUTFChars (jN, str);

   return rv;
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMClass__1getQualifiers
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);

   for (int i = 0, s = cls->getQualifierCount (); i < s; i++)
   {
      CIMQualifier *cq   = new CIMQualifier (cls->getQualifier (i));
      jlong         jCq  = DEBUG_ConvertCToJava (CIMQualifier*, jlong, cq);
      jobject       qual = jEnv->NewObject(
                               JMPIjvm::jv.CIMQualifierClassRef,
                               JMPIjvm::jv.CIMQualifierNewJ,
                               jCq);

      jEnv->CallVoidMethod (jVec,JMPIjvm::jv.VectorAddElement,qual);
   }

   return jVec;
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMClass__1hasQualifier
     (JNIEnv *jEnv, jobject jThs, jlong jCls, jstring jQ)
{
   CIMClass   *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   const char *str = jEnv->GetStringUTFChars (jQ,NULL);
   Uint32      pos = cls->findQualifier (String (str));

   jEnv->ReleaseStringUTFChars (jQ,str);

   return (jboolean)(pos != PEG_NOT_FOUND);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClass__1addProperty
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jlong jP)
{
   CIMClass    *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   CIMProperty *p   = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);

   try
   {
      cls->addProperty (*p);
   }
   Catch (jEnv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClass__1setProperties
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);

   for (int i = cls->getPropertyCount () - 1; i >= 0; i--)
   {
      cls->removeProperty (i);
   }
   for (Uint32 i = 0, s = jEnv->CallIntMethod(
                              jVec,
                              JMPIjvm::jv.VectorSize); i < s; i++)
   {
      JMPIjvm::checkException (jEnv);

      jobject o = jEnv->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,(i));
      jlong        jp = jEnv->CallLongMethod(o,JMPIjvm::jv.CIMPropertyCInst);
      CIMProperty *cp = DEBUG_ConvertJavaToC(jlong, CIMProperty*, jp);

      JMPIjvm::checkException (jEnv);

      cls->addProperty (*cp);
   }
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClass__1getProperty
     (JNIEnv *jEnv, jobject jThs, jlong jCls, jstring jN)
{
   CIMClass   *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   const char *str = jEnv->GetStringUTFChars (jN,NULL);
   jlong       rv  = 0;
   Uint32      pos = cls->findProperty (CIMName (str));

   if (pos != PEG_NOT_FOUND)
      rv = DEBUG_ConvertCToJava (CIMProperty*,
                                 jlong,
                                 new CIMProperty (cls->getProperty (pos)));

   jEnv->ReleaseStringUTFChars (jN,str);

   return rv;
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMClass__1getProperties
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);

   for (int i = 0, s = cls->getPropertyCount (); i < s; i++)
   {
      CIMProperty *cp   = new CIMProperty (cls->getProperty (i));
      jlong        jCp  = DEBUG_ConvertCToJava (CIMProperty*, jlong, cp);
      jobject      prop = jEnv->NewObject(
                              JMPIjvm::jv.CIMPropertyClassRef,
                              JMPIjvm::jv.CIMPropertyNewJ,
                              jCp);

      jEnv->CallVoidMethod (jVec,JMPIjvm::jv.VectorAddElement,prop);
   }

   return jVec;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClass__1new
      (JNIEnv *jEnv, jobject jThs, jstring jN)
{
   const char *str = jEnv->GetStringUTFChars (jN,NULL);
   CIMClass   *cls = new CIMClass (CIMName (str), CIMName ());

   jEnv->ReleaseStringUTFChars (jN,str);

   return DEBUG_ConvertCToJava (CIMClass*, jlong, cls);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMClass__1getSuperClass
      (JNIEnv *jEnv, jobject jThs, jlong jCls)
{
   CIMClass     *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   const String &cn  = cls->getSuperClassName ().getString ();

   jstring str = jEnv->NewStringUTF (cn.getCString ());

   return str;
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMClass__1getKeys
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);

   if (cls->hasKeys ())
   {
      Array<CIMName> keyNames;

      cls->getKeyNames (keyNames);

      for (int i = 0, s = keyNames.size (); i < s; i++)
      {
         Uint32 pos = cls->findProperty (keyNames[i]);

         if (pos != PEG_NOT_FOUND)
         {
            CIMProperty *cp  = new CIMProperty (cls->getProperty (pos));
            jlong        jCp = DEBUG_ConvertCToJava (CIMProperty*, jlong, cp);

            jobject prop = jEnv->NewObject(
                               JMPIjvm::jv.CIMPropertyClassRef,
                               JMPIjvm::jv.CIMPropertyNewJ,
                               jCp);

            jEnv->CallVoidMethod (jVec,JMPIjvm::jv.VectorAddElement,prop);
         }
      }
   }

   return jVec;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClass__1getMethod
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jstring jN)
{
   CIMClass   *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   const char *str = jEnv->GetStringUTFChars (jN,NULL);
   jlong       rv  = 0;
   Uint32      pos = cls->findMethod (String (str));

   if (pos != PEG_NOT_FOUND)
   {
      rv = DEBUG_ConvertCToJava(
               CIMMethod*,
               jlong,
               new CIMMethod(cls->getMethod(pos)));
   }

   jEnv->ReleaseStringUTFChars (jN,str);

   return rv;
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMClass__1equals
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jlong jClsToBeCompared)
{
   CIMClass *cls = DEBUG_ConvertJavaToC(jlong, CIMClass*, jCls);
   CIMClass *clsToBeCompared = DEBUG_ConvertJavaToC(
                                   jlong,
                                   CIMClass*,
                                   jClsToBeCompared);
   bool      fRc             = false;

   if (  cls && clsToBeCompared )
   {
      try
      {
         fRc = cls->identical (*clsToBeCompared);
      }
      Catch (jEnv);
   }

   return fRc;
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMClass__1isAssociation
      (JNIEnv *jEnv, jobject jThs, jlong jCls)
{
   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   jboolean  rv  = false;

   if (cls)
   {
      try
      {
         rv = cls->isAssociation ();
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMClass__1findMethod
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jstring jName)
{
   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   jint      rv  = -1;

   if (cls)
   {
      const char *cstrName = jEnv->GetStringUTFChars (jName, NULL);

      try
      {
         CIMName name (cstrName);

         rv = cls->findMethod (name);
      }
      Catch (jEnv);

      jEnv->ReleaseStringUTFChars (jName, cstrName);
   }

   return rv;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClass__1getMethodI
      (JNIEnv *jEnv, jobject jThs, jlong jCls, jint jMethod)
{
   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   jlong     rv  = 0;

   if (cls && jMethod >=0)
   {
      try
      {
         CIMMethod cm = cls->getMethod (jMethod);

         rv = DEBUG_ConvertCToJava (CIMMethod *, jlong, new CIMMethod (cm));
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMClass__1getMethodCount
      (JNIEnv *jEnv, jobject jThs, jlong jCls)
{
   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);
   jint      rv  = 0;

   if (cls)
   {
      try
      {
         rv = cls->getMethodCount ();
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClass__1finalize
     (JNIEnv *jEnv, jobject jThs, jlong jCls)
{
   CIMClass *cls = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCls);

   delete cls;

   DEBUG_ConvertCleanup (jlong, jCls);
}


// -------------------------------------
// ---
// -        CIMClient
// ---
// -------------------------------------

void checkNs (CIMObjectPath *cop, jlong jNs)
{
   if (cop->getNameSpace ().isNull ())
   {
      _nameSpace *cNs = DEBUG_ConvertJavaToC (jlong, _nameSpace*, jNs);

      cop->setNameSpace (CIMNamespaceName (cNs->nameSpace ()));
   }
}

static int normalizeNs (String &ns, String &nsBase, String &lastNsComp)
{
   Uint32 n = ns.size ();

   if (ns[n-1] == '/')
   {
      if (n >= 2)
         ns = ns.subString (0, n-2);
   }

   lastNsComp = ns;
   nsBase     = "root";

   n = ns.reverseFind ('/');

   if (n != PEG_NOT_FOUND)
   {
      lastNsComp = ns.subString (n+1);
      nsBase     = ns.subString (0, n);
   }

   return 0;
}

Boolean verifyServerCertificate (SSLCertificateInfo &certInfo)
{
   //
   // If server certificate was found in CA trust store and validated, then
   // return 'true' to accept the certificate, otherwise return 'false'.
   //
   if (certInfo.getResponseCode () == 1)
   {
       return true;
   }
   else
   {
       return false;
   }
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1newNaUnPw
  (JNIEnv *jEnv, jobject jThs, jlong jNs, jstring jUn, jstring jPw)
{
   _nameSpace *cNs = DEBUG_ConvertJavaToC (jlong, _nameSpace*, jNs);
   const char *un  = jEnv->GetStringUTFChars (jUn, NULL);
   const char *pw  = jEnv->GetStringUTFChars (jPw, NULL);
   jlong       jCc = 0;

   SSLContext *sslContext = 0; // initialized for unencrypted connection

#ifdef PEGASUS_HAS_SSL
   if (cNs->isHttps ())
   {
      try
      {
         sslContext = new SSLContext (PEGASUS_SSLCLIENT_CERTIFICATEFILE,
                                      verifyServerCertificate,
                                      PEGASUS_SSLCLIENT_RANDOMFILE);
      }
      catch (Exception &e)
      {
        cerr << "JMPI: Error: could not create SSLContext: "
             << e.getMessage() << endl;
        return jCc;
      }
   }
#endif

   try {
      CIMClient *cc = new CIMClient ();

      if (sslContext)
      {
         cc->connect (cNs->hostName (), cNs->port (), *sslContext, un, pw);
      }
      else
      {
         cc->connect (cNs->hostName (), cNs->port (), un, pw);
      }

      jCc = DEBUG_ConvertCToJava (CIMClient*, jlong, cc);
   }
   Catch (jEnv);

   jEnv->ReleaseStringUTFChars (jUn, un);
   jEnv->ReleaseStringUTFChars (jPw, pw);

   delete sslContext;

   return jCc;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1disconnect
  (JNIEnv *jEnv, jobject jThs, jlong jCc)
{
   CIMClient *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);

   try {
      cCc->disconnect ();
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1getClass
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jboolean lo,
         jboolean iq, jboolean ic, jobjectArray jPl)
{
   CIMClient       *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath   *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMPropertyList  pl = getList (jEnv, jPl);

   if (  cCc
      && cop
      )
   {
      try {
         checkNs (cop, jNs);

         CIMClass      cls    = cCc->getClass (cop->getNameSpace (),
                                               cop->getClassName (),
                                               (Boolean)lo,
                                               (Boolean)iq,
                                               (Boolean)ic,
                                               pl);
         CIMObjectPath copNew = cls.getPath ();

         copNew.setNameSpace (cop->getNameSpace ());
         cls.setPath (copNew);

         return DEBUG_ConvertCToJava (CIMClass*, jlong, new CIMClass (cls));
      }
      Catch (jEnv);
   }

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1deleteClass
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   try {
      checkNs (cop, jNs);

      cCc->deleteClass (cop->getNameSpace (),
                        cop->getClassName ());
   }
   Catch (jEnv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1createClass
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jlong jCl)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMClass      *cl  = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCl);

   try {
      cCc->createClass (cop->getNameSpace (), *cl);
   }
   Catch (jEnv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1setClass
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jlong jCl)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMClass      *cl  = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCl);

   try {
     checkNs (cop, jNs);

     cCc->modifyClass (cop->getNameSpace (), *cl);
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1getInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jboolean lo,
         jboolean iq, jboolean ic, jobjectArray jPl)
{
   CIMClient        *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMPropertyList   pl  = getList (jEnv, jPl);

   try {
      checkNs (cop, jNs);

      CIMInstance inst = cCc->getInstance (cop->getNameSpace (),
                                           *cop,
                                           (Boolean)lo,
                                           (Boolean)iq,
                                           (Boolean)ic,
                                           pl);

      inst.setPath (*cop);

      return DEBUG_ConvertCToJava (CIMInstance*, jlong, new CIMInstance (inst));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1deleteInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   try {
      checkNs (cop, jNs);

      cCc->deleteInstance (cop->getNameSpace (), *cop);
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1createInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jlong jCi)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMInstance   *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jCi);

   try {
      checkNs (cop, jNs);

      ci->setPath (*cop);

      CIMObjectPath obj = cCc->createInstance (cop->getNameSpace (), *ci);

      return DEBUG_ConvertCToJava(CIMObjectPath*,jlong,new CIMObjectPath (obj));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1modifyInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jlong jCi,
         jboolean iq, jobjectArray jPl)
{
   CIMClient       *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath   *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMInstance     *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jCi);
   CIMPropertyList  pl  = getList (jEnv, jPl);

   try {
      checkNs (cop, jNs);

      ci->setPath (*cop);

      cCc->modifyInstance (cop->getNameSpace (), *ci, (Boolean)iq, pl);
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1enumerateClasses(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jNs,
    jlong jCop,
    jboolean deep,
    jboolean lo,
    jboolean iq,
    jboolean ic)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   try {
      checkNs (cop, jNs);

      Array<CIMClass> enm = cCc->enumerateClasses (cop->getNameSpace (),
                                                   cop->getClassName (),
                                                   (Boolean)deep,
                                                   (Boolean)lo,
                                                   (Boolean)iq,
                                                   (Boolean)ic);

      return DEBUG_ConvertCToJava(
                 Array<CIMClass>*,
                 jlong,
                 new Array<CIMClass> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1enumerateClassNames
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jboolean deep)
{
   CIMClient        *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMNamespaceName  ns  = cop->getNameSpace ();

   try {
      checkNs (cop, jNs);

      Array<CIMName> enm = cCc->enumerateClassNames(ns,
                                                    cop->getClassName(),
                                                    (Boolean)deep);
      Array<CIMObjectPath> *enmop = new Array<CIMObjectPath> ();

      for (int i = 0, m = enm.size (); i < m; i++)
      {
         enmop->append (CIMObjectPath (String::EMPTY, ns, enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMObjectPath>*, jlong, enmop);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1enumerateInstanceNames
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jboolean deep)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   try {
      checkNs (cop,jNs);

      Array<CIMObjectPath> enm = cCc->enumerateInstanceNames(
                                     cop->getNameSpace(),
                                     cop->getClassName ()); //, (Boolean)deep);

      return DEBUG_ConvertCToJava(
                 Array<CIMObjectPath>*,
                 jlong,
                 new Array<CIMObjectPath> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1enumerateInstances(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jNs,
    jlong jCop,
    jboolean deep,
    jboolean lo,
    jboolean iq,
    jboolean ic,
    jobjectArray jPl)
{
   CIMClient       *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath   *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMPropertyList  pl  = getList (jEnv,jPl);

   try {
      checkNs (cop, jNs);

      CIMPropertyList    props;
      Array<CIMInstance> enm = cCc->enumerateInstances (cop->getNameSpace (),
                                                        cop->getClassName (),
                                                        (Boolean)deep,
                                                        (Boolean)lo,
                                                        (Boolean)iq,
                                                        (Boolean)ic,pl);

      return DEBUG_ConvertCToJava(
                 Array<CIMInstance>*,
                 jlong,
                 new Array<CIMInstance> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1enumerateQualifiers
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   try {
      checkNs (cop, jNs);

      Array<CIMQualifierDecl> enm=cCc->enumerateQualifiers(cop->getNameSpace());

      return DEBUG_ConvertCToJava(
                 Array<CIMQualifierDecl>*,
                 jlong,
                 new Array<CIMQualifierDecl> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1getQualifier
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   try {
      checkNs (cop,jNs);

      CIMQualifierDecl *val = new CIMQualifierDecl(
                                      cCc->getQualifier(
                                          cop->getNameSpace (),
                                          cop->getClassName ()));

      return DEBUG_ConvertCToJava (CIMQualifierDecl*, jlong, val);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1setQualifier
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jlong jQ)
{
   CIMClient        *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMQualifierDecl *qt  = DEBUG_ConvertJavaToC (jlong, CIMQualifierDecl*, jQ);

   try {
      checkNs (cop, jNs);

      cCc->setQualifier (cop->getNameSpace (), *qt);
   }
   Catch (jEnv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1deleteQualifier
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   try {
      checkNs (cop, jNs);

      cCc->deleteQualifier (cop->getNameSpace (), cop->getClassName ());
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1getProperty
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jstring jPn)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char    *str = jEnv->GetStringUTFChars (jPn, NULL);
   CIMName        pName (str);
   jlong          jCv = 0;

   try {
      checkNs (cop, jNs);

      CIMValue *val = new CIMValue(cCc->getProperty(
                                            cop->getNameSpace(),
                                            *cop,
                                            pName));

      jCv = DEBUG_ConvertCToJava (CIMValue*, jlong, val);
   }
   Catch (jEnv);

   jEnv->ReleaseStringUTFChars (jPn, str);

   return jCv;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1setProperty(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jNs,
    jlong jCop,
    jstring jPn,
    jlong jV)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMValue      *val = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);
   const char    *str = jEnv->GetStringUTFChars (jPn, NULL);
   CIMName        pName (str);

   try {
      checkNs (cop,jNs);

      cCc->setProperty (cop->getNameSpace (), *cop, pName, *val);
   }
   Catch (jEnv);

   jEnv->ReleaseStringUTFChars (jPn, str);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1execQuery(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jNs,
    jlong jCop,
    jstring jQuery,
    jstring jQl)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char    *str = jEnv->GetStringUTFChars (jQuery, NULL);
   String         query (str);

   jEnv->ReleaseStringUTFChars (jQuery, str);

   str = jEnv->GetStringUTFChars (jQl, NULL);

   String ql (str);

   jEnv->ReleaseStringUTFChars (jQl, str);

   try {
      checkNs (cop, jNs);

      Array<CIMObject>    enm     = cCc->execQuery (cop->getNameSpace (),
                                                    ql,
                                                    query);
      Array<CIMInstance> *enmInst = new Array<CIMInstance> ();

      for (int i = 0,m = enm.size (); i<m; i++)
      {
         enmInst->append (CIMInstance (enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMInstance>*, jlong, enmInst);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1invokeMethod(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jNs,
    jlong jCop,
    jstring jMn,
    jobject jIn,
    jobject jOut)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMClient *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC(jlong, CIMObjectPath*, jCop);
   const char *str = jEnv->GetStringUTFChars (jMn, NULL);
   CIMName method (str);
   jlong jCv = 0;
   Array<CIMParamValue> in;
   Array<CIMParamValue> out;

   for (int i = 0,m = jEnv->CallIntMethod(jIn,JMPIjvm::jv.VectorSize); i<m; i++)
   {
       JMPIjvm::checkException (jEnv);

       jobject jProp=jEnv->CallObjectMethod(jIn,JMPIjvm::jv.VectorElementAt,i);

       JMPIjvm::checkException (jEnv);

       jlong jp = jEnv->CallLongMethod(jProp,JMPIjvm::jv.CIMPropertyCInst);
       CIMProperty *p  = DEBUG_ConvertJavaToC(jlong, CIMProperty*, jp);

       JMPIjvm::checkException (jEnv);

       in.append (CIMParamValue (p->getName ().getString (),p->getValue ()));
   }

   try {
      checkNs (cop, jNs);

      CIMValue *val = new CIMValue(
                              cCc->invokeMethod(
                                       cop->getNameSpace(),
                                       *cop,
                                       method,
                                       in,
                                       out));

      for (int i = 0,m = out.size (); i<m; i++)
      {
         const CIMParamValue &parm = out[i];
         const CIMValue v = parm.getValue ();
         CIMProperty *p = new CIMProperty(
                                  parm.getParameterName(),
                                  v,
                                  v.getArraySize());
         jlong jp = DEBUG_ConvertCToJava (CIMProperty*, jlong, p);
         jobject prop = jEnv->NewObject(
                            JMPIjvm::jv.CIMPropertyClassRef,
                            JMPIjvm::jv.CIMPropertyNewJ,
                            jp);

         jEnv->CallVoidMethod (jOut,JMPIjvm::jv.VectorAddElement,prop);
      }

      jCv = DEBUG_ConvertCToJava (CIMValue*, jlong, val);
   }
   Catch (jEnv);

   jEnv->ReleaseStringUTFChars (jMn,str);

   return jCv;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1invokeMethod24
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop, jstring jMn,
        jobjectArray jIn, jobjectArray jOut)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMClient *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char *str = jEnv->GetStringUTFChars (jMn, NULL);
   CIMName method (str);
   jlong jCv = 0;
   Array<CIMParamValue> in;
   Array<CIMParamValue> out;

   if (jIn)
   {
      for (int i = 0, m = jEnv->GetArrayLength (jIn); i < m; i++)
      {
          JMPIjvm::checkException (jEnv);

          jobject jArg = jEnv->GetObjectArrayElement (jIn, i);

          JMPIjvm::checkException (jEnv);

          jlong jp = jEnv->CallLongMethod (jArg,JMPIjvm::jv.CIMArgumentCInst);
          CIMParamValue *p = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jp);

          JMPIjvm::checkException (jEnv);

          in.append (*p);
      }
   }
   try {
      checkNs (cop,jNs);

      CIMValue *val = new CIMValue (cCc->invokeMethod (cop->getNameSpace (),
                                                       *cop,
                                                       method,
                                                       in,
                                                       out));

      if (jOut)
      {
         for (int i=0,m=out.size(),o=jEnv->GetArrayLength(jOut);i<m && i<o;i++)
         {
            CIMParamValue *parm  = new CIMParamValue (out[i]);
            jlong jParm = DEBUG_ConvertCToJava (CIMParamValue*, jlong, parm);

            jEnv->SetObjectArrayElement(
                jOut,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.CIMArgumentClassRef,
                    JMPIjvm::jv.CIMArgumentNewJ,
                    jParm));
         }
      }
      jCv = DEBUG_ConvertCToJava (CIMValue*, jlong, val);
   }
   Catch (jEnv);

   jEnv->ReleaseStringUTFChars (jMn, str);

   return jCv;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1associatorNames(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jNs,
    jlong jCop,
    jstring jAssocClass,
    jstring jResultClass,
    jstring jRole,
    jstring jResultRole)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char    *str = jEnv->GetStringUTFChars (jAssocClass, NULL);

   CIMName assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject(
                                JMPIjvm::jv.CIMExceptionClassRef,
                                JMPIjvm::jv.CIMExceptionNewISt,
                                (jint)4, // CIM_ERR_INVALID_PARAMETER
                                jEnv->NewStringUTF(
                                    "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass,str);

   str = jEnv->GetStringUTFChars (jResultClass, NULL);

   CIMName resultClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         resultClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid result class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jResultClass, str);

   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   str = jEnv->GetStringUTFChars (jResultRole, NULL);

   String resultRole (str);

   jEnv->ReleaseStringUTFChars (jResultRole, str);

   try {
      checkNs (cop,jNs);

      Array<CIMObjectPath> enm = cCc->associatorNames (cop->getNameSpace (),
                                                      *cop,
                                                      assocClass,
                                                      resultClass,
                                                      role,
                                                      resultRole);
      return DEBUG_ConvertCToJava(
          Array<CIMObjectPath>*,
          jlong,
          new Array<CIMObjectPath> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1associators(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jNs,
    jlong jCop,
    jstring jAssocClass,
    jstring jResultClass,
    jstring jRole,
    jstring jResultRole,
    jboolean includeQualifiers,
    jboolean includeClassOrigin,
    jobjectArray jPl)
{
   CIMClient       *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath   *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMPropertyList  pl  = getList (jEnv, jPl);
   const char      *str = jEnv->GetStringUTFChars (jAssocClass, NULL);

   CIMName assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject(JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass, str);

   str = jEnv->GetStringUTFChars (jResultClass, NULL);

   CIMName resultClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         resultClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid result class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jResultClass, str);

   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   str = jEnv->GetStringUTFChars (jResultRole, NULL);

   String resultRole (str);

   jEnv->ReleaseStringUTFChars (jResultRole, str);

   try {
      checkNs (cop, jNs);

      Array<CIMObject> enm = cCc->associators (cop->getNameSpace (),
                                               *cop,
                                               assocClass,
                                               resultClass,
                                               role,
                                               resultRole,
                                               (Boolean)includeQualifiers,
                                               (Boolean)includeClassOrigin,
                                               pl);
      Array<CIMInstance> *enmInst = new Array<CIMInstance> ();

      for (int i = 0,m = enm.size (); i<m; i++)
      {
         enmInst->append (CIMInstance (enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMInstance>*, jlong, enmInst);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1referenceNames
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop,
   jstring jAssocClass, jstring jRole)
{
   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char    *str = jEnv->GetStringUTFChars (jAssocClass, NULL);

   CIMName assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass, str);

   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   try {
      checkNs (cop, jNs);

      Array<CIMObjectPath> enm = cCc->referenceNames (cop->getNameSpace (),
                                                      *cop,
                                                      assocClass,
                                                      role);

      return DEBUG_ConvertCToJava(
          Array<CIMObjectPath>*,
          jlong,
          new Array<CIMObjectPath> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMClient__1references
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jlong jNs, jlong jCop,
   jstring jAssocClass, jstring jRole,
   jboolean includeQualifiers, jboolean includeClassOrigin, jobjectArray jPl)
{
   CIMClient       *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath   *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMPropertyList  pl  = getList (jEnv, jPl);
   const char      *str = jEnv->GetStringUTFChars (jAssocClass, NULL);

   CIMName assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass, str);
   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   try {
      checkNs (cop, jNs);

      Array<CIMObject> enm = cCc->references (cop->getNameSpace (),
                                              *cop,
                                              assocClass,
                                              role,
                                              (Boolean)includeQualifiers,
                                              (Boolean)includeClassOrigin,
                                              pl);
      Array<CIMInstance> *enmInst = new Array<CIMInstance> ();

      for (int i = 0, m = enm.size (); i < m; i++)
      {
         enmInst->append (CIMInstance (enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMInstance>*, jlong, enmInst);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1createNameSpace
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jstring jNs)
{
   CIMClient  *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   const char *str = jEnv->GetStringUTFChars (jNs, NULL);
   String      ns (str);

   jEnv->ReleaseStringUTFChars (jNs, str);

   String lastNsComp;
   String nsBase;

   normalizeNs (ns, nsBase, lastNsComp);

   CIMInstance newInstance (CIMName ("__Namespace"));
   newInstance.addProperty(CIMProperty(PEGASUS_PROPERTYNAME_NAME, lastNsComp));

   try {
      cCc->createInstance (CIMNamespaceName (nsBase), newInstance);
   }
   Catch (jEnv);
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMClient__1enumerateNameSpaces(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCc,
    jlong jCop,
    jboolean deep,
    jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMClient     *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   String         ns  = cop->getNameSpace ().getString ();

   try {
      Array<CIMObjectPath> enm = cCc->enumerateInstanceNames(
                                          cop->getNameSpace(),
                                          CIMName("__Namespace"));

      for (int i = 0, s = enm.size (); i < s; i++)
      {
         CIMObjectPath&              cop = enm[i];
         const Array<CIMKeyBinding>& kb  = cop.getKeyBindings ();
         const String&               n   = kb[0].getValue ();

         if (!deep && n.find ('/') != PEG_NOT_FOUND)
            continue;

         String  x   = ns+"/"+n;
         jstring str = jEnv->NewStringUTF (x.getCString ());

         jEnv->CallVoidMethod (jVec, JMPIjvm::jv.VectorAddElement, str);
      }
   }
   Catch (jEnv);

   return jVec;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1deleteNameSpace
  (JNIEnv *jEnv, jobject jThs, jlong jCc, jstring jNs)
{
   CIMClient  *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);
   const char *str = jEnv->GetStringUTFChars (jNs,NULL);
   String      ns (str);

   jEnv->ReleaseStringUTFChars (jNs,str);

   String lastNsComp;
   String nsBase;

   normalizeNs (ns,nsBase,lastNsComp);

   CIMObjectPath        cop (String::EMPTY,
                             CIMNamespaceName (nsBase),
                             CIMName ("__Namespace"));
   Array<CIMKeyBinding> kb;

   kb.append (CIMKeyBinding (PEGASUS_PROPERTYNAME_NAME,CIMValue (lastNsComp)));
   cop.setKeyBindings (kb);

   try {
      cCc->deleteInstance (CIMNamespaceName (nsBase), cop);
   }
   Catch (jEnv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMClient__1finalize
   (JNIEnv *jEnv, jobject jThs, jlong jCc)
{
   CIMClient  *cCc = DEBUG_ConvertJavaToC (jlong, CIMClient*, jCc);

   delete cCc;

   DEBUG_ConvertCleanup (jlong, jCc);
}


// -------------------------------------
// ---
// -        CIMDataType
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMDataType__1new
  (JNIEnv *jEnv, jobject jThs, jint type)
{
  return DEBUG_ConvertCToJava (_dataType*, jlong, new _dataType (type));
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMDataType__1newAr
  (JNIEnv *jEnv, jobject jThs, jint type, jint size)
{
  return DEBUG_ConvertCToJava (_dataType*, jlong, new _dataType (type,size));
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMDataType__1newRef
  (JNIEnv *jEnv, jobject jThs, jint type, jstring jRef)
{
   const char *ref   = jEnv->GetStringUTFChars (jRef,NULL);
   jlong cInst = DEBUG_ConvertCToJava(
                     _dataType*,
                     jlong,
                     new _dataType(type,String (ref)));

   jEnv->ReleaseStringUTFChars (jRef,ref);

   return cInst;
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMDataType__1isReference
  (JNIEnv *jEnv, jobject jThs, jlong jDt)
{
   _dataType *dt = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);

   return dt->_reference == true;
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMDataType__1isArray
  (JNIEnv *jEnv, jobject jThs, jlong jDt)
{
   _dataType *dt = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);

   return dt->_array == true;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMDataType__1getType
  (JNIEnv *jEnv, jobject jThs, jlong jDt)
{
   _dataType *dt = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);

   return dt->_type;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMDataType__1getSize
  (JNIEnv *jEnv, jobject jThs, jlong jDt)
{
   _dataType *dt = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);

   return dt->_size;
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMDataType__1getRefClassName
  (JNIEnv *jEnv, jobject jThs, jlong jDt)
{
   _dataType *dt  = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);
   jstring    str = jEnv->NewStringUTF (dt->_refClass.getCString ());

   return str;
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMDataType__1toString
  (JNIEnv *jEnv, jobject jThs, jlong jDt)
{
   _dataType *dt  = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);
   jstring    str = NULL;

   if (dt->_type & 0x10)
   {
      bool   fSuccess = false;
      String tmp = _dataType::convertJavaTypeToChars(dt->_type-0x10, &fSuccess);

      if (!fSuccess)
         return str;

      tmp = tmp + "[]";

      str = jEnv->NewStringUTF (tmp.getCString ());
   }
   else if (dt->_type == 0x20 + 1) // REFERENCE
   {
      String tmp = dt->_refClass + " REF";

      str = jEnv->NewStringUTF (tmp.getCString ());
   }
   else
   {
      bool  fSuccess = false;
      char *tmp      = _dataType::convertJavaTypeToChars (dt->_type, &fSuccess);

      if (!fSuccess)
         return str;

      str = jEnv->NewStringUTF (tmp);
   }

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMDataType__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jDt)
{
   _dataType *dt = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);

   delete dt;

   DEBUG_ConvertCleanup (jlong, jDt);
}


// -------------------------------------
// ---
// -        CIMDateTime
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMDateTime__1datetime
      (JNIEnv *jEnv, jobject jThs, jstring jN)
{
   const char  *str  = jEnv->GetStringUTFChars (jN, NULL);
   CIMDateTime *dt   = 0;
   String       date;

   if (  str
      && *str
      )
   {
      date = str;
   }

   jEnv->ReleaseStringUTFChars (jN, str);

   try
   {
      dt = new CIMDateTime (date);
   }
   Catch (jEnv);

   return DEBUG_ConvertCToJava (CIMDateTime*, jlong, dt);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMDateTime__1datetimeempty
      (JNIEnv *jEnv, jobject jThs)
{
   CIMDateTime *dt = new CIMDateTime (CIMDateTime::getCurrentDateTime ());

   return DEBUG_ConvertCToJava (CIMDateTime*, jlong, dt);
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMDateTime__1after
      (JNIEnv *jEnv, jobject jThs, jlong jC, jlong jD)
{
   CIMDateTime *ct = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jC);
   CIMDateTime *dt = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jD);
   jboolean     rv  = 0;

   if (  ct
      && dt
      )
   {
      rv = (jboolean)(ct->getDifference (*ct, *dt) < 0);
   }

   return rv;
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMDateTime__1before
      (JNIEnv *jEnv, jobject jThs, jlong jC, jlong jD)
{
   CIMDateTime *ct  = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jC);
   CIMDateTime *dt  = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jD);
   jboolean     rv  = 0;

   if (  ct
      && dt
      )
   {
      rv = (jboolean)(ct->getDifference (*ct, *dt) > 0);
   }

   return rv;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMDateTime__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jDT)
{
   CIMDateTime *cdt = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jDT);

   delete cdt;

   DEBUG_ConvertCleanup (jlong, jDT);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMDateTime__1getCIMString
      (JNIEnv *jEnv, jobject jThs, jlong jDT)
{
   CIMDateTime *cdt  = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jDT);
   jstring      jRet = 0;

   if (cdt)
   {
      String dateString = cdt->toString ();

      if (dateString.size () > 0)
      {
         jRet = jEnv->NewStringUTF (dateString.getCString ());
      }
   }

   return jRet;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMDateTime__1getMicroseconds
      (JNIEnv *jEnv, jobject jThs, jlong jDT)
{
   CIMDateTime *cdt  = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jDT);
   jlong        jRet = 0;

   if (cdt)
   {
      // Convert from 1 BCE epoch to POSIX 1970 microseconds
      jRet = cdt->toMicroSeconds () - PEGASUS_UINT64_LITERAL(62167219200000000);
   }

   return jRet;
}


// -------------------------------------
// ---
// -        CIMInstance
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1new
      (JNIEnv *jEnv, jobject jThs)
{
   return DEBUG_ConvertCToJava (CIMInstance*, jlong, new CIMInstance ());
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1newCn
      (JNIEnv *jEnv, jobject jThs, jstring jN)
{
   const char *str = jEnv->GetStringUTFChars (jN,NULL);
   CIMInstance *ci = new CIMInstance (CIMName (str));

   jEnv->ReleaseStringUTFChars (jN,str);

   return DEBUG_ConvertCToJava (CIMInstance*, jlong, ci);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1filterProperties(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jInst,
    jobjectArray jPl,
    jboolean iq,
    jboolean ic,
    jboolean lo)
{
   CIMInstance *ci   = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   CIMInstance *cf   = 0;
   CIMName      clsn = ci->getClassName ();

   if (lo)
   {
      cf = new CIMInstance (ci->clone ());

      CIMName clsn = ci->getClassName ();

      for (int i = cf->getPropertyCount () - 1; i >= 0; i--)
      {
         if (cf->getProperty (i).getClassOrigin () == clsn)
            cf->removeProperty (i);
      }

      return DEBUG_ConvertCToJava (CIMInstance*, jlong, cf);
   }
   else if (jPl)
   {
      CIMPropertyList pl = getList (jEnv, jPl);
      Array<CIMName>  n  = pl.getPropertyNameArray ();

      cf = new CIMInstance (clsn);

      for (int i = 0, s = n.size (); i < s; i++)
      {
         Uint32 pos = ci->findProperty (n[i]);

         if (pos != PEG_NOT_FOUND)
         {
            if (iq)
            {
               CIMProperty cp = ci->getProperty (pos).clone ();

               if (!ic)
                  cp.setClassOrigin (CIMName ());

               cf->addProperty (cp);
            }
            else
            {
               CIMProperty cp = ci->getProperty (pos);
               CIMName     co;

               if (ic)
                  co = cp.getClassOrigin ();

               CIMProperty np (cp.getName (),
                               cp.getValue (),
                               cp.getArraySize (),
                               cp.getReferenceClassName (),
                               co,
                               cp.getPropagated ());

               cf->addProperty (np);
            }
         }
      }

      cf->setPath (ci->getPath ());
   }
   else if (iq)
   {
      cf = new CIMInstance (ci->clone ());

      if (ic)
         return DEBUG_ConvertCToJava (CIMInstance*, jlong, cf);

      for (int i = cf->getPropertyCount () - 1; i >= 0; i--)
      {
         CIMProperty cp = cf->getProperty (i);

         cp.setClassOrigin (CIMName ());
         cf->removeProperty (i);
         cf->addProperty (cp);
      }
   }
   else
   {
      cf = new CIMInstance (clsn);

      for (int i = ci->getPropertyCount () - 1; i >= 0; i--)
      {
         CIMProperty cp = ci->getProperty (i);
         CIMName     co;

         if (ic)
            co = cp.getClassOrigin ();

         CIMProperty np (cp.getName (),
                         cp.getValue (),
                         cp.getArraySize (),
                         cp.getReferenceClassName (),
                         co,
                         cp.getPropagated ());

         cf->addProperty (np);
      }
      cf->setPath (ci->getPath ());
   }

   return DEBUG_ConvertCToJava (CIMInstance*, jlong, cf);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMInstance__1setName
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jstring jN)
{
   CIMInstance *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   const char  *str = jEnv->GetStringUTFChars (jN, NULL);

   jEnv->ReleaseStringUTFChars (jN,str);

   /* NOT SUPPORTED AND NOT NEEDED*/
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMInstance__1setProperty
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jstring jN, jlong jV)
{
   CIMInstance *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   CIMValue    *cv  = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);
   const char  *str = jEnv->GetStringUTFChars (jN, NULL);
   Uint32       pos;

   if (  ci
      && cv
      )
   {
      try {
         pos = ci->findProperty (CIMName (str));

         if (pos != PEG_NOT_FOUND)
         {
            CIMProperty cp = ci->getProperty (pos);

            if (cp.getType () == cv->getType ())
            {
               cp.setValue (*cv);
            }
            else
            {
               throw CIMException(
                         CIM_ERR_TYPE_MISMATCH,
                         String ("Property type mismatch"));
            }

            ci->removeProperty (pos);
            ci->addProperty (cp);
         }
         else
         {
            CIMProperty *cp;

            if (cv->getType() != CIMTYPE_REFERENCE)
            {
               cp = new CIMProperty (CIMName (str), *cv);
            }
            else
            {
               if (!cv->isArray ())
               {
                  CIMObjectPath cop;

                  cv->get (cop);
                  cp = new CIMProperty(CIMName(str),*cv,0,cop.getClassName());
               }
               else
               {
                  throwCIMException(
                      jEnv,
                      "+++ unsupported type in CIMProperty.property");
               }
            }

            ci->addProperty (*cp);
         }
      }
      Catch (jEnv);
   }

   jEnv->ReleaseStringUTFChars (jN, str);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMInstance__1setProperties
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jobject jV)
{
   CIMInstance *ci = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);

   if (!ci)
   {
      return;
   }

   try
   {
      for (int i = 0, m = jEnv->CallIntMethod (jV,
                                               JMPIjvm::jv.VectorSize);
           i < m;
           i++)
      {
         JMPIjvm::checkException (jEnv);

         jobject jProp = jEnv->CallObjectMethod (jV,
                                                 JMPIjvm::jv.VectorElementAt,
                                                 i);

         JMPIjvm::checkException (jEnv);

         jlong jCpRef = jEnv->CallLongMethod(
                            jProp,
                            JMPIjvm::jv.CIMPropertyCInst);
         CIMProperty *cpNew = DEBUG_ConvertJavaToC(
                                  jlong,
                                  CIMProperty*,
                                  jCpRef);

         if (cpNew)
         {
            Uint32 pos = ci->findProperty (cpNew->getName ());

            if (pos != PEG_NOT_FOUND)
            {
               CIMProperty cpOld = ci->getProperty (pos);

               if (cpOld.getType () == cpNew->getType ())
               {
                  ci->removeProperty (pos);
                  ci->addProperty (*cpNew);
               }
               else
               {
                  throw CIMException(
                      CIM_ERR_TYPE_MISMATCH,
                      String ("Property type mismatch"));
               }
            }
         }
      }
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1getProperty
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jstring jN)
{
   CIMInstance *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   const char  *str = jEnv->GetStringUTFChars (jN, NULL);
   jlong        rv  = 0;

   try {
      Uint32 pos = ci->findProperty (CIMName (str));

      if (pos != PEG_NOT_FOUND)
      {
         CIMProperty *cp = new CIMProperty (ci->getProperty (pos));

         rv = DEBUG_ConvertCToJava (CIMProperty*, jlong, cp);
      }
   }
   Catch (jEnv);

   jEnv->ReleaseStringUTFChars (jN, str);

   return rv;
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMInstance__1getKeyValuePairs
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMInstance *ci = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);

   if (  ci
      && CIMNamespaceName::legal (ci->getPath ().getNameSpace ().getString ())
      )
   {
      CIMOMHandle      ch;
      OperationContext oc;
      CIMClass         cc;

      cc = ch.getClass (oc,
                        ci->getPath ().getNameSpace (),
                        ci->getClassName (),
                        false,              // localOnly
                        true,               // includeQualifiers
                        true,               // includeClassOrigin
                        CIMPropertyList ());// propertyList

      if (!cc.hasKeys ())
      {
         return jVec;
      }

      Array<CIMName> keyNames;

      cc.getKeyNames (keyNames);

      for (Uint32 i = 0; i < keyNames.size (); i++)
      {
         PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
              "finding key %s ",
              (const char*)keyNames[i].getString().getCString()));

         for (Uint32 j = 0; j < ci->getPropertyCount (); j++)
         {
            CIMProperty cp = ci->getProperty (j);

            if (cp.getName () == keyNames[i])
            {
               PEG_TRACE((TRC_PROVIDERMANAGER, Tracer::LEVEL4,
                    "adding key (%d) %s ",i,
                    (const char*)keyNames[i].getString().getCString()));

               CIMProperty *cpRef  = new CIMProperty (cp);
               jlong jCpRef = DEBUG_ConvertCToJava (CIMProperty*, jlong, cpRef);
               jobject jProp  = jEnv->NewObject(
                                         JMPIjvm::jv.CIMPropertyClassRef,
                                         JMPIjvm::jv.CIMPropertyNewJ,
                                         jCpRef);

               jEnv->CallVoidMethod (jVec,
                                     JMPIjvm::jv.VectorAddElement,
                                     jProp);
            }
         }
      }
   }

   return jVec;
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMInstance__1getProperties
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMInstance *ci = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);

   for (int i = 0, s = ci->getPropertyCount (); i < s; i++)
   {
      CIMProperty *cp  = new CIMProperty (ci->getProperty (i));
      jlong        jCp = DEBUG_ConvertCToJava (CIMProperty*, jlong, cp);

      jobject prop = jEnv->NewObject(
                         JMPIjvm::jv.CIMPropertyClassRef,
                         JMPIjvm::jv.CIMPropertyNewJ,
                         jCp);

      jEnv->CallVoidMethod(jVec,JMPIjvm::jv.VectorAddElement,prop);
   }

   return jVec;
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMInstance__1getClassName(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jInst)
{
   CIMInstance  *ci = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   const String &cn = ci->getClassName ().getString ();

   jstring str = jEnv->NewStringUTF (cn.getCString ());

   return str;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1getQualifier
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jstring jN)
{
   CIMInstance *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   const char  *str = jEnv->GetStringUTFChars (jN, NULL);
   jlong        rv  = 0;
   Uint32       pos;

   if (ci)
   {
      try
      {
         CIMOMHandle      ch;
         OperationContext oc;
         CIMClass         cc;

         cc = ch.getClass (oc,
                           ci->getPath ().getNameSpace (),
                           ci->getClassName (),
                           false,               // localOnly
                           true,                // includeQualifiers
                           true,                // includeClassOrigin
                           CIMPropertyList ()); // propertyList

         pos = cc.findQualifier (String (str));

         if (pos != PEG_NOT_FOUND)
         {
            CIMQualifier *cq = 0;

            cq = new CIMQualifier (cc.getQualifier (pos));

            rv = DEBUG_ConvertCToJava (CIMQualifier*, jlong, cq);
         }
      }
      Catch (jEnv);
   }

   jEnv->ReleaseStringUTFChars (jN, str);

   return rv;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1clone
      (JNIEnv *jEnv, jobject jThs, jlong jInst)
{
   CIMInstance *ci = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   CIMInstance *cl = new CIMInstance (ci->clone ());

   return DEBUG_ConvertCToJava (CIMInstance*, jlong, cl);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMInstance__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jInst)
{
   CIMInstance *ci = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);

   delete ci;

   DEBUG_ConvertCleanup (jlong, jInst);
}

/*
 * Class:     org_pegasus_jmpi_CIMInstance
 * Method:    _getObjectPath
 * Signature: (I)I
 */
JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1getObjectPath
  (JNIEnv *jEnv, jobject jThs, jlong jciCi)
{
   CIMInstance   *ci     = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciCi);
   CIMObjectPath *copRet = NULL;

   try
   {
      if (ci)
      {
         const CIMObjectPath& cop = ci->getPath ();

         copRet = new CIMObjectPath (cop);
      }
   }
   Catch (jEnv);

   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, copRet);
}

/*
 * Class:     org_pegasus_jmpi_CIMInstance
 * Method:    _setObjectPath
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMInstance__1setObjectPath
  (JNIEnv *jEnv, jobject jThs, jlong jciCi, jlong jciCop)
{
   CIMInstance   *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciCi);
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jciCop);

   try
   {
      if (  ci
         && cop
         )
      {
         ci->setPath (*cop);
      }
   }
   Catch (jEnv);
}

/*
 * Class:     org_pegasus_jmpi_CIMInstance
 * Method:    _getPropertyCount
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMInstance__1getPropertyCount
  (JNIEnv *jEnv, jobject jThs, jlong jciCi)
{
   CIMInstance *ci      = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciCi);
   Uint32       ui32Ret = 0;

   try
   {
      if (ci)
      {
         ui32Ret = ci->getPropertyCount ();
      }
   }
   Catch (jEnv);

   return ui32Ret;
}

/*
 * Class:     org_pegasus_jmpi_CIMInstance
 * Method:    _getProperty
 * Signature: (II)I
 */
JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMInstance__1getPropertyI
  (JNIEnv *jEnv, jobject jThs, jlong jciCi, jint ji)
{
   CIMInstance *ci    = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jciCi);
   CIMProperty *cpRet = NULL;

   try
   {
      if (ci)
      {
         CIMProperty  cp;

         cp = ci->getProperty (ji);

         cpRet = new CIMProperty (cp);
      }
   }
   Catch (jEnv);

   return DEBUG_ConvertCToJava (CIMProperty*, jlong, cpRet);
}


// -------------------------------------
// ---
// -        CIMMethod
// ---
// -------------------------------------

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMMethod__1getType
      (JNIEnv *jEnv, jobject jThs, jlong jM)
{
   CIMMethod *cm       = DEBUG_ConvertJavaToC (jlong, CIMMethod*, jM);
   bool       fSuccess = false;
   jint       jType    = 0;

   if (cm)
   {
      CIMType ct = cm->getType ();

      jType = _dataType::convertCTypeToJavaType (ct, &fSuccess);
   }

   return jType;
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMMethod__1getName
      (JNIEnv *jEnv, jobject jThs, jlong jM)
{
   CIMMethod *cm  = DEBUG_ConvertJavaToC (jlong, CIMMethod*, jM);
   jstring    str = 0;

   if (cm)
   {
      CIMName cn = cm->getName ();

      str = jEnv->NewStringUTF (cn.getString ().getCString ());
   }

   return str;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMMethod__1findParameter
      (JNIEnv *jEnv, jobject jThs, jlong jM, jstring jName)
{
   CIMMethod *cm  = DEBUG_ConvertJavaToC (jlong, CIMMethod*, jM);
   jint       rv  = -1;

   if (cm && jName)
   {
      const char *cstrName = jEnv->GetStringUTFChars (jName, NULL);

      try
      {
         CIMName name (cstrName);

         rv = cm->findParameter (name);
      }
      Catch (jEnv);

      jEnv->ReleaseStringUTFChars (jName, cstrName);
   }

   return rv;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMMethod__1getParameter
      (JNIEnv *jEnv, jobject jThs, jlong jM, jint jParameter)
{
   CIMMethod *cm  = DEBUG_ConvertJavaToC (jlong, CIMMethod*, jM);
   jlong      rv  = 0;

   if (cm && jParameter >= 0)
   {
      try
      {
         CIMParameter cp = cm->getParameter (jParameter);

         rv = DEBUG_ConvertCToJava(CIMParameter *, jlong, new CIMParameter(cp));
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMMethod__1getParameterCount
      (JNIEnv *jEnv, jobject jThs, jlong jM)
{
   CIMMethod *cm  = DEBUG_ConvertJavaToC (jlong, CIMMethod*, jM);
   jint       rv  = 0;

   if (cm)
   {
      try
      {
         rv = cm->getParameterCount ();
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMMethod__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jM)
{
   CIMMethod *cm = DEBUG_ConvertJavaToC (jlong, CIMMethod*, jM);

   delete cm;

   DEBUG_ConvertCleanup (jlong, jM);
}


// -------------------------------------
// ---
// -        CIMNameSpace
// ---
// -------------------------------------

_nameSpace::_nameSpace ()
{
   port_ = 0;
   hostName_ = System::getHostName ();
   nameSpace_ = "root/cimv2";
   fHttps = false;
}

_nameSpace::_nameSpace (String hn)
{
   port_ = 0;
   hostName_ = hn;
   nameSpace_ = "root/cimv2";
   fHttps = false;
}

_nameSpace::_nameSpace (String hn, String ns)
{
    port_ = 0;
    hostName_ = hn;
    nameSpace_ = ns;
    fHttps = false;
}

int _nameSpace::port ()
{
   if (port_)
      return port_;

   port_ = 5988;

   if (hostName_.subString (0,7) == "http://")
   {
      protocol_ = hostName_.subString (0,7);
      hostName_ = hostName_.subString (7);
   }
   else if (hostName_.subString (0,8) == "https://")
   {
      protocol_ = hostName_.subString (0,8);
      hostName_ = hostName_.subString (8);
      fHttps = true;
   }

   Sint32 p = hostName_.reverseFind (':');

   if (p >= 0)
   {
      if ((hostName_[p+1] >= '0') && (hostName_[p+1] <= '9'))
         port_ = atoi (hostName_.subString (p+1).getCString ());

      hostName_.remove (p);
   }

   return port_;
}

String _nameSpace::hostName ()
{
   port ();
   return hostName_;
}

String _nameSpace::nameSpace ()
{
   return nameSpace_;
}

Boolean _nameSpace::isHttps ()
{
   port ();
   return fHttps;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1new
  (JNIEnv *jEnv, jobject jThs)
{
  return DEBUG_ConvertCToJava (_nameSpace*, jlong, new _nameSpace ());
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1newHn
  (JNIEnv *jEnv, jobject jThs, jstring jHn)
{
   const char *hn    = jEnv->GetStringUTFChars (jHn, NULL);
   jlong cInst = DEBUG_ConvertCToJava(_nameSpace*, jlong, new _nameSpace(hn));

   jEnv->ReleaseStringUTFChars (jHn, hn);

   return cInst;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1newHnNs
  (JNIEnv *jEnv, jobject jThs, jstring jHn, jstring jNs)
{
   const char *hn    = jEnv->GetStringUTFChars (jHn, NULL);
   const char *ns    = jEnv->GetStringUTFChars (jNs, NULL);
   jlong cInst = DEBUG_ConvertCToJava(
                     _nameSpace*,
                     jlong,
                     new _nameSpace(String (hn),String (ns)));

   jEnv->ReleaseStringUTFChars (jHn, hn);
   jEnv->ReleaseStringUTFChars (jNs, ns);

   return cInst;
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1getNameSpace
  (JNIEnv *jEnv, jobject jThs, jlong jNs)
{
   _nameSpace   *cNs = DEBUG_ConvertJavaToC (jlong, _nameSpace*, jNs);
   const String &ns  = cNs->nameSpace_;
   jstring       str = jEnv->NewStringUTF (ns.getCString ());

   return str;
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1getHost
  (JNIEnv *jEnv, jobject jThs, jlong jNs)
{
   _nameSpace   *cNs = DEBUG_ConvertJavaToC (jlong, _nameSpace*, jNs);
   const String &hn  = cNs->hostName_;
   jstring       str = jEnv->NewStringUTF (hn.getCString ());

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1setNameSpace
  (JNIEnv *jEnv, jobject jThs, jlong jNs, jstring jN)
{
   _nameSpace *cNs = DEBUG_ConvertJavaToC (jlong, _nameSpace*, jNs);
   const char *str = jEnv->GetStringUTFChars (jN, NULL);

   cNs->nameSpace_ = str;

   jEnv->ReleaseStringUTFChars (jN, str);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1setHost
  (JNIEnv *jEnv, jobject jThs, jlong jNs, jstring jHn)
{
   _nameSpace *cNs = DEBUG_ConvertJavaToC (jlong, _nameSpace*, jNs);
   const char *str = jEnv->GetStringUTFChars (jHn, NULL);

   cNs->port_ = 0;
   cNs->hostName_ = str;

   jEnv->ReleaseStringUTFChars (jHn, str);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMNameSpace__1finalize
   (JNIEnv *jEnv, jobject jThs, jlong jNs)
{
   _nameSpace *cNs = DEBUG_ConvertJavaToC (jlong, _nameSpace*, jNs);

   delete cNs;

   DEBUG_ConvertCleanup (jlong, jNs);
}


// -------------------------------------
// ---
// -        CIMObject
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObject__1newClass
  (JNIEnv *jEnv, jobject jThs, jlong jCc)
{
   CIMClass *cCc = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCc);

   try {
      CIMObject *cCo = new CIMObject (*cCc);

      return DEBUG_ConvertCToJava (CIMObject*, jlong, cCo);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObject__1newInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCi)
{
   CIMInstance *cCi = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jCi);

   try {
      CIMObject *cCo = new CIMObject (*cCi);

      return DEBUG_ConvertCToJava (CIMObject*, jlong, cCo);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMObject__1finalize
  (JNIEnv *jEnv, jobject jThs, jlong jInst)
{
   CIMObject *co = DEBUG_ConvertJavaToC (jlong, CIMObject*, jInst);

   delete co;

   DEBUG_ConvertCleanup (jlong, jInst);
}


// -------------------------------------
// ---
// -        CIMObjectPath
// ---
// -------------------------------------

CIMObjectPath* construct ()
{
   CIMObjectPath *cop = new CIMObjectPath ();
   _nameSpace     n;

   cop->setNameSpace (n.nameSpace ());
   cop->setHost (n.hostName ());

   return cop;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1new
      (JNIEnv *jEnv, jobject jThs)
{
   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, construct ());
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1newCn
      (JNIEnv *jEnv, jobject jThs, jstring jCn)
{
   CIMObjectPath *cop = construct ();
   const char    *str = jEnv->GetStringUTFChars (jCn, NULL);

   if (str)
      cop->setClassName (str);

   jEnv->ReleaseStringUTFChars (jCn, str);

   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1newCnNs
      (JNIEnv *jEnv, jobject jThs, jstring jCn, jstring jNs)
{
   CIMObjectPath *cop  = construct ();
   const char    *str1 = NULL;
   const char    *str2 = NULL;

   try {
      if (jCn)
         str1 = jEnv->GetStringUTFChars (jCn, NULL);
      if (jNs)
         str2 = jEnv->GetStringUTFChars (jNs, NULL);
      if (str1)
         cop->setClassName (str1);
      if (str2)
         cop->setNameSpace (str2);
   }
   Catch (jEnv);

   if (str1)
      jEnv->ReleaseStringUTFChars (jCn, str1);
   if (str2)
      jEnv->ReleaseStringUTFChars (jNs, str2);

   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1newCi
      (JNIEnv *jEnv, jobject jThs, jlong jInst)
{
   CIMInstance   *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   CIMObjectPath *cop = 0;
   _nameSpace     n;

   try
   {
      if (ci)
      {
         cop = new CIMObjectPath (ci->getPath ());

         if (cop)
         {
            if (cop->getNameSpace ().isNull ())
               cop->setNameSpace (n.nameSpace ());

            if (cop->getHost ().size () == 0)
               cop->setHost (n.hostName ());
         }
      }
   }
   Catch (jEnv);

   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1newCiNs
      (JNIEnv *jEnv, jobject jThs, jlong jInst, jstring jNamespace)
{
   CIMInstance      *ci = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInst);
   CIMObjectPath    *cop = 0;
   const char       *pszNamespace = jEnv->GetStringUTFChars (jNamespace, NULL);
   CIMNamespaceName  cnnNamespace;
   bool              fNamespaceValid = false;
   _nameSpace        n;

   try
   {
      cnnNamespace    = pszNamespace;
      fNamespaceValid = true;
   }
   catch (Exception e)
   {
   }

   jEnv->ReleaseStringUTFChars (jNamespace, pszNamespace);

   try
   {
      if (ci)
      {
         cop = new CIMObjectPath (ci->getPath ());

         if (cop)
         {
            if (fNamespaceValid)
            {
               cop->setNameSpace (cnnNamespace);
            }
            else
            {
               if (cop->getNameSpace ().isNull ())
                  cop->setNameSpace (n.nameSpace ());
            }

            if (cop->getHost ().size () == 0)
               cop->setHost (n.hostName ());
         }
      }
   }
   Catch (jEnv);

   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jCop)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);

   delete cop;

   DEBUG_ConvertCleanup (jlong, jCop);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1getNameSpace
      (JNIEnv *jEnv, jobject jThs, jlong jCop)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const String  &ns  = cop->getNameSpace ().getString ();

   jstring str = jEnv->NewStringUTF (ns.getCString ());

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1setNameSpace
      (JNIEnv *jEnv, jobject jThs, jlong jCop, jstring jName)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char    *str = jEnv->GetStringUTFChars (jName,NULL);

   cop->setNameSpace (CIMNamespaceName (str));

   jEnv->ReleaseStringUTFChars (jName,str);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1getHost
      (JNIEnv *jEnv, jobject jThs, jlong jCop)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const String  &hn = cop->getHost ();

   jstring str = jEnv->NewStringUTF (hn.getCString ());

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1setHost
      (JNIEnv *jEnv, jobject jThs, jlong jCop, jstring jName)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char    *str = jEnv->GetStringUTFChars (jName,NULL);

   cop->setHost (String (str));

   jEnv->ReleaseStringUTFChars (jName,str);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1getObjectName
      (JNIEnv *jEnv, jobject jThs, jlong jCop)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const String  &cn  = cop->getClassName ().getString ();

   jstring str = jEnv->NewStringUTF (cn.getCString ());

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1setObjectName
      (JNIEnv *jEnv, jobject jThs, jlong jCop, jstring jName)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const char    *str = jEnv->GetStringUTFChars (jName,NULL);

   cop->setClassName (String (str));

   jEnv->ReleaseStringUTFChars (jName,str);
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1getKeys
      (JNIEnv *jEnv, jobject jThs, jlong jCop, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const Array<CIMKeyBinding> &akb = cop->getKeyBindings ();

   for (Uint32 i = 0, s = akb.size (); i < s; i++)
   {
      const String        &n  = akb[i].getName ().getString ();
      const String        &v  = akb[i].getValue ();
      CIMKeyBinding::Type  t  = akb[i].getType ();
      CIMValue            *cv = 0;

      switch (t)
      {
      case CIMKeyBinding::NUMERIC:
         cv = new CIMValue ((Sint32)atol (v.getCString ()));
         break;
      case CIMKeyBinding::STRING:
         cv = new CIMValue (v);
         break;
      case CIMKeyBinding::BOOLEAN:
         cv = new CIMValue ((Boolean) (v.getCString ()));
         break;
      case CIMKeyBinding::REFERENCE:
         cv = new CIMValue (CIMObjectPath (akb[i].getValue ()));
         break;
      default:
         throwCIMException (jEnv,"+++ unsupported type: ");
      }

      CIMProperty *cp = 0;

      if (t != CIMKeyBinding::REFERENCE)
         cp = new CIMProperty (n, *cv);
      else
         cp = new CIMProperty(
                      n,
                      *cv,
                      0,
                      ((CIMObjectPath) akb[i].getValue()).getClassName());

      jlong   jCp  = DEBUG_ConvertCToJava (CIMProperty*, jlong, cp);
      jobject prop = jEnv->NewObject(
                         JMPIjvm::jv.CIMPropertyClassRef,
                         JMPIjvm::jv.CIMPropertyNewJ,
                         jCp);

      jEnv->CallVoidMethod (jVec,JMPIjvm::jv.VectorAddElement,prop);
   }

   return jVec;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1setKeys
      (JNIEnv *jEnv, jobject jThs, jlong jCop, jobject jVec)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMObjectPath *cop = DEBUG_ConvertJavaToC(jlong, CIMObjectPath*, jCop);
   Array<CIMKeyBinding>  akb;

   for (Uint32 i=0,s=jEnv->CallIntMethod(jVec,JMPIjvm::jv.VectorSize);i<s;i++)
   {
      jobject o = jEnv->CallObjectMethod(jVec,JMPIjvm::jv.VectorElementAt,(i));
      jlong        jCp = jEnv->CallLongMethod (o,JMPIjvm::jv.CIMPropertyCInst);
      CIMProperty *cp  = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jCp);

      akb.append (CIMKeyBinding (cp->getName (), cp->getValue ()));
   }

   cop->setKeyBindings (akb);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1addKey
      (JNIEnv *jEnv, jobject jThs, jlong jCop, jstring jId, jlong jVal)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC(jlong, CIMObjectPath*, jCop);
   const char *str = jEnv->GetStringUTFChars (jId, NULL);
   CIMValue *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jVal);
   Array<CIMKeyBinding> keyBindings = cop->getKeyBindings ();

   keyBindings.append (CIMKeyBinding (str,*cv));
   cop->setKeyBindings (Array<CIMKeyBinding> (keyBindings));

   jEnv->ReleaseStringUTFChars (jId, str);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1getKeyValue
      (JNIEnv *jEnv, jobject jThs, jlong jCop, jstring jStr)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const Array<CIMKeyBinding> &akb = cop->getKeyBindings ();
   const char *strKeyName = jEnv->GetStringUTFChars (jStr,NULL);
   jstring                     retStr     = NULL;

   for (Uint32 i = 0,s = akb.size (); i<s; i++)
   {
      const String &n = akb[i].getName ().getString ();

      if (n == String (strKeyName))
      {
         retStr = jEnv->NewStringUTF (akb[i].getValue ().getCString ());
         break;
      }
   }

   jEnv->ReleaseStringUTFChars (jStr,strKeyName);

   return retStr;
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1toString
      (JNIEnv *jEnv, jobject jThs, jlong jCop)
{
   CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   const String  &ns  = cop->toString ();

   jstring str = jEnv->NewStringUTF (ns.getCString ());

   return str;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1clone
      (JNIEnv *jEnv, jobject jThs, jlong jCop)
{
   CIMObjectPath *cop  = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMObjectPath *copl = new CIMObjectPath(
                                 cop->getHost(),
                                 cop->getNameSpace(),
                                 cop->getClassName(),
                                 cop->getKeyBindings());

   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, copl);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMObjectPath__1set
      (JNIEnv *jEnv, jobject jThs, jstring jStr)
{
   const char    *strCop = jEnv->GetStringUTFChars (jStr,NULL);
   CIMObjectPath *cop    = new CIMObjectPath ();

   cop->set (String (strCop));

   jEnv->ReleaseStringUTFChars (jStr,strCop);

   return DEBUG_ConvertCToJava (CIMObjectPath*, jlong, cop);
}


// -------------------------------------
// ---
// -        CIMOMHandle
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1getClass
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jboolean lo,
         jboolean iq, jboolean ic, jobjectArray jPl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMPropertyList   pl;
   OperationContext  ctx;

   if (jPl)
      pl = getList (jEnv, jPl);
   else
      pl = CIMPropertyList ();

   try {
      CIMClass cls = ch->getClass (ctx,
                                   cop->getNameSpace (),
                                   cop->getClassName (),
                                   (Boolean)lo,
                                   (Boolean)iq,
                                   (Boolean)ic,
                                   pl);

      return DEBUG_ConvertCToJava (CIMClass*, jlong, new CIMClass (cls));
   }
   Catch(jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1deleteClass
   (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;

   try {
      ch->deleteClass (ctx,
                       cop->getNameSpace (),
                       cop->getClassName ());
   }
   Catch(jEnv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1createClass
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jlong jCl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMClass         *cl  = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCl);
   OperationContext  ctx;

   try {
      ch->createClass (ctx,
                       cop->getNameSpace (),
                       *cl);
   }
   Catch(jEnv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1setClass
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jlong jCl)
{
   CIMOMHandle     *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath   *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMClass        *cl  = DEBUG_ConvertJavaToC (jlong, CIMClass*, jCl);
   OperationContext ctx;

   try {
     ch->modifyClass (ctx,
                      cop->getNameSpace (),
                      *cl);
   }
   Catch(jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1enumerateClassNames
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jboolean deep)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMNamespaceName  ns  = cop->getNameSpace ();
   OperationContext  ctx;

   try {
      Array<CIMName> enm = ch->enumerateClassNames(ctx,
                                                   cop->getNameSpace(),
                                                   cop->getClassName(),
                                                   (Boolean)deep);
      Array<CIMObjectPath> *enmop = new Array<CIMObjectPath> ();

      for (int i = 0, m = enm.size (); i < m; i++)
      {
         enmop->append (CIMObjectPath (String::EMPTY,
                                       ns,
                                       enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMObjectPath>*, jlong, enmop);
   }
   Catch(jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1enumerateClasses
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jboolean deep,
        jboolean lo, jboolean iq, jboolean ic)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;

   try {
      Array<CIMClass> en = ch->enumerateClasses (ctx,
                                                 cop->getNameSpace(),
                                                 cop->getClassName(),
                                                 (Boolean)deep,
                                                 (Boolean)lo,
                                                 (Boolean)iq,
                                                 (Boolean)ic);

      return DEBUG_ConvertCToJava(
                 Array<CIMClass>*,
                 jlong,
                 new Array<CIMClass> (en));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1getInstance(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCh,
    jlong jCop,
    jboolean lo,
    jboolean iq,
    jboolean ic,
    jobjectArray jPl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   CIMPropertyList   pl;

   if (jPl)
      pl = getList (jEnv,jPl);
   else
      pl = CIMPropertyList ();

   try {
      CIMInstance *inst = new CIMInstance(ch->getInstance(
                                                  ctx,
                                                  cop->getNameSpace(),
                                                  *cop,
                                                  (Boolean)lo,
                                                  (Boolean)iq,
                                                  (Boolean)ic,
                                                  pl));

      return DEBUG_ConvertCToJava (CIMInstance*, jlong, inst);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1deleteInstance
   (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;

   try {
      ch->deleteInstance (ctx,cop->getNameSpace (),*cop);
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1createInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jlong jCi)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMInstance      *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jCi);
   OperationContext  ctx;

   try {
      ci->setPath (*cop);

      CIMObjectPath obj = ch->createInstance (ctx,
                                              cop->getNameSpace (),
                                              *ci);

      return DEBUG_ConvertCToJava(CIMObjectPath*,jlong,new CIMObjectPath(obj));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1modifyInstance
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jlong jCi,
         jboolean iq, jobjectArray jPl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMInstance      *ci  = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jCi);
   CIMPropertyList   pl  = getList (jEnv, jPl);
   OperationContext  ctx;

   try {
      ci->setPath (*cop);
      ch->modifyInstance (ctx,
                          cop->getNameSpace (),
                          *ci,
                          (Boolean)iq,
                          pl);
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL
Java_org_pegasus_jmpi_CIMOMHandle__1enumerateInstanceNames(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCh,
    jlong jCop,
    jboolean deep)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;

   try {
      Array<CIMObjectPath> enm = ch->enumerateInstanceNames(
                                         ctx,
                                         cop->getNameSpace(),
                                         cop->getClassName());
      return DEBUG_ConvertCToJava(
                 Array<CIMObjectPath>*,
                 jlong,
                 new Array<CIMObjectPath> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1enumerateInstances
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jboolean deep,
        jboolean lo, jboolean iq, jboolean ic, jobjectArray jPl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   CIMPropertyList   pl;
   OperationContext  ctx;

   if (jPl)
      pl = getList (jEnv, jPl);
   else
      pl = CIMPropertyList ();

   try {
      Array<CIMInstance> en = ch->enumerateInstances (ctx,
                                                      cop->getNameSpace (),
                                                      cop->getClassName (),
                                                      (Boolean)deep,
                                                      (Boolean)lo,
                                                      (Boolean)iq,
                                                      (Boolean)ic,
                                                      pl);

      return DEBUG_ConvertCToJava(
                 Array<CIMInstance>*,
                 jlong,
                 new Array<CIMInstance> (en));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1execQuery(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCh,
    jlong jCop,
    jstring jQuery,
    jstring jQl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   const char       *str = jEnv->GetStringUTFChars (jQuery,NULL);
   String            query (str);

   jEnv->ReleaseStringUTFChars (jQuery,str);

   str = jEnv->GetStringUTFChars (jQl, NULL);

   String ql (str);

   jEnv->ReleaseStringUTFChars (jQl, str);

   try {
      Array<CIMObject>    enm = ch->execQuery (ctx,
                                               cop->getNameSpace (),
                                               ql,
                                               query);
      Array<CIMInstance> *enmInst = new Array<CIMInstance> ();

      for (int i = 0,m = enm.size (); i<m; i++)
      {
         enmInst->append (CIMInstance (enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMInstance>*, jlong, enmInst);
   }
   Catch (jEnv);

   return 0;
}


JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1getProperty
   (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jstring jN)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   const char       *str = jEnv->GetStringUTFChars (jN, NULL);
   CIMName           prop (str);

   jEnv->ReleaseStringUTFChars (jN, str);

   try {
      CIMValue *cv = new CIMValue (ch->getProperty (ctx,
                                                    cop->getNameSpace (),
                                                    *cop,
                                                    prop));

      return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1setProperty
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jstring jPn, jlong jV)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   CIMValue         *val = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);
   const char       *str = jEnv->GetStringUTFChars (jPn, NULL);
   CIMName           pName (str);

   jEnv->ReleaseStringUTFChars (jPn, str);

   try {
      ch->setProperty (ctx,
                       cop->getNameSpace (),
                       *cop,
                       pName,
                       *val);
   }
   Catch (jEnv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1associatorNames(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCh,
    jlong jCop,
    jstring jAssocClass,
    jstring jResultClass,
    jstring jRole,
    jstring jResultRole)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   const char       *str = jEnv->GetStringUTFChars (jAssocClass, NULL);

   CIMName           assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass, str);

   str = jEnv->GetStringUTFChars (jResultClass, NULL);

   CIMName resultClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         resultClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid result class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jResultClass, str);

   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   str = jEnv->GetStringUTFChars (jResultRole, NULL);

   String resultRole (str);

   jEnv->ReleaseStringUTFChars (jResultRole, str);

   try {
      Array<CIMObjectPath> enm = ch->associatorNames (ctx,
                                                      cop->getNameSpace (),
                                                      *cop,
                                                      assocClass,
                                                      resultClass,
                                                      role,
                                                      resultRole);

      return DEBUG_ConvertCToJava(
                 Array<CIMObjectPath>*,
                 jlong,
                 new Array<CIMObjectPath> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1associators(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCh,
    jlong jCop,
    jstring jAssocClass,
    jstring jResultClass,
    jstring jRole,
    jstring jResultRole,
    jboolean includeQualifiers,
    jboolean includeClassOrigin,
    jobjectArray jPl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   CIMPropertyList   pl  = getList (jEnv, jPl);
   const char       *str = jEnv->GetStringUTFChars (jAssocClass, NULL);
   CIMName           assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass, str);

   str = jEnv->GetStringUTFChars (jResultClass, NULL);

   CIMName resultClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         resultClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid result class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jResultClass, str);

   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   str = jEnv->GetStringUTFChars (jResultRole, NULL);

   String resultRole (str);

   jEnv->ReleaseStringUTFChars (jResultRole, str);

   try {
      Array<CIMObject> enm = ch->associators(
                                     ctx,
                                     cop->getNameSpace(),
                                     *cop,
                                     assocClass,
                                     resultClass,
                                     role,
                                     resultRole,
                                     (Boolean)includeQualifiers,
                                     (Boolean)includeClassOrigin,
                                     pl);
      Array<CIMInstance> *enmInst = new Array<CIMInstance> ();

      for (int i = 0, m = enm.size (); i < m; i++)
      {
         enmInst->append (CIMInstance (enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMInstance>*, jlong, enmInst);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1referenceNames
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop,
   jstring jAssocClass, jstring jRole)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   const char       *str = jEnv->GetStringUTFChars (jAssocClass, NULL);
   CIMName           assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass, str);

   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   try {
      Array<CIMObjectPath> enm = ch->referenceNames (ctx,
                                                     cop->getNameSpace (),
                                                     *cop,
                                                     assocClass,
                                                     role);

      return DEBUG_ConvertCToJava(
                 Array<CIMObjectPath>*,
                 jlong,
                 new Array<CIMObjectPath> (enm));
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1references
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop,
   jstring jAssocClass, jstring jRole,
   jboolean includeQualifiers, jboolean includeClassOrigin, jobjectArray jPl)
{
   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   CIMPropertyList   pl  = getList (jEnv, jPl);
   const char       *str = jEnv->GetStringUTFChars (jAssocClass, NULL);
   CIMName           assocClass;

   if (  str
      && *str
      )
   {
      if (CIMName::legal (str))
      {
         assocClass = str;
      }
      else
      {
         jobject ev = jEnv->NewObject (JMPIjvm::jv.CIMExceptionClassRef,
                                       JMPIjvm::jv.CIMExceptionNewISt,
                                       (jint)4, // CIM_ERR_INVALID_PARAMETER
                                       jEnv->NewStringUTF(
                                           "Invalid association class name"));

         jEnv->Throw ((jthrowable)ev);

         return 0;
      }
   }

   jEnv->ReleaseStringUTFChars (jAssocClass, str);

   str = jEnv->GetStringUTFChars (jRole, NULL);

   String role (str);

   jEnv->ReleaseStringUTFChars (jRole, str);

   try {
      Array<CIMObject>    enm     = ch->references (ctx,
                                                    cop->getNameSpace (),
                                                    *cop,
                                                    assocClass,
                                                    role,
                                                    (Boolean)includeQualifiers,
                                                    (Boolean)includeClassOrigin,
                                                    pl);
      Array<CIMInstance> *enmInst = new Array<CIMInstance> ();

      for (int i = 0, m = enm.size (); i < m; i++)
      {
         enmInst->append (CIMInstance (enm[i]));
      }

      return DEBUG_ConvertCToJava (Array<CIMInstance>*, jlong, enmInst);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1invokeMethod(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCh,
    jlong jCop,
    jstring jMn,
    jobject jIn,
    jobject jOut)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   const char       *str = jEnv->GetStringUTFChars (jMn,NULL);
   CIMName           method (str);

   jEnv->ReleaseStringUTFChars (jMn,str);

   Array<CIMParamValue> in;
   Array<CIMParamValue> out;

   for (int i=0,m=jEnv->CallIntMethod(jIn,JMPIjvm::jv.VectorSize);i<m;i++)
   {
       JMPIjvm::checkException (jEnv);

       jobject jProp=jEnv->CallObjectMethod(jIn,JMPIjvm::jv.VectorElementAt,i);

       JMPIjvm::checkException (jEnv);

       jlong jp = jEnv->CallLongMethod(jProp,JMPIjvm::jv.CIMPropertyCInst);
       CIMProperty *p  = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jp);

       JMPIjvm::checkException (jEnv);

       in.append (CIMParamValue (p->getName ().getString (),p->getValue ()));
   }
   try {
      CIMValue *val = new CIMValue (ch->invokeMethod (ctx,
                                                      cop->getNameSpace (),
                                                      *cop,
                                                      method,
                                                      in,
                                                      out));

      for (int i = 0,m = out.size (); i<m; i++)
      {
         const CIMParamValue &parm = out[i];
         const CIMValue       v    = parm.getValue ();
         CIMProperty         *p    = new CIMProperty (parm.getParameterName (),
                                                      v,
                                                      v.getArraySize ());
         jlong jp   = DEBUG_ConvertCToJava (CIMProperty*, jlong, p);
         jobject prop = jEnv->NewObject (JMPIjvm::jv.CIMPropertyClassRef,
                                         JMPIjvm::jv.CIMPropertyNewJ,
                                         jp);

         jEnv->CallVoidMethod (jOut, JMPIjvm::jv.VectorAddElement, prop);
      }
      return DEBUG_ConvertCToJava (CIMValue*, jlong, val);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1invokeMethod24
  (JNIEnv *jEnv, jobject jThs, jlong jCh, jlong jCop, jstring jMn,
        jobjectArray jIn, jobjectArray jOut)

{
   JMPIjvm::cacheIDs (jEnv);

   CIMOMHandle      *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMObjectPath    *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jCop);
   OperationContext  ctx;
   const char       *str = jEnv->GetStringUTFChars (jMn, NULL);
   CIMName           method (str);

   jEnv->ReleaseStringUTFChars (jMn, str);

   Array<CIMParamValue> in;
   Array<CIMParamValue> out;

   if (jIn)
   {
      for (int i = 0, m = jEnv->GetArrayLength (jIn); i < m; i++)
      {
          JMPIjvm::checkException (jEnv);

          jobject jArg = jEnv->GetObjectArrayElement (jIn,i);

          JMPIjvm::checkException (jEnv);

          jlong jp = jEnv->CallLongMethod (jArg,JMPIjvm::jv.CIMArgumentCInst);
          CIMParamValue *p  = DEBUG_ConvertJavaToC (jlong, CIMParamValue*, jp);

          JMPIjvm::checkException (jEnv);

          in.append (*p);
      }
   }
   try {
      CIMValue *val = new CIMValue (ch->invokeMethod (ctx,
                                                      cop->getNameSpace (),
                                                      *cop,
                                                      method,
                                                      in,
                                                      out));

      if (jOut)
      {
         for (int i=0,m=out.size(),o=jEnv->GetArrayLength(jOut);i<m && i<o;i++)
         {
            CIMParamValue *parm  = new CIMParamValue (out[i]);
            jlong jParm = DEBUG_ConvertCToJava (CIMParamValue*, jlong, parm);

            jEnv->SetObjectArrayElement(
                jOut,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.CIMArgumentClassRef,
                    JMPIjvm::jv.CIMArgumentNewJ,jParm));
         }
      }
      return DEBUG_ConvertCToJava (CIMValue*, jlong, val);
   }
   Catch (jEnv);

   return 0;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1deliverEvent(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCh,
    jstring jName,
    jstring jNs,
    jlong jInd)
{
   CIMOMHandle *ch  = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);
   CIMInstance *ind = DEBUG_ConvertJavaToC (jlong, CIMInstance*, jInd);
   const char  *str = jEnv->GetStringUTFChars (jName, NULL);
   String       name (str);

   jEnv->ReleaseStringUTFChars (jName, str);

   str = jEnv->GetStringUTFChars (jNs, NULL);

   String ns (str);

   jEnv->ReleaseStringUTFChars (jNs, str);

   CIMObjectPath ref (ind->getPath ());

   ref.setNameSpace (ns);

   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
       "Java_org_pegasus_jmpi_CIMOMHandle__1deliverEvent(): ref = %s",
       (const char*)ref.toString().getCString()));
   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
       "Java_org_pegasus_jmpi_CIMOMHandle__1deliverEvent(): ind = %s",
       (const char*)ind->getPath().toString().getCString()));

   ind->setPath (ref);

   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
       "Java_org_pegasus_jmpi_CIMOMHandle__1deliverEvent(): ind = %s",
       (const char*)ind->getPath().toString().getCString()));

   JMPIProviderManager::indProvRecord *prec = NULL;
   String sPathString = ind->getPath ().toString ();
   OperationContext *context = NULL;
   bool fResult = false;

   {
      AutoMutex lock (JMPIProviderManager::mutexProvTab);

      fResult = JMPIProviderManager::provTab.lookup (name, prec);
      PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
          "Java_org_pegasus_jmpi_CIMOMHandle__1deliverEvent(): "
          "fResult = %d, name = %s",
          fResult,(const char*)name.getCString()));
   }

   if (fResult)
   {
      if (prec->enabled)
      {
         try
         {
            prec->handler->deliver (*prec->ctx, *ind);
         }
         Catch (jEnv);
      }
   }
   else
   {
      PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL1,
          "Java_org_pegasus_jmpi_CIMOMHandle__1deliverEvent(): "
          "provider name \"%s\" not found",
          (const char*)name.getCString()));
   }
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMOMHandle__1finalize
   (JNIEnv *jEnv, jobject jThs, jlong jCh)
{
   CIMOMHandle *ch = DEBUG_ConvertJavaToC (jlong, CIMOMHandle*, jCh);

   delete ch;

   DEBUG_ConvertCleanup (jlong, jCh);
}


// -------------------------------------
// ---
// -        CIMParameter
// ---
// -------------------------------------

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMParameter__1getName
   (JNIEnv *jEnv, jobject jThs, jlong jCp)
{
   CIMParameter *cp  = DEBUG_ConvertJavaToC (jlong, CIMParameter*, jCp);
   jstring       rv  = 0;

   if (cp)
   {
      try
      {
         CIMName name = cp->getName ();

         rv = jEnv->NewStringUTF (name.getString ().getCString ());
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMParameter__1setName
   (JNIEnv *jEnv, jobject jThs, jlong jCp, jstring jName)
{
   CIMParameter *cp  = DEBUG_ConvertJavaToC (jlong, CIMParameter*, jCp);

   if (cp && jName)
   {
      try
      {
         const char *cstrName = jEnv->GetStringUTFChars (jName, NULL);

         CIMName name (cstrName);

         cp->setName (name);

         jEnv->ReleaseStringUTFChars (jName, cstrName);
      }
      Catch (jEnv);
   }
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMParameter__1isArray
   (JNIEnv *jEnv, jobject jThs, jlong jCp)
{
   CIMParameter *cp  = DEBUG_ConvertJavaToC (jlong, CIMParameter*, jCp);
   jboolean      rv  = 0;

   if (cp)
   {
      try
      {
         rv = cp->isArray ();
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMParameter__1getArraySize
   (JNIEnv *jEnv, jobject jThs, jlong jCp)
{
   CIMParameter *cp  = DEBUG_ConvertJavaToC (jlong, CIMParameter*, jCp);
   jint          rv  = 0;

   if (cp)
   {
      try
      {
         rv = cp->getArraySize ();
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT jstring JNICALL
Java_org_pegasus_jmpi_CIMParameter__1getReferenceClassName(
    JNIEnv *jEnv,
    jobject jThs,
    jlong jCp)
{
   CIMParameter *cp  = DEBUG_ConvertJavaToC (jlong, CIMParameter*, jCp);
   jstring       rv  = 0;

   if (cp)
   {
      try
      {
         CIMName name = cp->getReferenceClassName ();

         rv = jEnv->NewStringUTF (name.getString ().getCString ());
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMParameter__1getType
   (JNIEnv *jEnv, jobject jThs, jlong jCp)
{
   CIMParameter *cp  = DEBUG_ConvertJavaToC (jlong, CIMParameter*, jCp);
   jlong         rv  = 0;

   if (cp)
   {
      try
      {
         CIMType    ct       = cp->getType ();
         bool       fSuccess = false;
         String     ref      = cp->getReferenceClassName ().getString ();
         int        iJType   = 0;
         _dataType *type     = 0;

         iJType = _dataType::convertCTypeToJavaType (ct, &fSuccess);

         if (fSuccess)
         {
            type = new _dataType (iJType,
                                  cp->getArraySize (),
                                  false,
                                  false,
                                  cp->isArray (),
                                  ref,
                                  true);

            rv = DEBUG_ConvertCToJava (_dataType*, jlong, type);
         }
      }
      Catch (jEnv);
   }

   return rv;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMParameter__1finalize
   (JNIEnv *jEnv, jobject jThs, jlong jCp)
{
   CIMParameter *cp = DEBUG_ConvertJavaToC (jlong, CIMParameter*, jCp);

   delete cp;

   DEBUG_ConvertCleanup (jlong, jCp);
}


// -------------------------------------
// ---
// -        CIMProperty
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMProperty__1getValue
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty *cp = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   CIMValue    *cv = new CIMValue (cp->getValue ());

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMProperty__1getName
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty  *cp  = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   const String &n   = cp->getName ().getString ();
   jstring       str = jEnv->NewStringUTF (n.getCString ());

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMProperty__1setName
      (JNIEnv *jEnv, jobject jThs, jlong jP, jstring jN)
{
   CIMProperty *cp  = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   const char  *str = jEnv->GetStringUTFChars (jN,NULL);

   cp->setName (CIMName (str));

   jEnv->ReleaseStringUTFChars (jN,str);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMProperty__1property
      (JNIEnv *jEnv, jobject jThs, jstring jN, jlong jV)
{
   CIMValue    *cv  = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);
   const char  *str = jEnv->GetStringUTFChars (jN, NULL);
   CIMProperty *cp  = 0;
   jlong        jCp = 0;

   try {
      if (cv->getType ()!= CIMTYPE_REFERENCE)
      {
         cp = new CIMProperty (String (str),*cv);
      }
      else
      {
         if (!cv->isArray ())
         {
            CIMObjectPath cop;

            cv->get (cop);
            cp = new CIMProperty (String (str),*cv,0, cop.getClassName ());
         }
         else
         {
            throwCIMException(
                jEnv,
                "+++ unsupported type in CIMProperty.property");
         }
      }

      jCp = DEBUG_ConvertCToJava (CIMProperty*, jlong, cp);
   }
   Catch (jEnv);

   jEnv->ReleaseStringUTFChars (jN, str);

   return jCp;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMProperty__1new
      (JNIEnv *jEnv, jobject jThs, jstring jN, jlong jV)
{
   CIMProperty *p = new CIMProperty (CIMName (),CIMValue ());

   return DEBUG_ConvertCToJava (CIMProperty*, jlong, p);
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMProperty__1isReference
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty *cp = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);

   return (jboolean) (cp->getType () == CIMTYPE_REFERENCE);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMProperty__1getRefClassName
     (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty   *cp = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   const String  &n  = cp->getReferenceClassName ().getString ();

   jstring str = jEnv->NewStringUTF (n.getCString ());

   return str;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMProperty__1getType
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty  *cp       = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   String        ref      = cp->getReferenceClassName ().getString ();
   bool          fSuccess = false;
   int           iJType   = 0;
   _dataType     *type    = 0;

   iJType = _dataType::convertCTypeToJavaType (cp->getType (), &fSuccess);

   if (fSuccess)
   {
      type = new _dataType (iJType,
                            cp->getArraySize (),
                            ref.size () ? true : false,
                            false,
                            cp->isArray (),
                            ref,
                            true);
   }

   return DEBUG_ConvertCToJava (_dataType*, jlong, type);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMProperty__1setType
      (JNIEnv *jEnv, jobject jThs, jlong jP, jlong jDt)
{
   CIMProperty  *cp       = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   _dataType    *dt       = DEBUG_ConvertJavaToC (jlong, _dataType*, jDt);
   CIMValue      val;
   bool          fSuccess = false;
   CIMType       cType    = CIMTYPE_BOOLEAN;

   cType = _dataType::convertJavaTypeToCType (dt->_type, &fSuccess);

   if (fSuccess)
   {
      val.setNullValue (cType, dt->_array);

      CIMProperty *np = new CIMProperty (cp->getName (), val);

      delete cp;

      DEBUG_ConvertCleanup (jlong, jP);

      return DEBUG_ConvertCToJava (CIMProperty*, jlong, np);
   }
   else
   {
      return DEBUG_ConvertCToJava (CIMProperty*, jlong, cp);
   }
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMProperty__1setValue
      (JNIEnv *jEnv, jobject jThs, jlong jP,jlong jV)
{
   CIMProperty *cp = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   CIMValue    *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);

   cp->setValue (*cv);
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMProperty__1isArray
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty *cp = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);

   return (jboolean)cp->isArray ();
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMProperty__1getIdentifier
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty  *cp  = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   const String &n   = cp->getName ().getString ();
   jstring       str = jEnv->NewStringUTF (n.getCString ());

   return str;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMProperty__1addValue
      (JNIEnv *jEnv, jobject jThs, jlong jP, jlong jV)
{
   CIMProperty *cp   = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   CIMValue    *cvin = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);
   CIMValue     cv   = cp->getValue ();

   if (cvin->isNull ())
      throwCIMException (jEnv,"+++ null cvin value ");
   if (!cv.isArray ())
      throwCIMException (jEnv,"+++ not an array ");
   if (cvin->getType ()!= cv.getType ())
      throwCIMException (jEnv,"+++ type mismatch ");

   CIMType type = cv.getType ();

   switch (type)
   {
   case CIMTYPE_BOOLEAN:
   {
         Boolean bo;
         cvin->get (bo);
         Array<Boolean> boarr;
         cv.get (boarr);
         boarr.append (bo);
         cv.set (boarr);
         break;
   }
   case CIMTYPE_UINT8:
   {
         Uint8 u8;
         cvin->get (u8);
         Array<Uint8> u8arr;
         cv.get (u8arr);
         u8arr.append (u8);
         cv.set (u8arr);
         break;
   }
   case CIMTYPE_SINT8:
   {
         Sint8 s8;
         cvin->get (s8);
         Array<Sint8> s8arr;
         cv.get (s8arr);
         s8arr.append (s8);
         cv.set (s8arr);
         break;
   }
   case CIMTYPE_UINT16:
   {
         Uint16 u16;
         cvin->get (u16);
         Array<Uint16> u16arr;
         cv.get (u16arr);
         u16arr.append (u16);
         cv.set (u16arr);
         break;
   }
   case CIMTYPE_SINT16:
   {
         Sint16 s16;
         cvin->get (s16);
         Array<Sint16> s16arr;
         cv.get (s16arr);
         s16arr.append (s16);
         cv.set (s16arr);
         break;
   }
   case CIMTYPE_UINT32:
   {
         Uint32 u32;
         cvin->get (u32);
         Array<Uint32> u32arr;
         cv.get (u32arr);
         u32arr.append (u32);
         cv.set (u32arr);
         break;
   }
   case CIMTYPE_SINT32:
   {
         Sint32 s32;
         cvin->get (s32);
         Array<Sint32> s32arr;
         cv.get (s32arr);
         s32arr.append (s32);
         cv.set (s32arr);
         break;
   }
   case CIMTYPE_UINT64:
   {
         Uint64 u64;
         cvin->get (u64);
         Array<Uint64> u64arr;
         cv.get (u64arr);
         u64arr.append (u64);
         cv.set (u64arr);
         break;
   }
   case CIMTYPE_SINT64:
   {
         Sint64 s64;
         cvin->get (s64);
         Array<Sint64> s64arr;
         cv.get (s64arr);
         s64arr.append (s64);
         cv.set (s64arr);
         break;
   }
   case CIMTYPE_REAL32:
   {
         Real32 f;
         cvin->get (f);
         Array<Real32> farr;
         cv.get (farr);
         farr.append (f);
         cv.set (farr);
         break;
   }
   case CIMTYPE_REAL64:
   {
         Real64 d;
         cvin->get (d);
         Array<Real64> darr;
         cv.get (darr);
         darr.append (d);
         cv.set (darr);
         break;
   }
   case CIMTYPE_CHAR16:
   {
         Char16 c16;
         cvin->get (c16);
         Array<Char16> c16arr;
         cv.get (c16arr);
         c16arr.append (c16);
         cv.set (c16arr);
         break;
   }
   case CIMTYPE_STRING:
   {
         String str;
         cvin->get (str);
         Array<String> strarr;
         cv.get (strarr);
         strarr.append (str);
         cv.set (strarr);
         break;
   }
   case CIMTYPE_DATETIME:
   {
         CIMDateTime dt;
         cvin->get (dt);
         Array<CIMDateTime> dtarr;
         cv.get (dtarr);
         dtarr.append (dt);
         cv.set (dtarr);
         break;
   }
   case CIMTYPE_REFERENCE:
   {
         CIMObjectPath ref;
         cvin->get (ref);
         Array<CIMObjectPath> refarr;
         cv.get (refarr);
         refarr.append (ref);
         cv.set (refarr);
         break;
   }
   case CIMTYPE_OBJECT:
   {
         CIMObject obj;
         cvin->get (obj);
         Array<CIMObject> objarr;
         cv.get (objarr);
         objarr.append (obj);
         cv.set (objarr);
         break;
   }
   default:
   {
      throwCIMException (jEnv,"+++ unsupported type ");
   }
   }

   cp->setValue (cv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMProperty__1addQualifier
      (JNIEnv *jEnv, jobject jThs, jlong jP, jlong jQ)
{
   CIMProperty  *cp = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   CIMQualifier *cq = DEBUG_ConvertJavaToC (jlong, CIMQualifier*, jQ);

   cp->addQualifier (*cq);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMProperty__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMProperty *cp = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);

   delete cp;

   DEBUG_ConvertCleanup (jlong, jP);
}

/*
 * Class:     org_pegasus_jmpi_CIMProperty
 * Method:    _findQualifier
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMProperty__1findQualifier
  (JNIEnv *jEnv, jobject jThs, jlong jP, jstring jQualifier)
{
   CIMProperty  *cp    = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   const char   *str   = jEnv->GetStringUTFChars (jQualifier, NULL);
   Uint32        index = PEG_NOT_FOUND;

   try
   {
      index = cp->findQualifier (CIMName (str));

      jEnv->ReleaseStringUTFChars (jQualifier, str);
   }
   Catch (jEnv);

   return index;
}

/*
 * Class:     org_pegasus_jmpi_CIMProperty
 * Method:    _getQualifier
 * Signature: (I)I
 */
JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMProperty__1getQualifier
  (JNIEnv *jEnv, jobject jThs, jlong jP, jint jIndex)
{
   CIMProperty  *cp        = DEBUG_ConvertJavaToC (jlong, CIMProperty*, jP);
   CIMQualifier *cq        = NULL;
   CIMQualifier  qualifier;

   try
   {
      qualifier = cp->getQualifier ((Uint32)jIndex);
      cq = new CIMQualifier (qualifier);
   }
   Catch (jEnv);

   return DEBUG_ConvertCToJava (CIMQualifier *, jlong, cq);
}


// -------------------------------------
// ---
// -     CIMQualifier
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMQualifier__1new
      (JNIEnv *jEnv, jobject jThs, jstring jN)
{
   const char   *str  = jEnv->GetStringUTFChars (jN,NULL);
   CIMQualifier *qual = new CIMQualifier (CIMName (str),CIMValue ());

   jEnv->ReleaseStringUTFChars (jN,str);

   return DEBUG_ConvertCToJava (CIMQualifier*, jlong, qual);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMQualifier__1getName
      (JNIEnv *jEnv, jobject jThs, jlong jQ)
{
   CIMQualifier *cq  = DEBUG_ConvertJavaToC (jlong, CIMQualifier*, jQ);
   const String &n   = cq->getName ().getString ();
   jstring       str = jEnv->NewStringUTF (n.getCString ());

   return str;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMQualifier__1getValue
      (JNIEnv *jEnv, jobject jThs, jlong jQ)
{
   CIMQualifier *cq = DEBUG_ConvertJavaToC (jlong, CIMQualifier*, jQ);
   CIMValue     *cv = new CIMValue (cq->getValue ());

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMQualifier__1setValue
      (JNIEnv *jEnv, jobject jThs, jlong jQ,jlong jV)
{
   CIMQualifier *cq = DEBUG_ConvertJavaToC (jlong, CIMQualifier*, jQ);
   CIMValue     *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);

   cq->setValue (*cv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMQualifier__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jQ)
{
   CIMQualifier *cq = DEBUG_ConvertJavaToC (jlong, CIMQualifier*, jQ);

   delete cq;

   DEBUG_ConvertCleanup (jlong, jQ);
}


// -------------------------------------
// ---
// -     CIMQualifierType
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMQualifierType__1new
      (JNIEnv *jEnv, jobject jThs)
{
   CIMQualifierDecl *qual = new CIMQualifierDecl ();

   return DEBUG_ConvertCToJava (CIMQualifierDecl*, jlong, qual);
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMQualifierType__1getName
      (JNIEnv *jEnv, jobject jThs, jlong jQ)
{
   CIMQualifierDecl *qt  = DEBUG_ConvertJavaToC (jlong, CIMQualifierDecl*, jQ);
   const String     &n   = qt->getName ().getString ();
   jstring           str = jEnv->NewStringUTF (n.getCString ());

   return str;
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMQualifierType__1setName
      (JNIEnv *jEnv, jobject jThs, jlong jQ, jstring jN)
{
   CIMQualifierDecl *qt   = DEBUG_ConvertJavaToC (jlong, CIMQualifierDecl*, jQ);
   const char       *str  = jEnv->GetStringUTFChars (jN, NULL);
   jlong             jret = 0;

   if (qt->isUninitialized ())
   {
      CIMQualifierDecl *nqt = new CIMQualifierDecl(
                                      CIMName(str),
                                      CIMValue(),
                                      CIMScope());

      jret = DEBUG_ConvertCToJava (CIMQualifierDecl*, jlong, nqt);
   }
   else
   {
      qt->setName (CIMName (str));

      CIMQualifierDecl *nqt = new CIMQualifierDecl (*qt);

      jret = DEBUG_ConvertCToJava (CIMQualifierDecl*, jlong, nqt);
   }

   jEnv->ReleaseStringUTFChars (jN, str);

   return jret;
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMQualifierType__1setValue
      (JNIEnv *jEnv, jobject jThs, jlong jQ,jlong jV)
{
   CIMQualifierDecl *qt = DEBUG_ConvertJavaToC (jlong, CIMQualifierDecl*, jQ);
   CIMValue         *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);

   qt->setValue (*cv);
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMQualifierType__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jQ)
{
   CIMQualifierDecl *qt = DEBUG_ConvertJavaToC (jlong, CIMQualifierDecl*, jQ);

   delete qt;

   DEBUG_ConvertCleanup (jlong, jQ);
}

// -------------------------------------
// ---
// -        CIMValue
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1byte
      (JNIEnv *jEnv, jobject jThs, jbyte jb, jboolean notSigned)
{
   CIMValue *cv = NULL;

   if (notSigned)
      cv = new CIMValue ((Uint8)jb);
   else
      cv = new CIMValue ((Sint8)jb);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1short
      (JNIEnv *jEnv, jobject jThs, jshort js, jboolean notSigned)
{
   CIMValue *cv = NULL;

   if (notSigned)
      cv = new CIMValue ((Uint16)js);
   else
      cv = new CIMValue( (Sint16)js);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1makeInt
      (JNIEnv *jEnv, jobject jThs, jlong ji, jboolean notSigned)
{
   CIMValue *cv = NULL;

   if (notSigned)
      cv = new CIMValue ((Uint32)ji);
   else
      cv = new CIMValue ((Sint32)ji);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1long
      (JNIEnv *jEnv, jobject jThs, jlong jl, jboolean notSigned)
{
   CIMValue *cv = NULL;

   if (notSigned)
      cv = new CIMValue ((Uint64)jl);
   else
      cv = new CIMValue ((Sint64)jl);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1string
      (JNIEnv *jEnv, jobject jThs, jstring jS)
{
   const char *str = jEnv->GetStringUTFChars (jS,NULL);
   CIMValue   *cv  = new CIMValue (String (str));

   jEnv->ReleaseStringUTFChars (jS,str);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1boolean
      (JNIEnv *jEnv, jobject jThs, jboolean jB)
{
   CIMValue *cv = new CIMValue ((Boolean)jB);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1float
      (JNIEnv *jEnv, jobject jThs, jfloat jF)
{
   CIMValue *cv = new CIMValue (jF);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1double
      (JNIEnv *jEnv, jobject jThs, jdouble jD)
{
   CIMValue *cv = new CIMValue (jD);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1cop
      (JNIEnv *jEnv, jobject jThs, jlong jR)
{
   CIMObjectPath *ref = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jR);
   CIMValue      *cv  = new CIMValue (*ref);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1datetime
      (JNIEnv *jEnv, jobject jThs, jlong jDT)
{
   CIMDateTime *dt = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jDT);
   CIMValue    *cv = new CIMValue (*dt);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1char16
      (JNIEnv *jEnv, jobject jThs, jchar jChar16)
{
   Char16       c16 (jChar16);
   CIMValue    *cv  = new CIMValue (c16);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1object
      (JNIEnv *jEnv, jobject jThs, jlong jO)
{
   CIMObject *co = DEBUG_ConvertJavaToC (jlong, CIMObject*, jO);
   CIMValue  *cv = new CIMValue (*co);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1byteArray
      (JNIEnv *jEnv, jobject jThs, jshortArray jshortA, jboolean notSigned)
{
   CIMValue *cv  = NULL;
   jboolean  b;
   jsize     len = jEnv->GetArrayLength (jshortA);
   jshort   *jsA = jEnv->GetShortArrayElements (jshortA, &b);

   if (notSigned)
   {
      Array<Uint8> u8;

      for (jsize i = 0; i < len; i++)
         u8.append ((Uint8)jsA[i]);

      cv = new CIMValue (u8);
   }
   else
   {
      Array<Sint8> s8;

      for (jsize i = 0; i < len; i++)
         s8.append ((Sint8)jsA[i]);

      cv = new CIMValue (s8);
   }

   jEnv->ReleaseShortArrayElements (jshortA, jsA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1shortArray
      (JNIEnv *jEnv, jobject jThs, jintArray jintA, jboolean notSigned)
{
   CIMValue *cv  = NULL;
   jboolean  b;
   jsize     len = jEnv->GetArrayLength (jintA);
   jint     *jiA = jEnv->GetIntArrayElements (jintA, &b);

   if (notSigned)
   {
      Array<Uint16> u16;

      for (jsize i = 0; i < len; i++)
         u16.append ((Uint16)jiA[i]);

      cv = new CIMValue (u16);
   }
   else
   {
      Array<Sint16> s16;

      for (jsize i = 0; i < len; i++)
         s16.append ((Sint16)jiA[i]);

      cv = new CIMValue (s16);
   }

   jEnv->ReleaseIntArrayElements (jintA, jiA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1intArray
      (JNIEnv *jEnv, jobject jThs, jlongArray jlongA, jboolean notSigned)
{
   CIMValue *cv  = NULL;
   jboolean  b;
   jsize     len = jEnv->GetArrayLength (jlongA);
   jlong    *jlA = jEnv->GetLongArrayElements (jlongA, &b);

   if (notSigned)
   {
      Array<Uint32> u32;

      for (jsize i = 0; i < len; i++)
         u32.append ((Uint32)jlA[i]);

      cv = new CIMValue (u32);
   }
   else
   {
      Array<Sint32> s32;

      for (jsize i = 0; i < len; i++)
         s32.append ((Sint32)jlA[i]);

      cv = new CIMValue (s32);
   }

   jEnv->ReleaseLongArrayElements (jlongA, jlA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1longArray
      (JNIEnv *jEnv, jobject jThs, jlongArray jlongA, jboolean notSigned)
{
   CIMValue *cv  = NULL;
   jboolean  b;
   jsize     len = jEnv->GetArrayLength (jlongA);
   jlong    *jlA = jEnv->GetLongArrayElements (jlongA, &b);

   if (notSigned)
   {
      Array<Uint64> u64;

      for (jsize i = 0; i < len; i++)
         u64.append ((Uint64)jlA[i]);

      cv = new CIMValue (u64);
   }
   else
   {
      Array<Sint64> s64;

      for (jsize i = 0; i < len; i++)
         s64.append ((Sint64)jlA[i]);

      cv = new CIMValue (s64);
   }

   jEnv->ReleaseLongArrayElements (jlongA, jlA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1stringArray
      (JNIEnv *jEnv, jobject jThs, jobjectArray jstringA)
{
   CIMValue     *cv   = NULL;
   jsize         len  = jEnv->GetArrayLength (jstringA);
   Array<String> strA;

   for (jsize i = 0;i < len; i++)
   {
      jstring     jsA = (jstring)jEnv->GetObjectArrayElement (jstringA, i);
      const char *str = jEnv->GetStringUTFChars (jsA, NULL);

      strA.append (String (str));

      jEnv->ReleaseStringUTFChars (jsA, str);
   }

   cv = new CIMValue (strA);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1booleanArray
      (JNIEnv *jEnv, jobject jThs, jbooleanArray jboolA)
{
   CIMValue       *cv  = NULL;
   jboolean        b;
   jsize           len = jEnv->GetArrayLength (jboolA);
   jboolean       *jbA = jEnv->GetBooleanArrayElements (jboolA, &b);
   Array<Boolean>  bA;

   for (jsize i = 0; i < len; i++)
      bA.append ((Boolean)jbA[i]);

   cv = new CIMValue (bA);

   jEnv->ReleaseBooleanArrayElements (jboolA, jbA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1floatArray
  (JNIEnv *jEnv, jobject jThs, jfloatArray jfloatA)
{
   CIMValue     *cv  = NULL;
   jboolean      b;
   jsize         len = jEnv->GetArrayLength (jfloatA);
   jfloat       *jfA = jEnv->GetFloatArrayElements (jfloatA, &b);
   Array<float>  fA;

   for (jsize i = 0; i< len; i++)
      fA.append ((float)jfA[i]);

   cv = new CIMValue (fA);

   jEnv->ReleaseFloatArrayElements (jfloatA, jfA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1doubleArray
  (JNIEnv *jEnv, jobject jThs, jdoubleArray jdoubleA)
{
   CIMValue      *cv  = NULL;
   jboolean       b;
   jsize          len = jEnv->GetArrayLength (jdoubleA);
   jdouble       *jdA = jEnv->GetDoubleArrayElements (jdoubleA, &b);
   Array<double>  dA;

   for (jsize i = 0; i < len; i++)
      dA.append ((double)jdA[i]);

   cv = new CIMValue (dA);

   jEnv->ReleaseDoubleArrayElements (jdoubleA, jdA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1copArray
      (JNIEnv *jEnv, jobject jThs, jlongArray jlongA)
{
   CIMValue             *cv  = NULL;
   jboolean              b;
   jsize                 len = jEnv->GetArrayLength (jlongA);
   jlong                *jiA = jEnv->GetLongArrayElements (jlongA, &b);
   Array<CIMObjectPath>  cA;

   for (jsize i = 0; i < len; i++)
   {
      CIMObjectPath *cop = DEBUG_ConvertJavaToC (jlong, CIMObjectPath*, jiA[i]);
      cA.append (*cop);
   }

   cv = new CIMValue (cA);

   jEnv->ReleaseLongArrayElements (jlongA, jiA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1datetimeArray
      (JNIEnv *jEnv, jobject jThs, jlongArray jlongA)
{
   CIMValue           *cv  = NULL;
   jboolean            b;
   jsize               len = jEnv->GetArrayLength (jlongA);
   jlong              *jiA = jEnv->GetLongArrayElements (jlongA, &b);
   Array<CIMDateTime>  cA;

   for (jsize i = 0; i < len; i++)
   {
      CIMDateTime *cdt = DEBUG_ConvertJavaToC (jlong, CIMDateTime*, jiA[i]);
      cA.append (*cdt);
   }

   cv = new CIMValue (cA);

   jEnv->ReleaseLongArrayElements (jlongA, jiA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1objectArray
      (JNIEnv *jEnv, jobject jThs, jlongArray jlongA)
{
   CIMValue           *cv  = NULL;
   jboolean            b;
   jsize               len = jEnv->GetArrayLength (jlongA);
   jlong              *jiA = jEnv->GetLongArrayElements (jlongA, &b);
   Array<CIMObject>    cA;

   for (jsize i = 0; i < len; i++)
   {
      CIMObject *co = DEBUG_ConvertJavaToC (jlong, CIMObject*, jiA[i]);
      cA.append (*co);
   }

   cv = new CIMValue (cA);

   jEnv->ReleaseLongArrayElements (jlongA, jiA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_CIMValue__1char16Array
      (JNIEnv *jEnv, jobject jThs, jcharArray jcharA)
{
   CIMValue      *cv  = NULL;
   jboolean       b;
   jsize          len = jEnv->GetArrayLength (jcharA);
   jchar         *jcA = jEnv->GetCharArrayElements (jcharA, &b);
   Array<Char16>  cA;

   for (jsize i = 0; i < len; i++)
      cA.append (Char16 (jcA[i]));

   cv = new CIMValue (cA);

   jEnv->ReleaseCharArrayElements (jcharA, jcA, 0);

   return DEBUG_ConvertCToJava (CIMValue*, jlong, cv);
}

JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_CIMValue__1getValue
      (JNIEnv *jEnv, jobject jThs, jlong jV)
{
   JMPIjvm::cacheIDs (jEnv);

   CIMValue *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);

   if (cv->isNull ())
      return NULL;

   CIMType type = cv->getType ();

   if (!cv->isArray ())
   {
      switch (type)
      {
      case CIMTYPE_BOOLEAN:
      {
         Boolean bo;
         cv->get (bo);
         return jEnv->NewObject (JMPIjvm::jv.BooleanClassRef,
                                 JMPIjvm::jv.BooleanNewZ,
                                 (jboolean)bo);
      }
      case CIMTYPE_SINT8:
      {
         Sint8 s8;
         cv->get (s8);
         return jEnv->NewObject (JMPIjvm::jv.ByteClassRef,
                                 JMPIjvm::jv.ByteNewB,
                                 (jbyte)s8);
      }
      case CIMTYPE_UINT8:
      {
         Uint8 u8;
         cv->get (u8);
         return jEnv->NewObject (JMPIjvm::jv.UnsignedInt8ClassRef,
                                 JMPIjvm::jv.UnsignedInt8NewS,
                                 (jshort)u8);
      }
      case CIMTYPE_SINT16:
      {
         Sint16 s16;
         cv->get (s16);
         return jEnv->NewObject (JMPIjvm::jv.ShortClassRef,
                                 JMPIjvm::jv.ShortNewS,
                                 (jshort)s16);
      }
      case CIMTYPE_UINT16:
      {
         Uint16 u16;
         cv->get (u16);
         return jEnv->NewObject (JMPIjvm::jv.UnsignedInt16ClassRef,
                                 JMPIjvm::jv.UnsignedInt16NewI,
                                 (jint)u16);
      }
      case CIMTYPE_SINT32:
      {
         Sint32 s32;
         cv->get (s32);
         return jEnv->NewObject (JMPIjvm::jv.IntegerClassRef,
                                 JMPIjvm::jv.IntegerNewI,
                                 (jint)s32);
      }
      case CIMTYPE_UINT32:
      {
         Uint32 u32;
         cv->get (u32);
         return jEnv->NewObject (JMPIjvm::jv.UnsignedInt32ClassRef,
                                 JMPIjvm::jv.UnsignedInt32NewJ,
                                 (jlong)u32);
      }
      case CIMTYPE_SINT64:
      {
         Sint64 s64;
         cv->get (s64);
         return jEnv->NewObject (JMPIjvm::jv.LongClassRef,
                                 JMPIjvm::jv.LongNewJ,
                                 (jlong)s64);
      }
      case CIMTYPE_UINT64:
      {
         Uint64 u64;

         cv->get (u64);

         std::ostringstream oss;
         jstring            jString = 0;
         jobject            jBIG    = 0;

         oss << u64 << ends;

         jString = jEnv->NewStringUTF (oss.str ().c_str ());

         if (jString)
         {
            jBIG = jEnv->NewObject (JMPIjvm::jv.UnsignedInt64ClassRef,
                                    JMPIjvm::jv.UnsignedInt64NewStr,
                                    jString);
         }

         return jEnv->NewObject (JMPIjvm::jv.UnsignedInt64ClassRef,
                                 JMPIjvm::jv.UnsignedInt64NewBi,
                                 jBIG);
      }
      case CIMTYPE_REAL32:
      {
         float f;
         cv->get (f);
         return jEnv->NewObject (JMPIjvm::jv.FloatClassRef,
                                 JMPIjvm::jv.FloatNewF,
                                 (jfloat)f);
      }
      case CIMTYPE_REAL64:
      {
         double d;
         cv->get (d);
         return jEnv->NewObject (JMPIjvm::jv.DoubleClassRef,
                                 JMPIjvm::jv.DoubleNewD,
                                 (jdouble)d);
      }
      case CIMTYPE_STRING:
      {
         String s;
         cv->get (s);
         return jEnv->NewStringUTF (s.getCString ());
      }
      case CIMTYPE_REFERENCE:
      {
         CIMObjectPath ref;
         cv->get (ref);
         jlong jOp = DEBUG_ConvertCToJava(
                         CIMObjectPath*,
                         jlong,
                         new CIMObjectPath (ref));
         return jEnv->NewObject(
                          JMPIjvm::jv.CIMObjectPathClassRef,
                          JMPIjvm::jv.CIMObjectPathNewJ,
                          jOp);
      }
      case CIMTYPE_CHAR16:
      {
         Char16 c16;
         cv->get (c16);
         return jEnv->NewObject (JMPIjvm::jv.CharacterClassRef,
                                 JMPIjvm::jv.CharacterNewC,
                                 (jchar)c16);
      }
      case CIMTYPE_DATETIME:
      {
         CIMDateTime dt;
         cv->get (dt);
         jlong jDT = DEBUG_ConvertCToJava(
                         CIMDateTime*, jlong,
                         new CIMDateTime(dt));
         return jEnv->NewObject (JMPIjvm::jv.CIMDateTimeClassRef,
                                 JMPIjvm::jv.CIMDateTimeNewJ,
                                 jDT);
      }
      case CIMTYPE_OBJECT:
      {
         CIMObject co;
         cv->get (co);
         if (co.isClass ())
         {
            jlong jCC = DEBUG_ConvertCToJava(
                            CIMClass*,
                            jlong,
                            new CIMClass (co));

            return jEnv->NewObject (JMPIjvm::jv.CIMObjectClassRef,
                                    JMPIjvm::jv.CIMObjectNewJZ,
                                    jCC,
                                    (jboolean)true);
         }
         else
         {
            jlong jCI = DEBUG_ConvertCToJava(
                            CIMInstance*,
                            jlong,
                            new CIMInstance (co));

            return jEnv->NewObject (JMPIjvm::jv.CIMObjectClassRef,
                                    JMPIjvm::jv.CIMObjectNewJZ,
                                    jCI,
                                    (jboolean)false);
         }
      }
      default:
         throwCIMException (jEnv,"+++ unsupported type: ");
      }
   }
   else
   {
      switch (type)
      {
      case CIMTYPE_BOOLEAN:
      {
         Array<Boolean> bo;

         cv->get (bo);

         int          s         = bo.size ();
         jobjectArray jbooleanA = (jobjectArray)jEnv->NewObjectArray(
                                                   s,
                                                   JMPIjvm::jv.BooleanClassRef,
                                                   0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jbooleanA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.BooleanClassRef,
                    JMPIjvm::jv.BooleanNewZ,
                    (jboolean)bo[i]));
         }
         return jbooleanA;
      }
      case CIMTYPE_SINT8:
      {
         Array<Sint8> s8;

         cv->get (s8);

         int          s      = s8.size ();
         jobjectArray jbyteA = (jobjectArray)jEnv->NewObjectArray(
                                                s,
                                                JMPIjvm::jv.ByteClassRef,
                                                0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jbyteA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.ByteClassRef,
                    JMPIjvm::jv.ByteNewB,
                    (jbyte)s8[i]));
         }
         return jbyteA;
      }
      case CIMTYPE_UINT8:
      {
         Array<Uint8> u8;

         cv->get (u8);

         int          s       = u8.size ();
         jobjectArray jshortA = (jobjectArray)jEnv->NewObjectArray(
                                               s,
                                               JMPIjvm::jv.UnsignedInt8ClassRef,
                                               0);
         for (int i = 0; i < s; i++)
         {
             jEnv->SetObjectArrayElement(
                 jshortA,
                 i,
                 jEnv->NewObject(
                     JMPIjvm::jv.UnsignedInt8ClassRef,
                     JMPIjvm::jv.UnsignedInt8NewS,
                     (jshort)u8[i]));
         }
         return jshortA;
      }
      case CIMTYPE_SINT16:
      {
         Array<Sint16> s16;

         cv->get (s16);

         int          s       = s16.size ();
         jobjectArray jshortA = (jobjectArray)jEnv->NewObjectArray(
                                     s,
                                     JMPIjvm::jv.ShortClassRef,
                                     0);

         for (int i = 0; i < s; i++)
         {
             jEnv->SetObjectArrayElement(
                 jshortA,
                 i,
                 jEnv->NewObject(
                     JMPIjvm::jv.ShortClassRef,
                     JMPIjvm::jv.ShortNewS,
                     (jshort)s16[i]));
         }
         return jshortA;
      }
      case CIMTYPE_UINT16:
      {
         Array<Uint16> u16;

         cv->get (u16);

         int          s     = u16.size ();
         jobjectArray jintA = (jobjectArray)jEnv->NewObjectArray(
                                  s,
                                  JMPIjvm::jv.UnsignedInt16ClassRef,
                                  0);

         for (int i = 0; i < s; i++)
         {
             jEnv->SetObjectArrayElement(
                 jintA,
                 i,
                 jEnv->NewObject(
                     JMPIjvm::jv.UnsignedInt16ClassRef,
                     JMPIjvm::jv.UnsignedInt16NewI,
                     (jint)u16[i]));
         }
         return jintA;
      }
      case CIMTYPE_SINT32:
      {
         Array<Sint32> s32;

         cv->get (s32);

         int          s     = s32.size ();
         jobjectArray jintA = (jobjectArray)jEnv->NewObjectArray(
                                  s,
                                  JMPIjvm::jv.IntegerClassRef,
                                  0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jintA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.IntegerClassRef,
                    JMPIjvm::jv.IntegerNewI,
                    (jint)s32[i]));
         }
         return jintA;
      }
      case CIMTYPE_UINT32:
      {
         Array<Uint32> u32;
         cv->get (u32);
         int          s      = u32.size ();
         jobjectArray jlongA = (jobjectArray)jEnv->NewObjectArray(
                                   s,
                                   JMPIjvm::jv.UnsignedInt32ClassRef,
                                   0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jlongA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.UnsignedInt32ClassRef,
                    JMPIjvm::jv.UnsignedInt32NewJ,
                    (jlong)u32[i]));
         }
         return jlongA;
      }
      case CIMTYPE_SINT64:
      {
         Array<Sint64> s64;

         cv->get (s64);

         int          s      = s64.size ();
         jobjectArray jlongA = (jobjectArray)jEnv->NewObjectArray(
                                   s,
                                   JMPIjvm::jv.LongClassRef,
                                   0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jlongA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.LongClassRef,
                    JMPIjvm::jv.LongNewJ,
                    (jlong)s64[i]));
         }
         return jlongA;
      }
      case CIMTYPE_UINT64:
      {
         Array<Uint64> u64;
         cv->get (u64);
         int          s     = u64.size ();
         jobjectArray ju64A = (jobjectArray)jEnv->NewObjectArray(
                                  s,
                                  JMPIjvm::jv.UnsignedInt64ClassRef,
                                  0);

         for (int i = 0; i < s; i++)
         {
            std::ostringstream oss;
            jstring            jString = 0;
            jobject            jBIG    = 0;

            oss << u64[i] << ends;

            jString = jEnv->NewStringUTF (oss.str ().c_str ());

            if (jString)
            {
               jBIG = jEnv->NewObject(
                          JMPIjvm::jv.UnsignedInt64ClassRef,
                          JMPIjvm::jv.UnsignedInt64NewStr,
                          jString);
            }

            if (jBIG)
            {
               jEnv->SetObjectArrayElement(
                   ju64A,
                   i,
                   jEnv->NewObject(
                       JMPIjvm::jv.UnsignedInt64ClassRef,
                       JMPIjvm::jv.UnsignedInt64NewBi,
                       jBIG));
            }
         }
         return ju64A;
      }
      case CIMTYPE_REAL32:
      {
         Array<Real32> r32;
         cv->get (r32);
         int          s       = r32.size ();
         jobjectArray jfloatA = (jobjectArray)jEnv->NewObjectArray(
                                    s,
                                    JMPIjvm::jv.FloatClassRef,
                                    0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jfloatA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.FloatClassRef,
                    JMPIjvm::jv.FloatNewF,
                    (jfloat)r32[i]));
         }
         return jfloatA;
      }
      case CIMTYPE_REAL64:
      {
         Array<Real64> r64;

         cv->get (r64);

         int          s        = r64.size ();
         jobjectArray jdoubleA = (jobjectArray)jEnv->NewObjectArray(
                                     s,
                                     JMPIjvm::jv.DoubleClassRef,
                                     0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jdoubleA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.DoubleClassRef,
                    JMPIjvm::jv.DoubleNewD,
                    (jdouble)r64[i]));
         }
         return jdoubleA;
      }
      case CIMTYPE_STRING:
      {
         Array<String> str;
         cv->get(str);
         int          s        = str.size ();
         jobjectArray jstringA = (jobjectArray)jEnv->NewObjectArray(
                                     s,
                                     JMPIjvm::jv.StringClassRef,
                                     0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jstringA,
                i,
                jEnv->NewStringUTF (str[i].getCString ()));
         }
         return jstringA;
      }
      case CIMTYPE_REFERENCE:
      {
         Array<CIMObjectPath> ref;

         cv->get (ref);

         int          s     = ref.size ();
         jobjectArray jrefA = (jobjectArray)jEnv->NewObjectArray(
                                  s,
                                  JMPIjvm::jv.CIMObjectPathClassRef,
                                  0);

         for (int i = 0; i < s; i++)
         {
            jlong jOP = DEBUG_ConvertCToJava(
                            CIMObjectPath*,
                            jlong,
                            new CIMObjectPath(ref[i]));

            jEnv->SetObjectArrayElement (jrefA,
                                         i,
                                         jEnv->NewObject(
                                             JMPIjvm::jv.CIMObjectPathClassRef,
                                             JMPIjvm::jv.CIMObjectPathNewJ,
                                             jOP));
         }
         return jrefA;
      }
      case CIMTYPE_CHAR16:
      {
         Array<Char16> c16;

         cv->get (c16);

         int          s     = c16.size ();
         jobjectArray jc16A = (jobjectArray)jEnv->NewObjectArray(
                                  s,
                                  JMPIjvm::jv.CharacterClassRef,
                                  0);

         for (int i = 0; i < s; i++)
         {
            jEnv->SetObjectArrayElement(
                jc16A,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.CharacterClassRef,
                    JMPIjvm::jv.CharacterNewC,
                    (jchar)c16[i]));
         }
         return jc16A;
      }
      case CIMTYPE_DATETIME:
      {
         Array<CIMDateTime> dt;

         cv->get (dt);

         int          s    = dt.size ();
         jobjectArray jdtA = (jobjectArray)jEnv->NewObjectArray(
                                 s,
                                 JMPIjvm::jv.CIMDateTimeClassRef,
                                 0);

         for (int i = 0; i < s; i++)
         {
            jlong jDT = DEBUG_ConvertCToJava(
                            CIMDateTime*,
                            jlong,
                            new CIMDateTime (dt[i]));

            jEnv->SetObjectArrayElement(
                jdtA,
                i,
                jEnv->NewObject(
                    JMPIjvm::jv.CIMDateTimeClassRef,
                    JMPIjvm::jv.CIMDateTimeNewJ,
                    jDT));
         }
         return jdtA;
      }
      case CIMTYPE_OBJECT:
      {
         Array<CIMObject> co;

         cv->get (co);

         int          s    = co.size ();
         jobjectArray jcoA = (jobjectArray)jEnv->NewObjectArray(
                                 s,
                                 JMPIjvm::jv.CIMObjectClassRef,
                                 0);

         for (int i = 0; i < s; i++)
         {
            if (co[i].isClass ())
            {
               jlong jCC = DEBUG_ConvertCToJava(
                               CIMClass*,
                               jlong,
                               new CIMClass (co[i]));

               jEnv->SetObjectArrayElement(
                   jcoA,
                   i,
                   jEnv->NewObject(
                       JMPIjvm::jv.CIMObjectClassRef,
                       JMPIjvm::jv.CIMObjectNewJZ,
                       jCC,
                       (jboolean)true));
            }
            else
            {
               jlong jCI = DEBUG_ConvertCToJava(
                               CIMInstance*,
                               jlong,
                               new CIMInstance (co[i]));

               jEnv->SetObjectArrayElement(
                   jcoA,
                   i,
                   jEnv->NewObject(
                       JMPIjvm::jv.CIMObjectClassRef,
                       JMPIjvm::jv.CIMObjectNewJZ,
                       jCI,
                       (jboolean)false));
            }
         }
         return jcoA;
      }
      default:
         throwCIMException (jEnv,"+++ unsupported type: ");
      }
   }

   return NULL;
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_CIMValue__1getType
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMValue *cv       = DEBUG_ConvertJavaToC (jlong, CIMValue*, jP);
   bool      fSuccess = false;
   jint      jiJType  = 0;

   jiJType = _dataType::convertCTypeToJavaType (cv->getType (), &fSuccess);

   return jiJType;
}

JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_CIMValue__1isArray
      (JNIEnv *jEnv, jobject jThs, jlong jP)
{
   CIMValue *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jP);

   return (jboolean)cv->isArray ();
}

JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_CIMValue__1toString
      (JNIEnv *jEnv, jobject jThs, jlong jV)
{
   CIMValue *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);

   return (jstring)jEnv->NewStringUTF (cv->toString ().getCString ());
}

JNIEXPORT void JNICALL Java_org_pegasus_jmpi_CIMValue__1finalize
      (JNIEnv *jEnv, jobject jThs, jlong jV)
{
   CIMValue *cv = DEBUG_ConvertJavaToC (jlong, CIMValue*, jV);

   delete cv;

   DEBUG_ConvertCleanup (jlong, jV);
}


// -------------------------------------
// ---
// -        ClassEnumeration
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_ClassEnumeration__1getClass
  (JNIEnv *jEnv, jobject jThs, jlong jEnum, jint pos)
{
   Array<CIMClass> *enm = DEBUG_ConvertJavaToC(jlong, Array<CIMClass>*, jEnum);

   return DEBUG_ConvertCToJava (CIMClass*, jlong, new CIMClass ((*enm)[pos]));
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_ClassEnumeration__1size
  (JNIEnv *jEnv, jobject jThs, jlong jEnum)
{
   Array<CIMClass> *enm = DEBUG_ConvertJavaToC(jlong, Array<CIMClass>*, jEnum);

   return enm->size ();
}


// -------------------------------------
// ---
// -        InstEnumeration
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_InstEnumeration__1getInstance
  (JNIEnv *jEnv, jobject jThs, jlong jEnum, jint pos)
{
   Array<CIMInstance> *enm = DEBUG_ConvertJavaToC(
                                 jlong,
                                 Array<CIMInstance>*,
                                 jEnum);

   return DEBUG_ConvertCToJava(
              CIMInstance*,
              jlong,
              new CIMInstance((*enm)[pos]));
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_InstEnumeration__1size
  (JNIEnv *jEnv, jobject jThs, jlong jEnum)
{
   Array<CIMInstance> *enm = DEBUG_ConvertJavaToC(
                                 jlong,
                                 Array<CIMInstance>*,
                                 jEnum);

   return enm->size ();
}

// -------------------------------------
// ---
// -        JMPIQueryExp
// ---
// -------------------------------------

/*
 * Class:     org_pegasus_jmpi_JMPIQueryExp
 * Method:    _applyInstance
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_org_pegasus_jmpi_JMPIQueryExp__1applyInstance
  (JNIEnv *jEnv, jobject jThs, jlong jWQLStmt, jlong jciInstance)
{
   WQLSelectStatement *wql_stmt = DEBUG_ConvertJavaToC(
                                      jlong,
                                      WQLSelectStatement*,
                                      jWQLStmt);
   CIMInstance        *ci       = DEBUG_ConvertJavaToC(
                                      jlong,
                                      CIMInstance*,
                                      jciInstance);

   if (  !wql_stmt
      || !ci
      )
   {
      return 0;
   }

   try
   {
      return wql_stmt->evaluate (*ci);
   }
   catch (const Exception &e)
   {
      cerr << "Java_org_pegasus_jmpi_JMPIQueryExp__1applyInstance: Caught: "
           << e.getMessage () << endl;

      return false;
   }
}


// -------------------------------------
// ---
// -        JMPISelectList
// ---
// -------------------------------------

/*
 * Class:     org_pegasus_jmpi_JMPISelectList
 * Method:    _applyInstance
 * Signature: (II)I
 */
JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_JMPISelectList__1applyInstance
  (JNIEnv *jEnv, jobject jThs, jlong jWQLStmt, jlong jciInstance)
{
   WQLSelectStatement *wql_stmt = DEBUG_ConvertJavaToC(
                                      jlong,
                                      WQLSelectStatement*,
                                      jWQLStmt);
   CIMInstance        *ci       = DEBUG_ConvertJavaToC(
                                      jlong,
                                      CIMInstance*,
                                      jciInstance);
   CIMInstance        *ciRet    = 0;

   if (  !wql_stmt
      || !ci
      )
   {
      return 0;
   }

   ciRet = new CIMInstance (ci->clone ());

   if (ciRet)
   {
      try
      {
         wql_stmt->applyProjection (*ciRet, false);
      }
      catch (const Exception &e)
      {
         cerr << "Java_org_pegasus_jmpi_JMPISelectList__1applyInstance:"
                     " Caught: "
              << e.getMessage () << endl;

         return 0;
      }
   }

   return DEBUG_ConvertCToJava (CIMInstance *, jlong, ciRet);
}

/*
 * Class:     org_pegasus_jmpi_JMPISelectList
 * Method:    _applyClass
 * Signature: (II)I
 */
JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_JMPISelectList__1applyClass
  (JNIEnv *jEnv, jobject jThs, jlong jWQLStmt, jlong jciClass)
{
   WQLSelectStatement *wql_stmt = DEBUG_ConvertJavaToC(
                                      jlong,
                                      WQLSelectStatement*,
                                      jWQLStmt);
   CIMClass           *cc       = DEBUG_ConvertJavaToC(
                                      jlong,
                                      CIMClass*,
                                      jciClass);
   CIMClass           *ccRet    = NULL;

   if (!wql_stmt)
   {
      return 0;
   }

   if (cc)
   {
      CIMObject co (cc->clone ());

      try
      {
         wql_stmt->applyProjection (co, false);
      }
      catch (const Exception &e)
      {
         cerr << "Java_org_pegasus_jmpi_JMPISelectList__1applyClass: Caught: "
              << e.getMessage () << endl;

         return 0;
      }

      ccRet = new CIMClass (co);

      return DEBUG_ConvertCToJava (CIMClass *, jlong, ccRet);
   }

   return 0;
}


// -------------------------------------
// ---
// -        OperationContext
// ---
// -------------------------------------

/*
 * Class:     OperationContext
 * Method:    _get
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_org_pegasus_jmpi_OperationContext__1get
  (JNIEnv *jEnv, jobject jThs, jlong jInst, jstring jContainer, jstring jKey)
{
   OperationContext *poc  = DEBUG_ConvertJavaToC(
                                jlong,
                                OperationContext*,
                                jInst);
   jobject           jRet = 0;

   if (!poc)
   {
      return jRet;
   }

   const char *pszContainer = jEnv->GetStringUTFChars (jContainer, NULL);
   const char *pszKey       = jEnv->GetStringUTFChars (jKey, NULL);
   String      container (pszContainer);
   String      key       (pszKey);

   try {
      if (container == "IdentityContainer")
      {
         IdentityContainer ic = poc->get(IdentityContainer::NAME);
         if (key == "userName")
         {
            String userName = ic.getUserName();
            jRet = jEnv->NewStringUTF((const char*)userName.getCString());
         }
      }
      else if (container == "SubscriptionInstanceContainer")
      {
         SubscriptionInstanceContainer sic =
             poc->get(SubscriptionInstanceContainer::NAME);

         if (key == "subscriptionInstance")
         {
            CIMInstance ci     = sic.getInstance ();
            jlong       jciRef = DEBUG_ConvertCToJava(
                                     CIMInstance*,
                                     jlong,
                                     new CIMInstance (ci));

            jRet = jEnv->NewObject (JMPIjvm::jv.CIMInstanceClassRef,
                                    JMPIjvm::jv.CIMInstanceNewJ,
                                    jciRef);
         }
      }
      else if (container == "SubscriptionInstanceNamesContainer")
      {
         SubscriptionInstanceNamesContainer sinc=
             poc->get (SubscriptionInstanceNamesContainer::NAME);

         if (key == "subscriptionInstanceNames")
         {
            Array<CIMObjectPath> copa = sinc.getInstanceNames ();
            jobjectArray         jcopa = 0;
            int                  jcopaLength = copa.size ();

            jcopa = (jobjectArray)jEnv->NewObjectArray(
                        jcopaLength,
                        JMPIjvm::jv.CIMObjectPathClassRef,
                        0);

            for (int i = 0; i < jcopaLength; i++)
            {
               jEnv->SetObjectArrayElement(
                   jcopa,
                   i,
                   jEnv->NewObject(
                       JMPIjvm::jv.CIMObjectPathClassRef,
                       JMPIjvm::jv.CIMObjectPathNewJ,
                       new CIMObjectPath(copa[i])));
            }

            jRet = (jobject)jcopa;
         }
      }
      else if (container == "SubscriptionFilterConditionContainer")
      {
         SubscriptionFilterConditionContainer sfcc =
             poc->get (SubscriptionFilterConditionContainer::NAME);

         if (key == "filterCondition")
         {
            String filterCondition = sfcc.getFilterCondition ();

            jRet=jEnv->NewStringUTF((const char*)filterCondition.getCString());
         }
         else if (key == "queryLanguage")
         {
            String queryLanguage = sfcc.getQueryLanguage ();

            jRet=jEnv->NewStringUTF((const char*)queryLanguage.getCString ());
         }
      }
      else if (container == "SubscriptionFilterQueryContainer")
      {
         SubscriptionFilterQueryContainer sfqc =
             poc->get(SubscriptionFilterQueryContainer::NAME);

         if (key == "filterQuery")
         {
            String filterQuery = sfqc.getFilterQuery ();

            jRet = jEnv->NewStringUTF ((const char*)filterQuery.getCString ());
         }
         else if (key == "queryLanguage")
         {
            String queryLanguage = sfqc.getQueryLanguage ();

            jRet = jEnv->NewStringUTF((const char*)queryLanguage.getCString());
         }
         else if (key == "sourceNameSpace")
         {
            CIMNamespaceName cimNameSpaceName = sfqc.getSourceNameSpace();
            String           nameSpaceName    = cimNameSpaceName.getString();

            jRet = jEnv->NewStringUTF((const char*)nameSpaceName.getCString());
         }
      }
      else if (container == "SnmpTrapOidContainer")
      {
         SnmpTrapOidContainer stoc = poc->get (SnmpTrapOidContainer::NAME);

         if (key == "snmpTrapOid")
         {
            String snmpTrapOid = stoc.getSnmpTrapOid ();

            jRet = jEnv->NewStringUTF ((const char*)snmpTrapOid.getCString ());
         }
      }
   }
   Catch (jEnv);

///printf ("jRet: %08X\n", (int)jRet);

   jEnv->ReleaseStringUTFChars (jContainer, pszContainer);
   jEnv->ReleaseStringUTFChars (jKey, pszKey);

   return jRet;
}


// -------------------------------------
// ---
// -        PathEnumeration
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_PathEnumeration__1getObjectPath
  (JNIEnv *jEnv, jobject jThs, jlong jEnum, jint pos)
{
   Array<CIMObjectPath> *enm = DEBUG_ConvertJavaToC(
                                   jlong,
                                   Array<CIMObjectPath>*,
                                   jEnum);

   return DEBUG_ConvertCToJava(
              CIMObjectPath*,
              jlong,
              new CIMObjectPath((*enm)[pos]));
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_PathEnumeration__1size
  (JNIEnv *jEnv, jobject jThs, jlong jEnum)
{
   Array<CIMObjectPath> *enm = DEBUG_ConvertJavaToC(
                                   jlong,
                                   Array<CIMObjectPath>*,
                                   jEnum);

   return enm->size ();
}


// -------------------------------------
// ---
// -        QualEnumeration
// ---
// -------------------------------------

JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_QualEnumeration__1getQualifierType
  (JNIEnv *jEnv, jobject jThs, jlong jEnum, jint pos)
{
   Array<CIMQualifierDecl> *enm = DEBUG_ConvertJavaToC(
                                      jlong,
                                      Array<CIMQualifierDecl>*,
                                      jEnum);

   return DEBUG_ConvertCToJava(
              CIMQualifierDecl*,
              jlong,
              new CIMQualifierDecl((*enm)[pos]));
}

JNIEXPORT jint JNICALL Java_org_pegasus_jmpi_QualEnumeration__1size
  (JNIEnv *jEnv, jobject jThs, jlong jEnum)
{
   Array<CIMQualifierDecl> *enm = DEBUG_ConvertJavaToC(
                                      jlong,
                                      Array<CIMQualifierDecl>*,
                                      jEnum);

   return enm->size ();
}


// -------------------------------------
// ---
// -        SelectExp
// ---
// -------------------------------------

/*
 * Class:     org_pegasus_jmpi_SelectExp
 * Method:    _finalize
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_pegasus_jmpi_SelectExp__1finalize
  (JNIEnv *jEnv, jobject jThs, jlong jWQLStmt)
{
   WQLSelectStatement *wql_stmt = DEBUG_ConvertJavaToC (
                                      jlong,
                                      WQLSelectStatement*,
                                      jWQLStmt);

   delete wql_stmt;

   DEBUG_ConvertCleanup (jlong, jWQLStmt);
}

/*
 * Class:     org_pegasus_jmpi_SelectExp
 * Method:    _newSelectExp
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jlong JNICALL Java_org_pegasus_jmpi_SelectExp__1newSelectExp
  (JNIEnv *jEnv, jobject jThs, jstring jQuery)
{
   const char         *pszQuery = jEnv->GetStringUTFChars (jQuery, NULL);
   WQLSelectStatement *wql_stmt = NULL;
   String              queryLanguage (CALL_SIGN_WQL);
   String              query (pszQuery);

   wql_stmt = new WQLSelectStatement (queryLanguage, query);

   PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
       "Java_org_pegasus_jmpi_SelectExp__1newSelectExp: wql_stmt = %p",
       wql_stmt));

   try
   {
      WQLParser::parse (query, *wql_stmt);
   }
   catch (const Exception &e)
   {
      PEG_TRACE((TRC_PROVIDERMANAGER,Tracer::LEVEL4,
          "Java_org_pegasus_jmpi_SelectExp__1newSelectExp: Caught: %s",
          (const char*)e.getMessage().getCString()));
   }

   jEnv->ReleaseStringUTFChars (jQuery, pszQuery);

   return DEBUG_ConvertCToJava (WQLSelectStatement *, jlong, wql_stmt);
}

/*
 * Class:     org_pegasus_jmpi_SelectExp
 * Method:    _getSelectString
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_pegasus_jmpi_SelectExp__1getSelectString
  (JNIEnv *jEnv, jobject jThs, jlong jWQLStmt)
{
   WQLSelectStatement *wql_stmt = DEBUG_ConvertJavaToC(
                                      jlong,
                                      WQLSelectStatement*,
                                      jWQLStmt);
   String              cond;

   if (wql_stmt)
   {
      try
      {
         cond = wql_stmt->getQuery ();
      }
      catch (const Exception &e)
      {
         cerr << "Java_org_pegasus_jmpi_SelectExp__1getSelectString: Caught: "
              << e.getMessage () << endl;

         cond = "";
      }
   }
   else
   {
      cond = "";
   }

   return (jstring)jEnv->NewStringUTF (cond.getCString ());
}

} // extern "C"

PEGASUS_NAMESPACE_END
