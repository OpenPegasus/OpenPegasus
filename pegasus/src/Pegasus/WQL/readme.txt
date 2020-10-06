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

This directory defines a library to be used to compile WQL1 query
statement (which may only contain a simple SELECT statement).

The main interface is in <Pegasus/WQL/WQLParser.h> which contains an
example.

See program under tests/Parser for an example.

To generate documentation under:

    <pegasus-root>/pegasus/doc/reference/Pegasus/WQL/index.html

Type "make docxx".

WARNING: Compiling the grammar.  The Makefile includes a section to
compile the grammar that is defined in WQL.l and WQL.y.  This Makefile
allows either lex and yacc or flex/bision tools to be used for the
compilation.

However, we have noted a significant difference in the various versions
of bison and flex.  We recommend that anybody compiling the grammar for
WQL use an up-to-date version of lex/yacc or flex/bison.  In particular,
the versions generally available in windows have proven to be out of date
and created several questionable output structures.

This has been successfully compiled on current Linux platforms using
bison 1.87 and flex verison 2.5.4.



