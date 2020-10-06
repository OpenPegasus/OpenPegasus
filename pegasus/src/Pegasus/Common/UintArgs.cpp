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
//%////////////////////////////////////////////////////////////////////////////
#include "UintArgsRep.h"
#include "UintArgs.h"
#include "StringConversion.h"
#include "AtomicInt.h"

PEGASUS_NAMESPACE_BEGIN

template<class REP>
inline void Ref(REP* rep)
{
        rep->_refCounter++;
}

template<class REP>
inline void Unref(REP* rep)
{
    if (rep->_refCounter.decAndTestIfZero())
        delete rep;
}

//////////////////////////////////////////////////////////////////
// Uint32Arg Integer Class used for parameters that
// require Uint32 on input or output. Provides for NULL as well as
// all Uint32 values
//////////////////////////////////////////////////////////////////

Uint32Arg::Uint32Arg()
{
    _rep = new Uint32ArgRep();
}

Uint32Arg::Uint32Arg(const Uint32Arg& x)
{

    _rep = x._rep;
    Ref(_rep);
}

Uint32Arg::Uint32Arg(Uint32 x)
{
    _rep = new Uint32ArgRep();
    _rep->_value = x;
    _rep->_null = false;
}

static inline Uint32ArgRep* _copyOnWriteUint32ArgRep(
    Uint32ArgRep* rep)
{
    if (rep->_refCounter.get() > 1)
    {
        Uint32ArgRep* tmpRep= new Uint32ArgRep(*rep);
        Unref(rep);
        return tmpRep;
    }
    else
    {
        return rep;
    }
}

Uint32Arg::~Uint32Arg()
{
    Unref(_rep);
}

Uint32Arg& Uint32Arg::operator=(const Uint32Arg& x)
{
    if (x._rep != _rep)
    {
        Unref(_rep);
        _rep = x._rep;
        Ref(_rep);
    }
    return *this;
}


const Uint32& Uint32Arg::getValue() const
{
    return _rep->_value;
}

void Uint32Arg::setValue(Uint32 x)
{

    _rep = _copyOnWriteUint32ArgRep(_rep);
    _rep->_value = x;
    _rep->_null = false;
}

Boolean Uint32Arg::isNull() const
{
    return _rep->_null;
}

void Uint32Arg::setNullValue()
{
    _rep = _copyOnWriteUint32ArgRep(_rep);
    _rep->_value = 0;
    _rep->_null = true;
}

String Uint32Arg::toString()
{
    String s;
    if (_rep->_null)
    {
        s = "NULL";
    }
    else
    {
        char buffer[22];
        Uint32 size;
        const char* rtn = Uint32ToString(buffer, _rep->_value, size);
        s = rtn;
    }
    return s;
}

Boolean Uint32Arg::equal(const Uint32Arg& x) const
{
    if ((_rep->_null != x._rep->_null))
    {
        return false;
    }
    return _rep->_null? true : (_rep->_value == x._rep->_value);
}

Boolean operator==(const Uint32Arg& x, const Uint32Arg& y)
{
    return x.equal(y);
}

//////////////////////////////////////////////////////////////
// Uint64 Class Used for handling of Uint64
// parameters on Client input and output
/////////////////////////////////////////////////////////////

Uint64Arg::Uint64Arg()
{
    _rep = new Uint64ArgRep();
}

Uint64Arg::Uint64Arg(const Uint64Arg& x)
{

    _rep = x._rep;
    Ref(_rep);
}

Uint64Arg::Uint64Arg(Uint64 x)
{
    _rep = new Uint64ArgRep();
    _rep->_value = x;
    _rep->_null = false;
}

static inline Uint64ArgRep* _copyOnWriteUint64ArgRep(
    Uint64ArgRep* rep)
{
    if (rep->_refCounter.get() > 1)
    {
        Uint64ArgRep* tmpRep= new Uint64ArgRep(*rep);
        Unref(rep);
        return tmpRep;
    }
    else
    {
        return rep;
    }
}

Uint64Arg::~Uint64Arg()
{
    Unref(_rep);
}

Uint64Arg& Uint64Arg::operator=(const Uint64Arg& x)
{
    if (x._rep != _rep)
    {
        Unref(_rep);
        _rep = x._rep;
        Ref(_rep);
    }
    return *this;
}

const Uint64& Uint64Arg::getValue() const
{
    return _rep->_value;
}

void Uint64Arg::setValue(Uint64 x)
{

    _rep = _copyOnWriteUint64ArgRep(_rep);
    _rep->_value = x;
    _rep->_null = false;
}

Boolean Uint64Arg::isNull() const
{
    return _rep->_null;
}

void Uint64Arg::setNullValue()
{
    _rep = _copyOnWriteUint64ArgRep(_rep);
    _rep->_value = 0;
    _rep->_null = true;
}

String Uint64Arg::toString()
{
    String s;
    if (_rep->_null)
    {
        s = "NULL";
    }
    else
    {
        char buffer[22];
        Uint32 size;
        const char* rtn = Uint64ToString(buffer, _rep->_value, size);
        s = rtn;
    }
    return s;
}

Boolean Uint64Arg::equal(const Uint64Arg& x) const
{
    if ((_rep->_null != x._rep->_null))
    {
        return false;
    }
    return _rep->_null? true : (_rep->_value == x._rep->_value);
}

Boolean operator==(const Uint64Arg& x, const Uint64Arg& y)
{
    return x.equal(y);
}

PEGASUS_NAMESPACE_END
