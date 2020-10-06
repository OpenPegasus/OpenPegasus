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

#include <new>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/Common/PegasusAssert.h>

#include "WsmValue.h"
#include "WsmEndpointReference.h"
#include "WsmInstance.h"

PEGASUS_NAMESPACE_BEGIN

//==============================================================================
//
// WsmValueRep
//
//==============================================================================

struct WsmValueRep
{
    typedef WsmEndpointReference Ref;
    typedef Array<WsmEndpointReference> RefA;
    typedef WsmInstance Inst;
    typedef Array<WsmInstance> InstA;
    typedef String Str;
    typedef Array<String> StrA;

    AtomicInt refCount;
    WsmType type;
    Boolean isArray;
    Boolean isNull;

    // This union makes enough space for each of the given types in raw memory.
    // WsmValue takes care or doing placement construction and destruction as
    // needed.
    union
    {
        // Be sure all types are aligned on 64-bit boundary.
        Uint64 _alignment_;
        char _ref_[sizeof(Ref)];
        char _refa_[sizeof(RefA)];
        char _inst_[sizeof(Inst)];
        char _insta_[sizeof(InstA)];
        char _stra_[sizeof(StrA)];
        char _str_[sizeof(Str)];
        char buf[1];
    };

    enum EmptyRepTag { EMPTY_REP_TAG };

    WsmValueRep(EmptyRepTag) :
        refCount(2), type(WSMTYPE_OTHER), isArray(false), isNull(true)
    {
        // This construction is for the empty representation. By maintaining
        // a reference count of one, it will never fall to zero and never
        // be deleted by _unref() below.
    }

    // Construct a null rep.
    WsmValueRep() :
        refCount(1), type(WSMTYPE_OTHER), isArray(false), isNull(true)
    {
    }

    WsmValueRep(const Ref& x) :
        refCount(1), type(WSMTYPE_REFERENCE), isArray(false), isNull(false)
    {
        new(buf) Ref(x);
    }

    WsmValueRep(const RefA& x) :
        refCount(1), type(WSMTYPE_REFERENCE), isArray(true), isNull(false)
    {
        new(buf) RefA(x);
    }

    WsmValueRep(const Inst& x) :
        refCount(1), type(WSMTYPE_INSTANCE), isArray(false), isNull(false)
    {
        new(buf) Inst(x);
    }

    WsmValueRep(const InstA& x) :
        refCount(1), type(WSMTYPE_INSTANCE), isArray(true), isNull(false)
    {
        new(buf) InstA(x);
    }

    WsmValueRep(const Str& x) :
        refCount(1), type(WSMTYPE_OTHER), isArray(false), isNull(false)
    {
        new(buf) Str(x);
    }

    WsmValueRep(const StrA& x) :
        refCount(1), type(WSMTYPE_OTHER), isArray(true), isNull(false)
    {
        new(buf) StrA(x);
    }

    ~WsmValueRep()
    {
        clear();
    }

    void clear()
    {
        if (isNull)
            return;

        if (isArray)
        {
            switch (type)
            {
                case WSMTYPE_REFERENCE:
                    ((RefA*)buf)->~RefA();
                    break;

                case WSMTYPE_INSTANCE:
                    ((InstA*)buf)->~InstA();
                    break;

                case WSMTYPE_OTHER:
                    ((StrA*)buf)->~StrA();
                    break;

                default:
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                    break;
            }
        }
        else
        {
            switch (type)
            {
                case WSMTYPE_REFERENCE:
                    ((Ref*)buf)->~Ref();
                    break;

                case WSMTYPE_INSTANCE:
                    ((Inst*)buf)->~Inst();
                    break;

                case WSMTYPE_OTHER:
                    ((Str*)buf)->~Str();
                    break;

                default:
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                    break;
            }
        }

        type = WSMTYPE_OTHER;
        isNull = true;
        isArray = false;
    }

    WsmValueRep* clone() const
    {
        if (isNull)
            return new WsmValueRep();

        if (isArray)
        {
            switch (type)
            {
                case WSMTYPE_REFERENCE:
                    return new WsmValueRep(*((RefA*)buf));
                case WSMTYPE_INSTANCE:
                    return new WsmValueRep(*((InstA*)buf));
                case WSMTYPE_OTHER:
                    return new WsmValueRep(*((StrA*)buf));
                default:
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                    break;
            }
        }
        else
        {
            switch (type)
            {
                case WSMTYPE_REFERENCE:
                    return new WsmValueRep(*((Ref*)buf));
                case WSMTYPE_INSTANCE:
                    return new WsmValueRep(*((Inst*)buf));
                case WSMTYPE_OTHER:
                    return new WsmValueRep(*((Str*)buf));
                default:
                    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                    break;
            }
        }

        // Unreachable!
        return 0;
    }

    Ref& ref() { return *((Ref*)buf); }
    const Ref& ref() const { return *((Ref*)buf); }

    RefA& refa() { return *((RefA*)buf); }
    const RefA& refa() const { return *((RefA*)buf); }

    Inst& inst() { return *((Inst*)buf); }
    const Inst& inst() const { return *((Inst*)buf); }

    InstA& insta() { return *((InstA*)buf); }
    const InstA& insta() const { return *((InstA*)buf); }

    Str& str() { return *((Str*)buf); }
    const Str& str() const { return *((Str*)buf); }

    StrA& stra() { return *((StrA*)buf); }
    const StrA& stra() const { return *((StrA*)buf); }
};

static WsmValueRep _emptyRep(WsmValueRep::EMPTY_REP_TAG);

inline void _ref(const WsmValueRep* rep_)
{
    WsmValueRep* rep = (WsmValueRep*)rep_;

    if (rep && rep != &_emptyRep)
        rep->refCount.inc();
}

inline void _unref(const WsmValueRep* rep_)
{
    WsmValueRep* rep = (WsmValueRep*)rep_;

    if (rep && rep != &_emptyRep && rep->refCount.decAndTestIfZero())
        delete rep;
}

//==============================================================================
//
// WsmValue
//
//==============================================================================

WsmValue::WsmValue() : _rep(&_emptyRep)
{
}

WsmValue::WsmValue(const WsmValue& x)
{
    _ref(_rep = x._rep);
}

WsmValue::WsmValue(const WsmEndpointReference& x)
{
    _rep = new WsmValueRep(x);
}

WsmValue::WsmValue(const Array<WsmEndpointReference>& x)
{
    _rep = new WsmValueRep(x);
}

WsmValue::WsmValue(const WsmInstance& x)
{
    _rep = new WsmValueRep(x);
}

WsmValue::WsmValue(const Array<WsmInstance>& x)
{
    _rep = new WsmValueRep(x);
}

WsmValue::WsmValue(const String& x)
{
    _rep = new WsmValueRep(x);
}

WsmValue::WsmValue(const Array<String>& x)
{
    _rep = new WsmValueRep(x);
}

WsmValue::~WsmValue()
{
    _unref(_rep);
}

WsmValue& WsmValue::operator=(const WsmValue& x)
{
    if (this != &x)
    {
        _unref(_rep);
        _ref(_rep = x._rep);
    }

    return *this;
}

Uint32 WsmValue::getArraySize()
{
    if (_rep->isArray)
    {
        switch (_rep->type)
        {
            case WSMTYPE_REFERENCE:
                return _rep->refa().size();

            case WSMTYPE_INSTANCE:
                return _rep->insta().size();

            case WSMTYPE_OTHER:
                return _rep->stra().size();

            default:
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
        }
    }

    return 0;
}

void WsmValue::get(WsmEndpointReference& ref) const
{
    if (_rep->type != WSMTYPE_REFERENCE || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        ref = _rep->ref();
}

void WsmValue::get(Array<WsmEndpointReference>& ref) const
{
    if (_rep->type != WSMTYPE_REFERENCE || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        ref = _rep->refa();
}

void WsmValue::get(WsmInstance& inst) const
{
    if (_rep->type != WSMTYPE_INSTANCE || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        inst = _rep->inst();
}

void WsmValue::get(Array<WsmInstance>& inst) const
{
    if (_rep->type != WSMTYPE_INSTANCE || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        inst = _rep->insta();
}

void WsmValue::get(String& str) const
{
    if (_rep->type != WSMTYPE_OTHER || _rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        str = _rep->str();
}

void WsmValue::get(Array<String>& str) const
{
    if (_rep->type != WSMTYPE_OTHER || !_rep->isArray)
        throw TypeMismatchException();

    if (!_rep->isNull)
        str = _rep->stra();
}

void WsmValue::set(const WsmEndpointReference& x)
{
    _unref(_rep);
    _rep = new WsmValueRep(x);
}

void WsmValue::set(const Array<WsmEndpointReference>& x)
{
    _unref(_rep);
    _rep = new WsmValueRep(x);
}

void WsmValue::set(const WsmInstance& x)
{
    _unref(_rep);
    _rep = new WsmValueRep(x);
}

void WsmValue::set(const Array<WsmInstance>& x)
{
    _unref(_rep);
    _rep = new WsmValueRep(x);
}

void WsmValue::set(const String& x)
{
    _unref(_rep);
    _rep = new WsmValueRep(x);
}

void WsmValue::set(const Array<String>& x)
{
    _unref(_rep);
    _rep = new WsmValueRep(x);
}

void WsmValue::setNull()
{
    if (_rep->refCount.get() == 1)
    {
        _rep->clear();
    }
    else
    {
        WsmValueRep* rep = new WsmValueRep();
        _unref(_rep);
        _rep = rep;
    }
}

void WsmValue::add(const WsmValue& x)
{
    PEGASUS_ASSERT(!x._rep->isArray);

    if (x._rep->isArray)
        return;

    if (_rep->type != x._rep->type)
        throw TypeMismatchException();

    if (_rep->isArray)
    {
        if (_rep->refCount.get() != 1)
        {
            WsmValueRep* rep = _rep->clone();
            _unref(_rep);
            _rep = rep;
        }

        switch (_rep->type)
        {
            case WSMTYPE_REFERENCE:
            {
                _rep->refa().append(x._rep->ref());
                break;
            }
            case WSMTYPE_INSTANCE:
            {
                _rep->insta().append(x._rep->inst());
                break;
            }
            case WSMTYPE_OTHER:
            {
                _rep->stra().append(x._rep->str());
                break;
            }
            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
            }
        }
    }
    else
    {
        switch (_rep->type)
        {
            case WSMTYPE_REFERENCE:
            {
                Array<WsmEndpointReference> t;
                t.append(_rep->ref());
                t.append(x._rep->ref());
                set(t);
                break;
            }
            case WSMTYPE_INSTANCE:
            {
                Array<WsmInstance> t;
                t.append(_rep->inst());
                t.append(x._rep->inst());
                set(t);
                break;
            }
            case WSMTYPE_OTHER:
            {
                Array<String> t;
                t.append(_rep->str());
                t.append(x._rep->str());
                set(t);
                break;
            }
            default:
            {
                PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
                break;
            }
        }
    }
}

void WsmValue::toArray()
{
    if (_rep->isArray)
        return;

    switch (_rep->type)
    {
        case WSMTYPE_REFERENCE:
        {
            Array<WsmEndpointReference> x;
            x.append(_rep->ref());
            set(x);
            break;
        }
        case WSMTYPE_INSTANCE:
        {
            Array<WsmInstance> x;
            x.append(_rep->inst());
            set(x);
            break;
        }
        case WSMTYPE_OTHER:
        {
            Array<String> x;
            x.append(_rep->str());
            set(x);
            break;
        }
        default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
            break;
        }
    }
}

Boolean WsmValue::isArray() const
{
    return _rep->isArray;
}

Boolean WsmValue::isNull() const
{
    return _rep->isNull;
}

WsmType WsmValue::getType() const
{
    return _rep->type;
}

PEGASUS_NAMESPACE_END
