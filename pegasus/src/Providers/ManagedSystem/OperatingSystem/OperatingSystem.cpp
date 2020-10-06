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
// Author: Mike Brasher (mbrasher@bmc.com)
//       : Denise Eckstein (denise_eckstein@hp.com)
//
// Modified By:  Susan Campbell, Hewlett-Packard Company <scampbell@hp.com>
//               k. v. le   <ikhanh@us.ibm.com>
//              Sean Keenan, Hewlett-Packard Company (sean.keenan@hp.com)
//              Jim Wunderlich (Jim_Wunderlich@prodigy.net)
//               Marek Szermutzky, IBM, (mszermutzky@de.ibm.com)
//
//%/////////////////////////////////////////////////////////////////////////////
#include <Pegasus/Common/Config.h>

PEGASUS_USING_PEGASUS;

#if defined(PEGASUS_PLATFORM_HPUX_ACC)
# include "OperatingSystem_HPUX.cpp"
#elif defined (PEGASUS_OS_TYPE_WINDOWS)
# include "OperatingSystem_Windows.cpp"
#elif defined (PEGASUS_PLATFORM_LINUX_GENERIC_GNU)
# include "OperatingSystem_Linux.cpp"
#elif defined (PEGASUS_OS_AIX)
# include "OperatingSystem_AIX.cpp"
#elif defined (PEGASUS_OS_DARWIN)
#include "OperatingSystem_DARWIN.cpp"
#elif defined (PEGASUS_OS_VMS)
# include "OperatingSystem_Vms.cpp"
#elif defined (PEGASUS_OS_SOLARIS)
# include "OperatingSystem_Solaris.cpp"
#elif defined (PEGASUS_OS_ZOS)
# include "OperatingSystem_zOS.cpp"
#elif defined (PEGASUS_OS_PASE)
# include "OperatingSystem_PASE.cpp"
#else
# include "OperatingSystem_Stub.cpp"
#endif
