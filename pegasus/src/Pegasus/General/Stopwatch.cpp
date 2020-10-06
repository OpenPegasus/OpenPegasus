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
#include "Stopwatch.h"

#include <Pegasus/Common/TimeValue.h>
#include <Pegasus/Common/System.h>

#include <iostream>

PEGASUS_NAMESPACE_BEGIN

Stopwatch::Stopwatch() : _start(0), _stop(0), _total(0)
{
}

void Stopwatch::start()
{
    _start = TimeValue::getCurrentTime().toMicroseconds();
}

void Stopwatch::stop()
{
    _stop = TimeValue::getCurrentTime().toMicroseconds();
    _total += _stop - _start;
}

void Stopwatch::reset()
{
    _start = 0;
    _stop = 0;
    _total = 0;
}

double Stopwatch::getElapsed() const
{
    Sint64 tmp = (Sint64)_total;
    return (double)tmp / (double)1000000.0;
}

Uint64 Stopwatch::getElapsedUsec() const
{
    return _total;
}

void Stopwatch::printElapsed()
{
    PEGASUS_STD(cout) << getElapsed() << " seconds" << PEGASUS_STD(endl);
}

PEGASUS_NAMESPACE_END
