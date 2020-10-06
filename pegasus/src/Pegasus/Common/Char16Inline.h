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

#ifndef Pegasus_Char16Inline_h
#define Pegasus_Char16Inline_h

#ifdef PEGASUS_INTERNALONLY
# define PEGASUS_CHAR16_INLINE inline
#else
# define PEGASUS_CHAR16_INLINE /* */
#endif

PEGASUS_NAMESPACE_BEGIN

PEGASUS_CHAR16_INLINE Char16::Char16() : _code(0)
{
}

PEGASUS_CHAR16_INLINE Char16::Char16(Uint16 x) : _code(x)
{
}

PEGASUS_CHAR16_INLINE Char16::Char16(const Char16& x) : _code(x._code)
{
}

PEGASUS_CHAR16_INLINE Char16::~Char16()
{
}

PEGASUS_CHAR16_INLINE Char16& Char16::operator=(Uint16 x)
{
    _code = x;
    return *this;
}

PEGASUS_CHAR16_INLINE Char16& Char16::operator=(const Char16& x)
{
    _code = x._code;
    return *this;
}

PEGASUS_CHAR16_INLINE Char16::operator Uint16() const
{
    return _code;
}

PEGASUS_CHAR16_INLINE Boolean operator==(const Char16& x, const Char16& y)
{
    return Uint16(x) == Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator==(const Char16& x, char y)
{
    return Uint16(x) == Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator==(char x, const Char16& y)
{
    return Uint16(x) == Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator!=(const Char16& x, const Char16& y)
{
    return Uint16(x) != Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator!=(const Char16& x, char y)
{
    return Uint16(x) != Uint16(y);
}

PEGASUS_CHAR16_INLINE Boolean operator!=(char x, const Char16& y)
{
    return Uint16(x) != Uint16(y);
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Char16Inline_h */
