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

#ifndef Pegasus_FQLOperation_h
#define Pegasus_FQLOperation_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/FQL/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** Represents FQL operators.

    These operators include the following:

    <ul>
        <li>OR</li>
        <li>AND</li>
        <li>NOT</li>
        <li>=</li>
        <li>&lt;&gt;</li>
        <li>&lt;</li>
        <li>&le;</li>
        <li>&gt;</li>
        <li>&ge;</li>
        <li>IS NULL</li>
        <li>IS NOT NULL</li>
        <li>IS TRUE</li>
        <li>IS NOT TRUE</li>
        <li>IS FALSE</li>
        <li>IS NOT FALSE</li>
    </ul>

    Here's an example of a query that uses some of these operations.

    <pre>
        x &gt; 10 OR y &le; 1.5) OR z = 19 AND name IS NOT NULL
    </pre>

    As the parser recognizes these operations, it adds the corresponding
    FQLOperation enumeration tag to the FQLQueryStatement by
    calling FQLQueryStatement::appendOperation().

    Operators may be converted to string by calling FQLOperationToString().

    Note that in expressions like this one:

    <pre>
        count IS TRUE
    </pre>

    The IS operator is NOT treated as a binary operator (which would take
    count and TRUE as its operands). Instead, the IS operator and TRUE operand
    are combined to form a single unary operation called "IS_TRUE". In this
    way, evaluation may be implemented by more efficiently (the TRUE operand
    is eliminated and therefore need not be handled). Conceptually, the above
    expression is equivalent to the following psuedo expression:

    <pre>
        IS_TRUE(count)
    </pre>

    Note that this technique is applied the following operations (also
    combined to be unary operations).

    <ul>
        <li>FQL_IS_NULL</li>
        <li>FQL_IS_NOT_NULL</li>
        <li>FQL_IS_TRUE</li>
        <li>FQL_IS_NOT_TRUE</li>
        <li>FQL_IS_FALSE</li>
        <li>FQL_IS_NOT_FALSE</li>
    </ul>
*/
enum FQLOperation
{
    FQL_UNKNOWN,
    FQL_OR,
    FQL_AND,
    FQL_NOT,
    FQL_EQ,
    FQL_NE,
    FQL_LT,
    FQL_LE,
    FQL_GT,
    FQL_GE,
    FQL_LIKE,
    FQL_NOT_LIKE,
    FQL_ANY,
    FQL_NOT_ANY,
    FQL_EVERY,
    FQL_NOT_EVERY,
    FQL_IS_NULL,
////  FQL_IS_NOT_NULL,
////  FQL_IS_TRUE,
////  FQL_IS_NOT_TRUE,
////  FQL_IS_FALSE,
////  FQL_IS_NOT_FALSE
};

PEGASUS_FQL_LINKAGE const char* FQLOperationToString(FQLOperation x);

PEGASUS_NAMESPACE_END

#endif /* Pegasus_FQLOperation_h */

