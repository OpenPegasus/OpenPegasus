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

#include <iostream>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/StatisticalData.h>
#include <Pegasus/Common/Message.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

Boolean verbose;
#define VCOUT if (verbose) cout

int main(int, char** argv)
{
    verbose = getenv("PEGASUS_TEST_VERBOSE") ? true : false;

StatisticalData* sd = StatisticalData::current();
StatisticalData* curr = StatisticalData::current();
//cur = StatisticalData::current();

//check to make sure current() returns a pointer to the existing StatisticalData
// object

PEGASUS_TEST_ASSERT(sd->length == curr->length);
PEGASUS_TEST_ASSERT(sd->requestSize == curr->requestSize);
PEGASUS_TEST_ASSERT(sd->copyGSD == curr->copyGSD);
PEGASUS_TEST_ASSERT(sd->numCalls[5] == curr->numCalls[5]);
PEGASUS_TEST_ASSERT(sd->requestSize[6] == curr->requestSize[6]);


//  *****************************************
// check the addToValue() method

// Changes sd.numCalls[5] from 0 to 10
sd->addToValue(10,CIM_GET_CLASS_REQUEST_MESSAGE,
               StatisticalData::PEGASUS_STATDATA_SERVER);

// Changes sd.requestSize[6] form 0 to 10
sd->addToValue(10,CIM_GET_CLASS_REQUEST_MESSAGE,
               StatisticalData::PEGASUS_STATDATA_BYTES_READ);

//PEGASUS_TEST_ASSERT(sd->numCalls[5] == 0);
//PEGASUS_TEST_ASSERT(sd->requestSize[6] == 10);

//**********************************************
// check the setCopyGSD method

sd->setCopyGSD(1);

PEGASUS_TEST_ASSERT(sd->copyGSD == 1);

//****************************************************
// make sure the cur the sd objects are still the same

PEGASUS_TEST_ASSERT(sd->length == curr->length);
PEGASUS_TEST_ASSERT(sd->requestSize == curr->requestSize);
PEGASUS_TEST_ASSERT(sd->copyGSD == curr->copyGSD);
PEGASUS_TEST_ASSERT(sd->numCalls[5] == curr->numCalls[5]);
PEGASUS_TEST_ASSERT(sd->requestSize[6] == curr->requestSize[6]);


//**************************

    cout << argv[0] << " +++++ passed all tests" << endl;

    return 0;
}

