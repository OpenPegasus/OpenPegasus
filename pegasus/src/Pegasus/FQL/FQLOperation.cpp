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

#include "FQLOperation.h"

PEGASUS_NAMESPACE_BEGIN

const char* FQLOperationToString(FQLOperation x)
{
    switch (x)
    {
        case FQL_UNKNOWN: return "FQL_UNKNOWN";
        case FQL_OR: return "FQL_OR";
        case FQL_AND: return "FQL_AND";
        case FQL_NOT: return "FQL_NOT";
        case FQL_EQ: return "FQL_EQ";
        case FQL_NE: return "FQL_NE";
        case FQL_LT: return "FQL_LT";
        case FQL_LE: return "FQL_LE";
        case FQL_GT: return "FQL_GT";
        case FQL_GE: return "FQL_GE";
        case FQL_LIKE: return "FQL_LIKE";
        case FQL_NOT_LIKE: return "FQL_NOT_LIKE";
        case FQL_ANY: return "FQL_ANY";
        case FQL_NOT_ANY: return "FQL_NOT_ANY";
        case FQL_EVERY: return "FQL_EVERY";
        case FQL_NOT_EVERY: return "FQL_NOT_EVERY";
        case FQL_IS_NULL: return "FQL_IS_NULL";
////      case FQL_IS_TRUE: return "FQL_IS_TRUE";
////      case FQL_IS_FALSE: return "FQL_IS_FALSE";
////      case FQL_IS_NOT_NULL: return "FQL_IS_NOT_NULL";
////      case FQL_IS_NOT_TRUE: return "FQL_IS_NOT_TRUE";
////      case FQL_IS_NOT_FALSE: return "FQL_IS_NOT_FALSE";
        default: return "UNKNOWN OPERATION";
    }
}

PEGASUS_NAMESPACE_END

