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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Array_h
/**
This class provides a list of object types that are supported by the the ArrayInter object.
The Array class contains parameter names for all Array objects.
*/
#define Pegasus_Array_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Char16.h>
#include <Pegasus/Common/String.h>

PEGASUS_NAMESPACE_BEGIN

#include <Pegasus/Common/ArrayInter.h>

#define PEGASUS_ARRAY_T Boolean
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Uint8
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Sint8
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Uint16
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Sint16
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Uint32
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Sint32
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Uint64
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Sint64
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Real32
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Real64
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T Char16
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T String
#include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Array_h */
