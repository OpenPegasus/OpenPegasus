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


#ifndef _Pegasus_Common_Uint32ArgsRep_h
#define _Pegasus_Common_Uint32ArgsRep_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AtomicInt.h>


PEGASUS_NAMESPACE_BEGIN

class Uint32ArgRep
{
public:
    Uint32ArgRep()
        : _refCounter(1),
          _null(true),
          _value(0)
    {
    }
    Uint32ArgRep(const Uint32ArgRep& x)
        : _refCounter(1),
        _null(x._null),
        _value(x._value)
    {
    }
    Uint32ArgRep& operator=(const Uint32ArgRep& x)
    {
        if (&x != this)
        {
            _null = x._null;
            _value = x._value;
        }
        return *this;
    }
    ~Uint32ArgRep()
    {
    }

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;
    bool _null;
    Uint32 _value;
};

class Uint64ArgRep
{
public:
    Uint64ArgRep()
        : _refCounter(1),
          _null(true),
          _value(0)
    {
    }

    Uint64ArgRep(const Uint64ArgRep& x)
        : _refCounter(1),
        _null(x._null),
        _value(x._value)
    {
    }

    Uint64ArgRep& operator=(const Uint64ArgRep& x)
    {
        if (&x != this)
        {
            _null = x._null;
            _value = x._value;
        }
        return *this;
    }

    ~Uint64ArgRep()
    {
    }

    // reference counter as member to avoid
    // virtual function resolution overhead
    AtomicInt _refCounter;
    bool _null;
    Uint64 _value;
};

PEGASUS_NAMESPACE_END

#endif /* _Pegasus_Common_Uint32ArgsRep_h */
