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
#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include <Providers/TestProviders/CMPI/TestUtilLib/Linkage.h>

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#define PEGASUS_64BIT_CONVERSION_WIDTH "I64"
#else
#define PEGASUS_64BIT_CONVERSION_WIDTH "ll"
#endif

#if defined(CMPI_PLATFORM_WIN32_IX86_MSVC)
#define PEGASUS_UINT64_LITERAL(X) ((unsigned __int64)X)
#else
#define PEGASUS_UINT64_LITERAL(X) X##ULL
#endif

PEGASUS_CMPI_TESTPRV_UTILLIB_LINKAGE extern const CMPIBoolean CMPI_true;
PEGASUS_CMPI_TESTPRV_UTILLIB_LINKAGE extern const CMPIBoolean CMPI_false;

CMPI_EXTERN_C void
PROV_LOG (const char *fmt, ...);

CMPI_EXTERN_C void
PROV_LOG_CLOSE (void);

CMPI_EXTERN_C void
PROV_LOG_OPEN (const char *file, const char *location);

CMPI_EXTERN_C const char *
strCMPIValueState (CMPIValueState state);

CMPI_EXTERN_C const char *
strCMPIPredOp (CMPIPredOp op);

CMPI_EXTERN_C const char *
strCMPIStatus (CMPIStatus stat);

CMPI_EXTERN_C int _CMSameType( CMPIData value1, CMPIData value2 );
CMPI_EXTERN_C int _CMSameValue( CMPIData value1, CMPIData value2 );
CMPI_EXTERN_C int _CMSameObject(const CMPIObjectPath * object1,
                                const CMPIObjectPath * object2 );
CMPI_EXTERN_C int _CMSameInstance(const CMPIInstance * instance1,
                                  const CMPIInstance * instance2 );
CMPI_EXTERN_C const char * _CMPIrcName ( CMPIrc rc );
CMPI_EXTERN_C const char * _CMPITypeName (CMPIType type);
CMPI_EXTERN_C char * _CMPIValueToString (CMPIData data);

CMPI_EXTERN_C void check_CMPIStatus (CMPIStatus rc);

CMPI_EXTERN_C CMPIBoolean
evalute_selectcond (const CMPISelectCond * cond,
                    CMPIAccessor *accessor,
                    void *parm);

CMPI_EXTERN_C CMPIBoolean
evaluate(const CMPISelectExp *se,
         const CMPIInstance *inst,
         CMPIAccessor *inst_accessor,
         void *parm );

CMPI_EXTERN_C CMPIData
instance_accessor (const char *name, void *param);

