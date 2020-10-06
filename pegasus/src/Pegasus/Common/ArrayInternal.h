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

#ifndef Pegasus_ArrayInternal_h
#define Pegasus_ArrayInternal_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/CIMName.h>

PEGASUS_NAMESPACE_BEGIN

#include <Pegasus/Common/ArrayImpl.h>

template<class PEGASUS_ARRAY_T>
Boolean operator==(
    const Array<PEGASUS_ARRAY_T>& x,
    const Array<PEGASUS_ARRAY_T>& y)
{
    if (x.size() != y.size())
        return false;

    for (Uint32 i = 0, n = x.size(); i < n; i++)
    {
        if (!(x[i] == y[i]))
            return false;
    }

    return true;
}

template<class PEGASUS_ARRAY_T>
Boolean Contains(const Array<PEGASUS_ARRAY_T>& a, const PEGASUS_ARRAY_T& x)
{
    Uint32 n = a.size();

    for (Uint32 i = 0; i < n; i++)
    {
        if (a[i] == x)
            return true;
    }

    return false;
}

template<class PEGASUS_ARRAY_T>
void BubbleSort(Array<PEGASUS_ARRAY_T>& x)
{
    Uint32 n = x.size();

    if (n < 2)
        return;

    for (Uint32 i = 0; i < n - 1; i++)
    {
        for (Uint32 j = 0; j < n - 1; j++)
        {
            if (x[j] > x[j+1])
            {
                PEGASUS_ARRAY_T t = x[j];
                x[j] = x[j+1];
                x[j+1] = t;
            }
        }
    }
}

inline void BubbleSort(Array<CIMName>& x)
{
    Uint32 n = x.size();

    if (n < 2)
        return;

    for (Uint32 i = 0; i < n - 1; i++)
    {
        for (Uint32 j = 0; j < n - 1; j++)
        {
            if (x[j].getString() > x[j+1].getString())
            {
                CIMName t = x[j];
                x[j] = x[j+1];
                x[j+1] = t;
            }
        }
    }
}

inline void BubbleSort(Array<CIMNamespaceName>& x)
{
    Uint32 n = x.size();

    if (n < 2)
        return;

    for (Uint32 i = 0; i < n - 1; i++)
    {
        for (Uint32 j = 0; j < n - 1; j++)
        {
            if (x[j].getString() > x[j+1].getString())
            {
                CIMNamespaceName t = x[j];
                x[j] = x[j+1];
                x[j+1] = t;
            }
        }
    }
}

#if 0
// Determine need for these functions
template<class PEGASUS_ARRAY_T>
void Unique(Array<PEGASUS_ARRAY_T>& x)
{
    Array<PEGASUS_ARRAY_T> result;

    for (Uint32 i = 0, n = x.size(); i < n; i++)
    {
        if (i == 0 || x[i] != x[i-1])
            result.append(x[i]);
    }

    x.swap(result);
}

template<class PEGASUS_ARRAY_T>
void Print(Array<PEGASUS_ARRAY_T>& x)
{
    for (Uint32 i = 0, n = x.size(); i < n; i++)
        PEGASUS_STD(cout) << x[i] << PEGASUS_STD(endl);
}
#endif

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ArrayInternal_h */
