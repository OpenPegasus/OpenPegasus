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

#include "CIMFlavor.h"
#include <Pegasus/Common/InternalException.h>

PEGASUS_NAMESPACE_BEGIN

const CIMFlavor CIMFlavor::NONE = 0;
const CIMFlavor CIMFlavor::OVERRIDABLE = 1;
const CIMFlavor CIMFlavor::ENABLEOVERRIDE = 1;
const CIMFlavor CIMFlavor::TOSUBCLASS = 2;
const CIMFlavor CIMFlavor::TOINSTANCE = 4;
const CIMFlavor CIMFlavor::TRANSLATABLE = 8;
const CIMFlavor CIMFlavor::TOSUBELEMENTS = TOSUBCLASS;
const CIMFlavor CIMFlavor::DISABLEOVERRIDE = 16;
const CIMFlavor CIMFlavor::RESTRICTED = 32;
const CIMFlavor CIMFlavor::DEFAULTS = OVERRIDABLE + TOSUBCLASS;


CIMFlavor::CIMFlavor ()
    : cimFlavor (CIMFlavor::NONE.cimFlavor)
{
}

CIMFlavor::CIMFlavor (const CIMFlavor & flavor)
    : cimFlavor (flavor.cimFlavor)
{
}

CIMFlavor::CIMFlavor (const Uint32 flavor)
    : cimFlavor (flavor)
{
    //
    //  Test that no undefined bits are set
    //
    //  Note that conflicting bits may be set in the Uint32 flavor
    //  For example, OVERRIDABLE and DISABLEOVERRIDE may both be set
    //  or TOSUBCLASS and RESTRICTED may both be set
    //  Currently, the flavor is not checked for these conflicts
    //  That is corrected later when a CIMQualifierDecl object is constructed
    //  with the CIMFlavor object
    //
    PEGASUS_ASSERT (flavor < 64);
}

CIMFlavor & CIMFlavor::operator= (const CIMFlavor & flavor)
{
    this->cimFlavor = flavor.cimFlavor;
    return *this;
}

void CIMFlavor::addFlavor (const CIMFlavor & flavor)
{
    this->cimFlavor |= flavor.cimFlavor;
}

void CIMFlavor::removeFlavor (const CIMFlavor & flavor)
{
    this->cimFlavor &= (~flavor.cimFlavor);
}

Boolean CIMFlavor::hasFlavor (const CIMFlavor & flavor) const
{
    return (this->cimFlavor & flavor.cimFlavor) == flavor.cimFlavor;
}

Boolean CIMFlavor::equal (const CIMFlavor & flavor) const
{
    return this->cimFlavor == flavor.cimFlavor;
}

CIMFlavor CIMFlavor::operator+ (const CIMFlavor & flavor) const
{
    return CIMFlavor(this->cimFlavor | flavor.cimFlavor);
}

String CIMFlavor::toString () const
{
    String tmp;

    if (this->hasFlavor (CIMFlavor::OVERRIDABLE))
        tmp.append("OVERRIDABLE ");

    if (this->hasFlavor (CIMFlavor::TOSUBCLASS))
        tmp.append("TOSUBCLASS ");

    if (this->hasFlavor (CIMFlavor::TOINSTANCE))
        tmp.append("TOINSTANCE ");

    if (this->hasFlavor (CIMFlavor::TRANSLATABLE))
        tmp.append("TRANSLATABLE ");

    if (this->hasFlavor (CIMFlavor::DISABLEOVERRIDE))
        tmp.append("DISABLEOVERRIDE ");

    if (this->hasFlavor (CIMFlavor::RESTRICTED))
        tmp.append("RESTRICTED ");

    if (tmp.size ())
        tmp.remove (tmp.size () - 1);

    return tmp;
}

PEGASUS_NAMESPACE_END
