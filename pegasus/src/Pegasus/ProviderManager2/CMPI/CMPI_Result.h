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

#ifndef _CMPI_Result_H_
#define _CMPI_Result_H_

#include <Pegasus/Provider/CMPI/cmpidt.h>
#include <Pegasus/Provider/CMPI/cmpift.h>
#include "CMPI_Ftabs.h"
#include "CMPI_Object.h"
#include "CMPI_Broker.h"
#include "CMPI_Error.h"

#include <Pegasus/Common/ResponseHandler.h>
#include <Pegasus/ProviderManager2/OperationResponseHandler.h>
PEGASUS_NAMESPACE_BEGIN

#define RESULT_Instance   1
#define RESULT_Object     2
#define RESULT_ObjectPath 4
#define RESULT_Value      8
#define RESULT_Method     16
#define RESULT_Indication 32
#define RESULT_Response   64
#define RESULT_set        128
#define RESULT_done       256

//typedef struct _CMPIResultRefFT : public CMPIResultFT {
//} CMPIResultRefFT;
//typedef struct _CMPIResultInstFT : public CMPIResultFT {
//} CMPIResultInstFT;
//typedef struct _CMPIResultDataFT : public CMPIResultFT {
//} CMPIResultDataFT;
//typedef struct _CMPIResultMethFT : public CMPIResultFT {
//} CMPIResultMethFT;

struct CMPI_Result : CMPIResult
{
    CMPI_Object *next,*prev;
    long flags;
    CMPI_Broker *xBroker;
    CMPI_Error *resError;
};

struct CMPI_ResultOnStack : CMPIResult
{
    CMPI_Object *next,*prev;
    long flags;
    CMPI_Broker *xBroker;
    CMPI_Error *resError;
    CMPI_ResultOnStack(const ExecQueryResponseHandler&,CMPI_Broker*);
    CMPI_ResultOnStack(const SimpleObjectPathResponseHandler&,CMPI_Broker*);
    CMPI_ResultOnStack(const SimpleInstanceResponseHandler&,CMPI_Broker*);
    CMPI_ResultOnStack(const MethodResultResponseHandler&,CMPI_Broker*);
    CMPI_ResultOnStack(const SimpleObjectResponseHandler&,CMPI_Broker*);
    CMPI_ResultOnStack(const ResponseHandler&,CMPI_Broker*);
    ~CMPI_ResultOnStack();
};

PEGASUS_NAMESPACE_END

#endif

