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

#include "Convert.h"
#include "JMPIImpl.h"
#include "JMPIProvider.h"
#include "JMPIProviderManager.h"

#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>
#include <Pegasus/Common/CIMProperty.h>
#include <Pegasus/Common/CIMParamValue.h>
#include <Pegasus/Config/ConfigManager.h>
#include <Pegasus/Common/FileSystem.h>

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;

int trace = 0;

#ifdef PEGASUS_DEBUG
#define DDD(x) if (trace) x;
#else
#define DDD(x)
#endif

#ifdef PEGASUS_OS_TYPE_WINDOWS
static LPCSTR g_cimservice_key=TEXT("SYSTEM\\CurrentControlSet\\Services\\%s");

static bool _getRegInfo(const char *lpchKeyword, char *lpchRetValue)
{
  HKEY   hKey;
  DWORD  dw                   = _MAX_PATH;
  char   subKey[_MAX_PATH]    = {0};

  sprintf(subKey, g_cimservice_key, "cimserver");

  if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    subKey,
                    0,
                    KEY_READ,
                    &hKey)) != ERROR_SUCCESS)
    {
      return false;
    }

  if ((RegQueryValueEx(hKey,
                       lpchKeyword,
                       NULL,
                       NULL,
                       (LPBYTE)lpchRetValue,
                       &dw)) != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return false;
    }

  RegCloseKey(hKey);

  return true;
}
#endif

void
printEnvironmentVariables ()
{
   const char *apszEnvVariables[] = {
      "PEGASUS_HOME",
      "PEGASUS_ROOT",
      "PEGASUS_PLATFORM",
      "PEGASUS_DEBUG",
      "PEGASUS_TEST_VERBOSE",
      "PATH",
      "LD_LIBRARY_PATH",
      "CLASSPATH",
      "JAVA_SDK",
      "JAVA_SDKINC"
   };

   for (size_t i=0;i<sizeof(apszEnvVariables)/sizeof(apszEnvVariables[0]);i++)
   {
      const char *pszValue = 0;

      pszValue = getenv (apszEnvVariables[i]);

      if (pszValue)
      {
         printf ("%s = \"%s\"\n", apszEnvVariables[i], pszValue);
      }
      else
      {
         printf ("%s = NULL\n", apszEnvVariables[i]);
      }
   }
}

int testJVM ()
{
   JvmVector  *jv   = NULL;
   JNIEnv     *jEnv = NULL;

   jEnv = JMPIjvm::attachThread (&jv);

   if (!jEnv)
   {
      PEGASUS_STD(cerr)<<"testJVM: FAILURE: Could not attach a thread!"
                       <<PEGASUS_STD(endl);
      return 1;
   }

   jobject jSI8 = jEnv->NewObject (JMPIjvm::jv.ByteClassRef,
                                   JMPIjvm::jv.ByteNewB,
                                   (jbyte)-42);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Byte"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI8)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Byte"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Byte"
                         << PEGASUS_STD (endl);
   }
   jobject jUI8 = jEnv->NewObject (JMPIjvm::jv.UnsignedInt8ClassRef,
                                   JMPIjvm::jv.UnsignedInt8NewS,
                                   (jshort)16);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt8"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI8)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt8"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt8"
                         << PEGASUS_STD (endl);
   }
   jobject jSI16 = jEnv->NewObject (JMPIjvm::jv.ShortClassRef,
                                    JMPIjvm::jv.ShortNewS,
                                    (jshort)-1578);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Short"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI16)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Short"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Short"
                         << PEGASUS_STD (endl);
   }
   jobject jUI16 = jEnv->NewObject (JMPIjvm::jv.UnsignedInt16ClassRef,
                                    JMPIjvm::jv.UnsignedInt16NewI,
                                    (jint)9831);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt16"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI16)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt16"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt16"
                         << PEGASUS_STD (endl);
   }
   jobject jSI32 = jEnv->NewObject (JMPIjvm::jv.IntegerClassRef,
                                    JMPIjvm::jv.IntegerNewI,
                                    (jint)-45000);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Integer"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI32)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Integer"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Integer"
                         << PEGASUS_STD (endl);
   }
   jobject jUI32 = jEnv->NewObject (JMPIjvm::jv.UnsignedInt32ClassRef,
                                    JMPIjvm::jv.UnsignedInt32NewJ,
                                    (jlong)33000);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt32"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI32)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt32"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt32"
                         << PEGASUS_STD (endl);
   }
   jobject jSI64 = jEnv->NewObject (JMPIjvm::jv.LongClassRef,
                                    JMPIjvm::jv.LongNewJ,
                                    (jlong)-4500000);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Long"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jSI64)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Long"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Long"
                         << PEGASUS_STD (endl);
   }
   // UnsignedInt64NewBi
   jobject jBiStr = jEnv->NewStringUTF ("1234567890");
   jobject jUI64  = jEnv->NewObject (JMPIjvm::jv.UnsignedInt64ClassRef,
                                     JMPIjvm::jv.UnsignedInt64NewStr,
                                     jBiStr);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt64"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jUI64)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create UnsignedInt64"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create UnsignedInt64"
                         << PEGASUS_STD (endl);
   }
   jobject jStr = jEnv->NewStringUTF ("Hello world");
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create String"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jStr)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create String"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create String"
                         << PEGASUS_STD (endl);
   }
   CIMDateTime *cdt = new CIMDateTime (String ("20060227183400.000000-360"));
   jlong jcdt = DEBUG_ConvertCToJava (CIMDateTime *, jlong, cdt);
   jobject jDateTime = jEnv->NewObject(
                           JMPIjvm::jv.CIMDateTimeClassRef,
                           JMPIjvm::jv.CIMDateTimeNewJ,
                           DEBUG_ConvertJavaToC(jlong, CIMDateTime *, jcdt));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMDateTime"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jDateTime)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMDateTime"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMDateTime"
                         << PEGASUS_STD (endl);
   }
   jobject jVector = jEnv->NewObject (JMPIjvm::jv.VectorClassRef,
                                      JMPIjvm::jv.VectorNew);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Vector"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jVector)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Vector"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Vector"
                         << PEGASUS_STD (endl);
   }
   jobject jBoolean = jEnv->NewObject (JMPIjvm::jv.BooleanClassRef,
                                       JMPIjvm::jv.BooleanNewZ,
                                       (jboolean)true);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Boolean"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jBoolean)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Boolean"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Boolean"
                         << PEGASUS_STD (endl);
   }
   jobject jCharacter = jEnv->NewObject (JMPIjvm::jv.CharacterClassRef,
                                         JMPIjvm::jv.CharacterNewC,
                                         (jchar)'A');
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Character"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCharacter)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Character"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Character"
                         << PEGASUS_STD (endl);
   }
   jobject jFloat = jEnv->NewObject (JMPIjvm::jv.FloatClassRef,
                                     JMPIjvm::jv.FloatNewF,
                                     (jfloat)3.1415279);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Float"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jFloat)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Float"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Float"
                         << PEGASUS_STD (endl);
   }
   jobject jDouble = jEnv->NewObject (JMPIjvm::jv.DoubleClassRef,
                                      JMPIjvm::jv.DoubleNewD,
                                      (jdouble)3.1415279);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Double"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jDouble)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create Double"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Double"
                         << PEGASUS_STD (endl);
   }
   Sint8 si8CVInit = -8;
   CIMParamValue *cpv = new CIMParamValue(
                                String ("bob"),
                                CIMValue (si8CVInit));

   jlong jcpv = DEBUG_ConvertCToJava (CIMParamValue *, jlong, cpv);
   jobject jCIMArgument = jEnv->NewObject(
                              JMPIjvm::jv.CIMArgumentClassRef,
                              JMPIjvm::jv.CIMArgumentNewJ,
                              DEBUG_ConvertJavaToC(
                                  jlong,
                                  CIMParamValue *,
                                  jcpv));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMArgument"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMArgument)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMArgument"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMArgument"
                         << PEGASUS_STD (endl);
   }
   CIMClass *cc = new CIMClass (CIMName ("bob"));
   jlong jcc = DEBUG_ConvertCToJava (CIMClass *, jlong, cc);
   jobject jCIMClass = jEnv->NewObject(
                           JMPIjvm::jv.CIMClassClassRef,
                           JMPIjvm::jv.CIMClassNewJ,
                           DEBUG_ConvertJavaToC (jlong, CIMClass *, jcc));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMClass"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMClass)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMClass"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMClass"
                         << PEGASUS_STD (endl);
   }
   jobject jCIMFlavor = jEnv->NewObject (JMPIjvm::jv.CIMFlavorClassRef,
                                         JMPIjvm::jv.CIMFlavorNewI,
                                         1);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMFlavor"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMFlavor)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMFlavor"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMFlavor"
                         << PEGASUS_STD (endl);
   }
   CIMInstance *ci = new CIMInstance (CIMName ("bob"));
   jlong jci = DEBUG_ConvertCToJava (CIMInstance *, jlong, ci);
   jobject jCIMInstance = jEnv->NewObject(
                              JMPIjvm::jv.CIMInstanceClassRef,
                              JMPIjvm::jv.CIMInstanceNewJ,
                              DEBUG_ConvertJavaToC (jlong, CIMInstance *, jci));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMInstance"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMInstance)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMInstance"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMInstance"
                         << PEGASUS_STD (endl);
   }
   CIMClass  *cc2        = new CIMClass (CIMName ("bob"));
   CIMObject *co         = new CIMObject (*cc2);
   jlong      jco        = DEBUG_ConvertCToJava (CIMObject *, jlong, co);
   jobject    jCIMObject = jEnv->NewObject (JMPIjvm::jv.CIMObjectClassRef,
                                            JMPIjvm::jv.CIMObjectNewJZ,
                                            jco,
                                            (jboolean)true);
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObject"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMObject)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObject"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMObject"
                         << PEGASUS_STD (endl);
   }
   CIMObjectPath *cop = new CIMObjectPath (String ("bob"));
   jlong jcop = DEBUG_ConvertCToJava (CIMObjectPath *, jlong, cop);
   jobject jCIMObjectPath = jEnv->NewObject(
                                JMPIjvm::jv.CIMObjectPathClassRef,
                                JMPIjvm::jv.CIMObjectPathNewJ,
                                DEBUG_ConvertJavaToC(
                                    jlong,
                                    CIMObjectPath *,
                                    jcop));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObjectPath"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMObjectPath)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMObjectPath"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMObjectPath"
                         << PEGASUS_STD (endl);
   }
   Uint32 ui32CVInit = 77;
   CIMProperty *cp = new CIMProperty(
                             CIMName("bobprop"),
                             CIMValue (ui32CVInit));
   jlong jcp = DEBUG_ConvertCToJava (CIMProperty *, jlong, cp);
   jobject jCIMProperty = jEnv->NewObject(
                                    JMPIjvm::jv.CIMPropertyClassRef,
                                    JMPIjvm::jv.CIMPropertyNewJ,
                                    DEBUG_ConvertJavaToC(
                                        jlong,
                                        CIMProperty *,
                                        jcp));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMProperty"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMProperty)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMProperty"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMProperty"
                         << PEGASUS_STD (endl);
   }
   Uint64 ui64CVInit = 42;
   CIMQualifier *cq = new CIMQualifier(
                              CIMName("bob"),
                              CIMValue(ui64CVInit));

   jlong jcq = DEBUG_ConvertCToJava (CIMQualifier *, jlong, cq);
   jobject jCIMQualifier = jEnv->NewObject(
                               JMPIjvm::jv.CIMQualifierClassRef,
                               JMPIjvm::jv.CIMQualifierNewJ,
                               DEBUG_ConvertJavaToC(
                                   jlong,
                                   CIMQualifier *,
                                   jcq));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMQualifier"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMQualifier)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMQualifier"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMQualifier"
                         << PEGASUS_STD (endl);
   }
   Uint8 ui8CVInit = 0;
   CIMValue *cv = new CIMValue (ui8CVInit);
   jlong jcv = DEBUG_ConvertCToJava (CIMValue *, jlong, cv);
   jobject jCIMValue = jEnv->NewObject(
                           JMPIjvm::jv.CIMValueClassRef,
                           JMPIjvm::jv.CIMValueNewJ,
                           DEBUG_ConvertJavaToC (jlong, CIMValue *, jcv));
   if (jEnv->ExceptionOccurred ())
   {
      DDD(jEnv->ExceptionDescribe ());
      DDD(jEnv->ExceptionClear ());
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMValue"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else if (!jCIMValue)
   {
      PEGASUS_STD (cout) << "testJVM: FAILURE: Create CIMValue"
                         << PEGASUS_STD (endl);
      return 1;
   }
   else
   {
      PEGASUS_STD (cout) << "testJVM: SUCCESS: Create CIMValue"
                         << PEGASUS_STD (endl);
   }
   // OperationContextNewJ
   // SelectExpNewJ

   // ---------------------------------------------------------------------

   // ---------------------------------------------------------------------

   {
      ConfigManager *manager      = ConfigManager::getInstance ();
      String pegasusHome;

// BEGIN: Copy from src/Server/cimserver.cpp
//        It would have been nice if this were part of ConfigManager.
#ifndef PEGASUS_OS_TYPE_WINDOWS
      //
      // Get environment variables:
      //
#if defined(PEGASUS_OS_AIX) && defined(PEGASUS_USE_RELEASE_DIRS)
      pegasusHome = AIX_RELEASE_PEGASUS_HOME;
#elif defined(PEGASUS_OS_PASE) && defined(PEGASUS_USE_RELEASE_DIRS)
      pegasusHome = PASE_PROD_HOME;
#elif !defined(PEGASUS_USE_RELEASE_DIRS) || defined(PEGASUS_OS_ZOS)
      const char* tmp = getenv("PEGASUS_HOME");

      if (tmp)
      {
         pegasusHome = tmp;
      }
#endif

      FileSystem::translateSlashes(pegasusHome);
#else

      // windows only
      // Determine the absolute path to the running program
      char exe_pathname[_MAX_PATH] = {0};
      char home_pathname[_MAX_PATH] = {0};
      if(0 != GetModuleFileName(NULL, exe_pathname, sizeof(exe_pathname)))
      {

        // Pegasus home search rules:
        // - look in registry (if set)
        // - if not found, look in PEGASUS_HOME (if set)
        // - if not found, use exe directory minus one level

        bool found_reg = _getRegInfo("home", home_pathname);
        if (found_reg == true)
          {
            // Make sure home matches
            String current_home(home_pathname);
            String current_exe(exe_pathname);
            current_home.toLower();
            current_exe.toLower();

            Uint32 pos = current_exe.find(current_home);
            if (pos != PEG_NOT_FOUND)
              {
                pegasusHome = home_pathname;
              }
            else
              {
                found_reg = false;
              }
          }
        if (found_reg == false)
          {
            const char* tmp = getenv("PEGASUS_HOME");
            if (tmp)
              {
                pegasusHome = tmp;
              }
            else
              {
                // ASSUMPTION: At a minimum, the cimserver program is running
                // from a "bin" directory
                pegasusHome = FileSystem::extractFilePath(exe_pathname);
                pegasusHome.remove(pegasusHome.size()-1, 1);
                pegasusHome = FileSystem::extractFilePath(pegasusHome);
                pegasusHome.remove(pegasusHome.size()-1, 1);
              }
          }
      }
#endif

      //
      // Set the value for pegasusHome property
      //
      ConfigManager::setPegasusHome(pegasusHome);
// END

      String path = ConfigManager::getHomedPath(
                        manager->getCurrentValue("providerDir"));
      ProviderVector pv           = { 0, 0 };
      String         providerName;
      String         fileName("JMPIExpAssociationProvider.jar");
      String         className("Associations/JMPIExpAssociationProvider");

      FileSystem::translateSlashes (fileName);

      fileName = JMPIProviderManager::resolveFileName(fileName);

      if (!FileSystem::exists (fileName))
      {
         PEGASUS_STD(cerr)<<"testJVM: FAILURE: \"" << fileName
                          <<"\" does not exist!"<<PEGASUS_STD(endl);

         return 1;
      }
      providerName = fileName + ":" + className;

      pv.jProvider = JMPIjvm::getProvider (jEnv,
                                           fileName,
                                           className,
                                           providerName.getCString (),
                                           &pv.jProviderClass);

      if (  !pv.jProvider
         || !pv.jProviderClass
         )
      {
         PEGASUS_STD (cout) << "testJVM: FAILURE: Create Provider"
                            << PEGASUS_STD (endl);

         return 1;
      }
      else
      {
         PEGASUS_STD (cout) << "testJVM: SUCCESS: Create Provider"
                            << PEGASUS_STD (endl);
      }
   }

   // ---------------------------------------------------------------------

   JMPIjvm::detachThread ();

   jv->jvm->DestroyJavaVM ();

   PEGASUS_STD(cerr)<<"testJVM: SUCCESS"<<PEGASUS_STD(endl);
// DOCHKSRC
   return 0;
}

int
main (int argc, char *argv[])
{
   if (getenv ("PEGASUS_TEST_VERBOSE"))
      trace = 1;
   else
      trace = 0;

   printEnvironmentVariables ();

   return testJVM ();
}
