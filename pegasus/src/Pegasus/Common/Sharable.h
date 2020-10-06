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

#ifndef Pegasus_Sharable_h
#define Pegasus_Sharable_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/AtomicInt.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The Sharable class implements a simple reference counting scheme.
    The static Inc() method increments the reference count member.
    The static Dec() method decrements the reference count and deletes
    the object when the reference count becomes zero.

    Other classes may inherit this reference counting behavior simply
    by deriving from this class.
*/
class PEGASUS_COMMON_LINKAGE Sharable
{
public:

    Sharable() : _ref(1) { }

    virtual ~Sharable();

    friend void Inc(Sharable* sharable);

    friend void Dec(Sharable* sharable);

private:
    Sharable(const Sharable &s);
    Sharable& operator=(const Sharable &s);

    AtomicInt _ref;
};

inline void Inc(Sharable* x)
{
    if (x)
    {
        // A sharable object should never be incremented from zero.
        PEGASUS_DEBUG_ASSERT(x->_ref.get());
        x->_ref++;
    }
}

inline void Dec(Sharable* x)
{
  if (x && x->_ref.decAndTestIfZero())
    delete x;
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Sharable_h */
