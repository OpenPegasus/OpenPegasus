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
// Author: Michael E. Brasher
//
//%=============================================================================

#ifndef _StripCmd_h
#define _StripCmd_h

#include "Config.h"
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
//
// Usage: strip start-pattern end-pattern filenames...
//
// Strip removes all line between a starting and ending pattern.
// Consider this file for example called myfile.txt
//
//     This is line one
//     This is line two
//     Start blah blah blah
//     This is line three
//     End blah blah blah
//     This is line four
//
// The following removes all lines between the line which begins
// with "Start" and ends with "End":
//
//     mu strip Start End myfile.txt
//
// The resulting file looks like this:
//
//     This is line one
//     This is line two
//     This is line four
//
////////////////////////////////////////////////////////////////////////////////

int StripCmd(const vector<string>& args);

#endif /* _StripCmd_h */
