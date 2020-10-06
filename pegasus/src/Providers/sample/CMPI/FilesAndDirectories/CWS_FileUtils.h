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

#ifndef CWS_FILEUTILS_H
#define CWS_FILEUTILS_H

#include "../CWS_Util/cwsutil.h"
#include <Pegasus/Provider/CMPI/cmpidt.h>

#if defined SIMULATED
# define CWS_FILEROOT  "/Simulated/CMPI/tests/"
#else
# define CWS_FILEROOT  "/tmp"
#endif

#if defined CWS_DEBUG
# define SILENT 0
#else
# define SILENT 1
#endif

char * CSCreationClassName(void);
char * CSName(void);
char * FSCreationClassName(void);
char * FSName(void);


CMPIObjectPath *makePath(const CMPIBroker *broker, const char *classname,
                         const char *Namespace, CWS_FILE *cwsf);
CMPIInstance   *makeInstance(const CMPIBroker *broker, const char *classname,
                             const char *Namespace, CWS_FILE *cwsf);
int             makeFileBuf(const CMPIInstance *instance, CWS_FILE *cwsf);
int silentMode(void);

#endif
