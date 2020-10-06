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
// Author: Chip Vincent (cvincent@us.ibm.com)
//
// Modified By: Paulo Sehn    (paulo_sehn@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_WMIFlavor_h
#define Pegasus_WMIFlavor_h

//#include "Config.h"
#include <Pegasus/Common/CIMFlavor.h>

PEGASUS_NAMESPACE_BEGIN

class PEGASUS_WMIPROVIDER_LINKAGE WMIFlavor : public CIMFlavor
{
public:
   WMIFlavor(const CIMFlavor & flavor);
   WMIFlavor(const LONG l);

//   operator Uint32(void) const;
//   operator LONG(void) const;

   // this function maps the flavors values from CIM to WMI
   long getAsWMIValue ();

protected:
   WMIFlavor(void) { };

protected:
//   Uint32 _ui32;

};

PEGASUS_NAMESPACE_END

#endif
