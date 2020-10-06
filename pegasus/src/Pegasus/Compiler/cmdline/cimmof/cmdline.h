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

//
// Constants for use by cmdline.cpp
//

#ifndef _CMDLINE_CMDLINE_H_
#define _CMDLINE_CMDLINE_H_

#include <Pegasus/Common/String.h>

enum opttypes {FILESPEC,
           HELPFLAG,
           INCLUDEPATH,
           SUPPRESSFLAG,
           NAMESPACE,
           REPOSITORYDIR,

           UPDATEFLAG,
           ALLOWFLAG,
#ifndef PEGASUS_OS_HPUX
           SYNTAXFLAG,
//PEP167     FILELIST,
           TRACEFLAG,
           XMLFLAG,
#endif
#ifdef PEGASUS_OS_PASE
           QUIETFLAG, //PASE env ship q option
#endif
#ifdef PEGASUS_ENABLE_MRR_GENERATION
           MRRFLAG,
           DISCARDFLAG,
#endif
           VERSIONFLAG,
           OPTEND_CIMMOF,    //PEP167
           REPOSITORYNAME,
           REPOSITORYMODE,
           NO_USAGE_WARNING,
           OPTEND_CIMMOFL};  //PEP167

struct optspec
{
    char *flag;
    opttypes catagory;
    int islong;
    int needsvalue;
};

// Wrap this around the PEGASUS_HOME define for OS/400

#define PEGASUS_HOME "PEGASUS_HOME"

#define PEGASUS_CIMMOF_CANNOT_CONNECT_EXCEPTION  -10
#define PEGASUS_CIMMOF_NO_DEFAULTNAMESPACEPATH    -9
#define PEGASUS_CIMMOF_COMPILER_GENERAL_EXCEPTION -8
#define PEGASUS_CIMMOF_BAD_FILENAME               -7
#define PEGASUS_CIMMOF_PARSING_ERROR              -6
#define PEGASUS_CIMMOF_PARSER_LEXER_ERROR         -5
#define PEGASUS_CIMMOF_UNEXPECTED_CONDITION       -4
#define PEGASUS_CIMMOF_CMDLINE_NOREPOSITORY       -3
#define PEGASUS_CIMMOF_CIM_EXCEPTION              -2

#define ROOTCIMV2 "root/cimv2"
#define REPOSITORY_NAME_DEFAULT "repository"
#define REPOSITORY_MODE_DEFAULT "XML"
#endif
