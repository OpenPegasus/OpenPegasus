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

// bug 4573 - cimmof include file search path processing is inadequate
//
// Bug 4573 changed the behavior of the processing for locating specified
//  include files. The new procssing is based on the include file processing
//  behaviour used by the C compiler.
//
//      The search path for included files previously was:
//          1. try to open the file in the current working directory.
//          2. process the include path array from the cimof(l) cmdline
//             processing which always include "dot" as a default search
//             path and then any paths specified on the command line
//             with the -I option.
//
//      The search path for included files now is:
//          1. try to open the file in the same directory as the current
//             file being processed.
//          2. process the include path array from the cimof(l) cmdline
//             processing which only includes paths specified on the
//             command line with the -I option.
//

//
// implementation of valueFactory
//
//
//
// implementation of those methods  of class parser which are not pure
// virtual
//

#include "parser.h"
//#include <sstream>
#include "parserExceptions.h"


//-----------------------------------------------------------------------
//Implementation of the parserTypeFunctions
//-----------------------------------------------------------------------
static const char* _parserTypeStrings[] =
{
   "Null","Integer", "Real", "String", "Boolean,", "Char"
};

static const Uint32 _NUM_TYPES = sizeof(_parserTypeStrings) /
                                sizeof(_parserTypeStrings[0]);

const char* strValTypeEnumToString(strValTypeNS::strValTypeEnum type)
{
    PEGASUS_ASSERT(Uint32(type) < _NUM_TYPES);
    return _parserTypeStrings[Uint32(type)];

}
//---------------------------------------------------------------------
// Take a YY_BUFFERSTATE off the stack of saved contexts
//---------------------------------------------------------------------
bufstate *
parser::pop_statebuff()
{
    bufstate *v = 0;
    if (!_include_stack.isEmpty())
    {
        v = _include_stack.top();
        _include_stack.pop();
    }
    return v;
}

//-------------------------------------------------------------------
// Create a flex input buffer from a String containing the file name
//-------------------------------------------------------------------
int parser::setInputBufferFromName(const String &filename)
{
#if defined (PEGASUS_OS_VMS)
    FILE *f = fopen(filename.getCString(),"r");
#else
    FILE *f = fopen(filename.getCString(),"rt");
#endif
    if (f)
    {
        set_current_filename(filename);
        set_lineno(1);
        return setInputBuffer(f, false);
    } else
    {
        return 1;
    }
}

//-----------------------------------------------------------------
// Handle the end of an input buffer.  Either there is saved context
// or there isn't.  If there is, restore the saved particulars
// of that context so we can keep on parsing
//-----------------------------------------------------------------
int parser::wrap()
{
    bufstate *v = pop_statebuff();
    if (v)
    {
        setInputBuffer(v->buffer_state, true);
        set_current_filename(v->filename);
        set_lineno(v->lineno);
        set_current_filenamePath(v->filenamePath);
        delete v;
        return 0;  // more data available
    } else
    {
        return 1;  // end of it all
    }
}

#ifdef PEGASUS_HAVE_NAMESPACES
using namespace ParserExceptions;
#endif /* PEGASUS_HAVE_NAMESPACES */

//----------------------------------------------------------------
// ATTN: P2 BB 2001 Log where an error occured.  This is lame, so it needs work
//
//----------------------------------------------------------------
void parser::log_parse_error(char *token, const char *errmsg) const
{
    char buf[40];
    sprintf(buf, "%u", _lineno);
    String s = _current_filename + ":" + buf + ": " + errmsg + " before `"
        + token + "'\n";
    throw ParserLexException(s);
}
