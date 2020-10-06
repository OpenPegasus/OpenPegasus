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
// Header for a class to generate CIMValue objects from String values
//
//
//
// This is a generic parser class from which controllers for particular
// yacc parsers can be derived.  It keeps enough state information that
// you should be able to get by without a reentrant parser.  You should
// compile both parser and lexer with a C++ compiler, although there
// is no need to generate a C++ lexer.
//
// The include file and compile-from-String techniques used here are
// supported only by bison and flex.
//

#ifndef _PARSER_H_
#define _PARSER_H_

#include <Pegasus/Common/Config.h>
#include <cstdio>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Stack.h>
#include <Pegasus/Compiler/Linkage.h>
#include <Pegasus/Common/FileSystem.h>



PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

// #define  DEBUG_INCLUDE // enables include file processing debug printout

// Define symbol for Empty Array.  Actually signal that this is not an
// array.
#define CIMMOF_EMPTY_ARRAY -1

//// Enum to type the string values from nonNullConstantValue.
//// Used primarily to seperate Boolean and String types but test within
//// valueFactory to compare types with parsed types.
////
namespace strValTypeNS
{
    enum strValTypeEnum
    {
        NULL_VALUE = 0,
        INTEGER_VALUE,
        REAL_VALUE,
        STRING_VALUE,
        BOOLEAN_VALUE,
        CHAR_VALUE
    } ;
};

// Display char def for the names (Used in exception generation)
const char * strValTypeEnumToString(strValTypeNS::strValTypeEnum x);

//// Define a struct to contain the string value and type for
//// initializer values in the parser. This is used only by the grammar
//// today.
typedef struct typedInitializerValue
{
    enum InitValueType
    {
        CONSTANT_VALUE = 1,
        ARRAY_VALUE,
        REFERENCE_VALUE,
        NULL_VALUE
    } ;

    InitValueType type;
    const String *value;
    int nonNullParserType;
    void set(InitValueType t, String* v)
    {
        type = t;
        value = v;
    }
    void setNull()
    {
        type = NULL_VALUE;
        value = new String(String::EMPTY);
        nonNullParserType = strValTypeNS::NULL_VALUE;
    }
} TYPED_INITIALIZER_VALUE;

struct bufstate
{
    void *buffer_state; // the YY_BUFFER_STATE of the stacked context
    String filename;    // the name of the file open in the stacked context
    int    lineno;      // the line number of the file
    String filenamePath; //the path of the file open in the stacked context
};

class PEGASUS_COMPILER_LINKAGE  parser
{
    private:
        unsigned int _buffer_size;   // the value of the YY_BUFFER_SIZE macro
        Stack<bufstate*> _include_stack;  // a stack of YY_BUFFER_STATEs
        String _current_filename; // name of the file being parsed
        unsigned int _lineno;     // current line number in the file
        String _current_filenamePath; // path of the file being parsed
    protected:
        void push_statebuff(bufstate *statebuff)
        {
            _include_stack.push(statebuff);
        }
        bufstate *pop_statebuff();
    public:

        // Constructor, destructor
        parser() : _buffer_size(16384), _lineno(0) {;}
        virtual ~parser() {;}

        virtual int parse() = 0;    // call the parser main yy_parse()
        virtual int wrap();         // handle the end of the current stream

        // start parsing this file
        int setInputBufferFromName(const String &filename);
        virtual int setInputBuffer(const FILE *f,
                Boolean closeCurrent) = 0;  // start parsing this handle
        //  int setInputBuffer(const char *buf);   // start parsing this String
        virtual int setInputBuffer(void *buffstate,
                Boolean closeCurrent) = 0; // start parsing this buffer

        // given a file stream, treat it as an include file
        virtual int enterInlineInclude(const FILE *f) = 0;
        virtual int wrapCurrentBuffer() = 0;

        unsigned int get_buffer_size() { return _buffer_size; }
        void set_buffer_size(unsigned int siz) { _buffer_size = siz; }

        // We keep track of the filename associated with the current input
        // buffer so we can report on it.
        void set_current_filename(const String &filename)
        {
            _current_filename = filename;

#ifdef DEBUG_INCLUDE
            cout << "cimmof parser - setting path = "
                << get_current_filenamePath() << endl; // DEBUG
#endif // DEBUG_INCLUDE

            String includePathTemp = FileSystem::extractFilePath(filename);

            // ************************************************************
            // if the filename path consisted of just the file name
            // becasue it is in the current directory then extractFilePath
            // returns just the filename rather than "dot". The following
            // test is to prevent adding file names to the include path.
            // ****************************************************************
            if (includePathTemp == filename)
            {
                includePathTemp = ".";
            }

            set_current_filenamePath(includePathTemp);

#ifdef DEBUG_INCLUDE
            cout << "cimmof parser set filename = " << filename
                << " include path = " << get_current_filenamePath()
                << endl; // DEBUG
#endif //  DEBUG_INCLUDE
        }

        const String &get_current_filename() const { return _current_filename; }

        // We keep track of the filename path associated with the current input
        // buffer so we can use it to search for include files in that
        // same directory
        void set_current_filenamePath(const String &filenamePath)
        { _current_filenamePath = filenamePath; }

        const String &get_current_filenamePath() const
        {
            return _current_filenamePath;
        }

        // Ditto the line number
        void set_lineno(int n) { _lineno = n; }
        void increment_lineno() { ++_lineno; }
        unsigned int get_lineno() const { return _lineno; }

        // This is the main entry point for parser error logging
        virtual void log_parse_error(char *token, const char *errmsg) const;
};
#endif
