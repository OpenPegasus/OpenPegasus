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

#ifndef _Provider_JMPIImpl_h
#define _Provider_JMPIImpl_h

#include <Pegasus/Common/Config.h>

#include <jni.h>

#ifdef PEGASUS_PLATFORM_LINUX_GENERIC_GNU
#if defined (__GNUC__) && GCC_VERSION >= 40000
// If gcc is compiled with -fvisibility=hidden then JMPI is broken.
// This is because the JNI h file defines JNIEXPORT as empty since
// the default is visible.
#undef  JNIEXPORT
#define JNIEXPORT __attribute__ ((visibility("default")))
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Pegasus/Common/String.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/HashTable.h>
#include <Pegasus/Common/CIMType.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/ProviderManager2/JMPI/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

#define NULL_CHECK0(e) if ((e) == 0) return 0 //added by Andy
#define NULL_CHECK(e) if ((e) == 0) return //added by Andy

#define Catch(jEnv) \
   catch(CIMException & e) { \
      JMPIjvm::cacheIDs(jEnv); \
      jobject ev=jEnv->NewObject( \
                     JMPIjvm::jv.CIMExceptionClassRef, \
                     JMPIjvm::jv.CIMExceptionNewISt, \
                     (jint)e.getCode(), \
                     jEnv->NewStringUTF(e.getMessage().getCString())); \
      jEnv->Throw((jthrowable)ev); \
   } \
   catch(Exception & e) { \
      JMPIjvm::cacheIDs(jEnv);\
      jobject ev=jEnv->NewObject( \
                     JMPIjvm::jv.CIMExceptionClassRef, \
                     JMPIjvm::jv.CIMExceptionNewISt, \
                     (jint)1, \
                     jEnv->NewStringUTF(e.getMessage().getCString())); \
      jEnv->Throw((jthrowable)ev); \
   } \
   catch(...)  { \
      JMPIjvm::cacheIDs(jEnv); \
      jobject ev=jEnv->NewObject( \
                     JMPIjvm::jv.CIMExceptionClassRef, \
                     JMPIjvm::jv.CIMExceptionNewISt, \
                     (jint)1, \
                     jEnv->NewStringUTF("Exception: Unknown")); \
      jEnv->Throw((jthrowable)ev); \
   }

typedef struct {
  int         clsIndex;
  const char *methodName;
  const char *signature;
} METHOD_STRUCT;

typedef struct jvmVector {
   int                  initRc;
   JavaVM              *jvm;
   JNIEnv              *env;
   jclass              *classRefs;
   jmethodID           *instMethodIDs;
   jmethodID           *staticMethodIDs;
   const METHOD_STRUCT *instanceMethodNames;
} JvmVector;

class PEGASUS_JMPIPM_LINKAGE JMPIjvm {
 public:
   static int trace;
   static JavaVM *jvm;
   static JvmVector jv;

   JMPIjvm();
  ~JMPIjvm();

   static JNIEnv* attachThread(JvmVector **jvp);
   static void detachThread();
   static jobject getProvider(JNIEnv *env, const char *cn, jclass *cls) ;
   static jobject getProvider(
                      JNIEnv *env,
                      String jar,
                      String cln,
                      const char *cn,
                      jclass *cls) ;
   static void checkException(JNIEnv *env);
   static jstring NewPlatformString(JNIEnv *env,char *s);
   static jobjectArray NewPlatformStringArray(
                           JNIEnv *env,
                           char **strv,
                           int strc);
   static int cacheIDs(JNIEnv *env);
   static int destroyJVM();

 private:
   static jclass getGlobalClassRef(JNIEnv *env, const char* name);
   static int initJVM();

   typedef HashTable<
               String,
               jclass,
               EqualFunc<String>,
               HashFunc<String> >  ClassTable;
   typedef HashTable<
               String,
               jobject,
               EqualFunc<String>,
               HashFunc<String> > ObjectTable;

   static ClassTable  _classTable;
   static ObjectTable _objectTable;

   static Mutex _initMutex;
};

class _nameSpace {
 public:
   _nameSpace();
   _nameSpace(String hn);
   _nameSpace(String hn, String ns);

   int port();
   String hostName();
   String nameSpace();
   Boolean isHttps ();
   int port_;
   String protocol_;
   String hostName_;
   String nameSpace_;
   Boolean fHttps;
};

class _dataType {
   public:

   _dataType (int          type,
              int          size,
              Boolean      reference,
              Boolean      null,
              Boolean      array,
              String&      refClass,
              Boolean      fromProperty)
      : _type(type),
        _size(size),
        _reference(reference),
        _array(array),
        _refClass(refClass),
        _fromProperty(fromProperty)
   {
   }

   _dataType (int          type)
      : _type(type),
        _size(1),
        _reference(false),
        _array(false),
        _refClass(String::EMPTY),
        _fromProperty(false)
   {
   }

   _dataType (int          type,
              int          size)
      : _type(type),
        _size(size),
        _reference(false),
        _array(true),
        _refClass(String::EMPTY),
        _fromProperty(true)
   {
   }

   _dataType (int          type,
              const String ref)
       : _type(type),
         _size(0),
         _reference(true),
         _array(false),
         _refClass(ref),
         _fromProperty(true)
   {
   }

   static int convertCTypeToJavaType (CIMType cType, bool *pfSuccess)
   {
      static int cTypeToJType[]=
      /*  CIMType.h                CIMDataType.java
          CIMTYPE_BOOLEAN,     0   public static final int BOOLEAN   =      10;
          CIMTYPE_UINT8,       1   public static final int UINT8     =       1;
          CIMTYPE_SINT8,       2   public static final int SINT8     =       2;
          CIMTYPE_UINT16,      3   public static final int UINT16    =       3;
          CIMTYPE_SINT16,      4   public static final int SINT16    =       4;
          CIMTYPE_UINT32,      5   public static final int UINT32    =       5;
          CIMTYPE_SINT32,      6   public static final int SINT32    =       6;
          CIMTYPE_UINT64,      7   public static final int UINT64    =       7;
          CIMTYPE_SINT64,      8   public static final int SINT64    =       8;
          CIMTYPE_REAL32,      9   public static final int REAL32    =      11;
          CIMTYPE_REAL64,     10   public static final int REAL64    =      12;
          CIMTYPE_CHAR16,     11   public static final int CHAR16    =      14;
          CIMTYPE_STRING,     12   public static final int STRING    =       9;
          CIMTYPE_DATETIME,   13   public static final int DATETIME  =      13;
          CIMTYPE_REFERENCE,  14   public static final int REFERENCE = 0x20+ 1;
          CIMTYPE_OBJECT      15   public static final int OBJECT    =      15;
      */
      //  p:      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14     15
      {/* j: */  10, 1, 2, 3, 4, 5, 6, 7, 8,11,12,14, 9,13,0x20+1,15};

      if ((int)cType < (int)CIMTYPE_BOOLEAN || (int)cType > (int)CIMTYPE_OBJECT)
      {
         *pfSuccess = false;

         return cTypeToJType[0];
      }
      else
      {
         *pfSuccess = true;

         return cTypeToJType[cType];
      }
   }

   static CIMType convertJavaTypeToCType (int jType, bool *pfSuccess)
   {
      static int jTypeToCType[]=
      /*  CIMDataType.java                              CIMType.h
          public static final int UINT8     =       1;  CIMTYPE_UINT8,       1
          public static final int SINT8     =       2;  CIMTYPE_SINT8,       2
          public static final int UINT16    =       3;  CIMTYPE_UINT16,      3
          public static final int SINT16    =       4;  CIMTYPE_SINT16,      4
          public static final int UINT32    =       5;  CIMTYPE_UINT32,      5
          public static final int SINT32    =       6;  CIMTYPE_SINT32,      6
          public static final int UINT64    =       7;  CIMTYPE_UINT64,      7
          public static final int SINT64    =       8;  CIMTYPE_SINT64,      8
          public static final int STRING    =       9;  CIMTYPE_STRING,     12
          public static final int BOOLEAN   =      10;  CIMTYPE_BOOLEAN,     0
          public static final int REAL32    =      11;  CIMTYPE_REAL32,      9
          public static final int REAL64    =      12;  CIMTYPE_REAL64,     10
          public static final int DATETIME  =      13;  CIMTYPE_DATETIME,   13
          public static final int CHAR16    =      14;  CIMTYPE_CHAR16,     11
          public static final int REFERENCE = 0x20+ 1;  CIMTYPE_REFERENCE,  14
          public static final int OBJECT    =      15;  CIMTYPE_OBJECT      15
      */
      //  j:      0  1  2  3  4  5  6  7  8   9 10 11  12  13  14  15
      {/* p: */   0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 0, 9, 10, 13, 11, 15};

      if (jType < 1)
      {
         *pfSuccess = false;

         return CIMTYPE_BOOLEAN;
      }
      else if (jType <= 15)
      {
         *pfSuccess = true;

         return (CIMType)jTypeToCType[jType];
      }
      else if (jType == 0x20 + 1) // REFERENCE
      {
         *pfSuccess = true;

         return CIMTYPE_REFERENCE;
      }
      else
      {
         *pfSuccess = false;

         return CIMTYPE_BOOLEAN;
      }
   }

   static char *convertJavaTypeToChars (int jType, bool *pfSuccess)
   {
      static char *jTypeToChars[]= {
      /* CIMDataType.java                             */ NULL,
      /* public static final int UINT8     =       1; */ "uint8",
      /* public static final int SINT8     =       2; */ "sint8",
      /* public static final int UINT16    =       3; */ "uint16",
      /* public static final int SINT16    =       4; */ "sint16",
      /* public static final int UINT32    =       5; */ "uint32",
      /* public static final int SINT32    =       6; */ "sint32",
      /* public static final int UINT64    =       7; */ "uint64",
      /* public static final int SINT64    =       8; */ "sint64",
      /* public static final int STRING    =       9; */ "string",
      /* public static final int BOOLEAN   =      10; */ "boolean",
      /* public static final int REAL32    =      11; */ "real32",
      /* public static final int REAL64    =      12; */ "real64",
      /* public static final int DATETIME  =      13; */ "datetime",
      /* public static final int CHAR16    =      14; */ "char16",
      /* public static final int OBJECT    =      15; */ "object"
      };

      if (jType < 1)
      {
         *pfSuccess = false;

         return NULL;
      }
      else if (jType <= 15)
      {
         *pfSuccess = true;

         return jTypeToChars[jType];
      }
      else if (jType == 0x20 + 1) // REFERENCE
      {
         *pfSuccess = true;

         return "reference";
      }
      else
      {
         *pfSuccess = false;

         return NULL;
      }
   }

   int     _type;
   int     _size;
   Boolean _reference;
   Boolean _null;
   Boolean _array;
   String  _refClass;
   Boolean _fromProperty;
};

#define VectorClassRef                 classRefs[0]
#define BooleanClassRef                classRefs[1]
#define ByteClassRef                   classRefs[2]
#define ShortClassRef                  classRefs[3]
#define IntegerClassRef                classRefs[4]
#define LongClassRef                   classRefs[5]
#define FloatClassRef                  classRefs[6]
#define DoubleClassRef                 classRefs[7]
#define UnsignedInt8ClassRef           classRefs[8]
#define UnsignedInt16ClassRef          classRefs[9]
#define UnsignedInt32ClassRef          classRefs[10]
#define UnsignedInt64ClassRef          classRefs[11]
#define CIMObjectPathClassRef          classRefs[12]
#define CIMExceptionClassRef           classRefs[13]
#define BigIntegerClassRef             classRefs[14]
#define CIMPropertyClassRef            classRefs[15]
#define CIMOMHandleClassRef            classRefs[16]
#define CIMClassClassRef               classRefs[17]
#define CIMInstanceClassRef            classRefs[18]
#define CIMValueClassRef               classRefs[19]
#define ObjectClassRef                 classRefs[20]
#define ThrowableClassRef              classRefs[21]
#define StringClassRef                 classRefs[22]
#define JarClassLoaderClassRef         classRefs[23]
#define CIMDateTimeClassRef            classRefs[24]
#define SelectExpClassRef              classRefs[25]
#define CIMQualifierClassRef           classRefs[26]
#define CIMQualifierTypeClassRef       classRefs[27]
#define CIMFlavorClassRef              classRefs[28]
#define CIMArgumentClassRef            classRefs[29]
#define CIMInstanceExceptionClassRef   classRefs[30]
#define CIMObjectClassRef              classRefs[31]
#define CharacterClassRef              classRefs[32]
#define OperationContextClassRef       classRefs[33]
#define ClassClassRef                  classRefs[34]
#define ByteArrayOutputStreamClassRef  classRefs[35]
#define PrintStreamClassRef            classRefs[36]

#define BigIntegerValueOf              staticMethodIDs[0]
#define JarClassLoaderLoad             staticMethodIDs[1]

#define VectorNew                      instMethodIDs[0]
#define VectorAddElement               instMethodIDs[15]
#define VectorElementAt                instMethodIDs[16]
#define VectorRemoveElementAt          instMethodIDs[30]
#define VectorSize                     instMethodIDs[27]
#define BooleanNewZ                    instMethodIDs[1]
#define ByteNewB                       instMethodIDs[2]
#define ShortNewS                      instMethodIDs[3]
#define IntegerNewI                    instMethodIDs[4]
#define LongNewJ                       instMethodIDs[5]
#define FloatNewF                      instMethodIDs[6]
#define DoubleNewD                     instMethodIDs[7]
#define UnsignedInt8NewS               instMethodIDs[8]
#define UnsignedInt16NewI              instMethodIDs[9]
#define UnsignedInt32NewJ              instMethodIDs[10]
#define UnsignedInt64NewBi             instMethodIDs[11]
#define CIMObjectPathNewJ              instMethodIDs[12]
#define CIMObjectPathCInst             instMethodIDs[21]
#define CIMExceptionNewSt              instMethodIDs[13]
#define CIMExceptionNewISt             instMethodIDs[32]
#define CIMExceptionNewI               instMethodIDs[18]
#define CIMExceptionNew                instMethodIDs[41]
#define CIMExceptionNewStOb            instMethodIDs[42]
#define CIMExceptionNewStObOb          instMethodIDs[43]
#define CIMExceptionNewStObObOb        instMethodIDs[44]
#define CIMExceptionGetCode            instMethodIDs[33]
#define CIMExceptionGetID              instMethodIDs[26]
#define CIMPropertyNewJ                instMethodIDs[14]
#define CIMPropertyCInst               instMethodIDs[28]
#define CIMOMHandleNewJSt              instMethodIDs[17]
#define CIMOMHandleGetClass            instMethodIDs[29]
#define CIMClassNewJ                   instMethodIDs[19]
#define CIMClassCInst                  instMethodIDs[23]
#define CIMInstanceNewJ                instMethodIDs[20]
#define CIMInstanceCInst               instMethodIDs[22]
#define CIMValueNewJ                   instMethodIDs[45]
#define CIMValueCInst                  instMethodIDs[31]
#define CIMDateTimeNewJ                instMethodIDs[34]
#define SelectExpNewJ                  instMethodIDs[35]
#define CIMQualifierNewJ               instMethodIDs[36]
#define CIMFlavorNewI                  instMethodIDs[37]
#define CIMFlavorGetFlavor             instMethodIDs[38]
#define CIMArgumentNewJ                instMethodIDs[40]
#define CIMArgumentCInst               instMethodIDs[39]
#define ObjectToString                 instMethodIDs[24]
#define ThrowableGetMessage            instMethodIDs[25]
#define CIMObjectNewJZ                 instMethodIDs[46]
#define CharacterNewC                  instMethodIDs[47]
#define OperationContextNewJ           instMethodIDs[48]
#define OperationContextUnassociate    instMethodIDs[49]
#define ClassGetInterfaces             instMethodIDs[50]
#define ClassGetName                   instMethodIDs[51]
#define UnsignedInt64NewStr            instMethodIDs[52]
#define ByteArrayOutputStreamNew       instMethodIDs[53]
#define PrintStreamNewOb               instMethodIDs[54]
#define ThrowablePrintStackTrace       instMethodIDs[55]
#define ByteArrayOutputStreamToString  instMethodIDs[56]

//extern "C" JNIEnv* attachThread(JvmVector**);
//extern "C" void detachThread();
//extern "C" jobject getProvider(JNIEnv*,const char*,jclass*);
//extern "C" void checkException(JNIEnv *env);

typedef JNIEnv* (*JvmAttach)(JvmVector**);
typedef void (*JvmDetach)();

PEGASUS_NAMESPACE_END

#endif
