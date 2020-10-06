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
// Author: Jair Santos, Hewlett-Packard Company (jair.santos@hp.com)
//
// Modified By: Terry Martin, Hewlett-Packard Company (terry.martin@hp.com)
//
//%////////////////////////////////////////////////////////////////////////////

#ifndef _WMIop_h_
#define _WMIop_h_

#include <Pegasus/Client/CIMClient.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

void _usage();
void _giUsage();
void _diUsage();
void _gpUsage();
void _spUsage();
void _miUsage();
void _ciUsage();
int _getClass(const int, const char **);
int _enumerateClasses(const int, const char **);
int _enumerateClassNames(const int, const char **);
int _getInstance(const int, const char **);
int _enumerateInstances(const int, const char **);
int _enumerateInstanceNames(const int, const char **);
int _getProperty(const int, const char **);
int _setProperty(const int, const char **);
int _invokeMethod(const int, const char **);
int _createClass(const int, const char **);
int _modifyClass(const int, const char **);
int _deleteClass(const int, const char **);
int _createInstance(const int, const char **);
int _modifyInstance(const int, const char **);
int _deleteInstance(const int, const char **);
int _associators(const int, const char **);
int _associatorNames(const int, const char **);
int _references(const int, const char **);
int _referenceNames(const int, const char **);
int _execQuery(const int, const char **);
int _getQualifier(const int, const char **);
int _setQualifier(const int, const char **);
int _deleteQualifier(const int, const char **);
int _enumerateQualifiers(const int, const char **);
void _displayInstance(CIMInstance&);
Array<CIMKeyBinding> _inputInstanceKeys(CIMClass&);
CIMObjectPath _selectInstance(const String&);
Boolean _isKey(const CIMProperty&);
String _displayProperty(const CIMProperty&);
CIMValue _makeValue(const char*, const CIMProperty&);
CIMKeyBinding _makeKey(const CIMProperty &);
int _recursiveClassEnum(const CIMName&, String);
CIMClass _makeNamespaceClass(void);
Boolean verifyServerCertificate(SSLCertificateInfo &certInfo);

#endif
