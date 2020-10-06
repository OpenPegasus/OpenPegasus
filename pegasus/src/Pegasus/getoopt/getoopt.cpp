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
// implementation of getoopt

#include <Pegasus/Common/PegasusVersion.h>
#include <Pegasus/Common/MessageLoader.h>

#include "getoopt.h"
#include <cctype>
#include <cstdlib>

PEGASUS_USING_STD;

//-----------------------------------------------------------------------
//              Implementation of class Optarg
// An Optarg is created for each parameter on the command line.
//-----------------------------------------------------------------------

// Constructors
//   Default Constructor
Optarg::Optarg()
    : _name(""),
      _opttype(REGULAR),
      _value("")
{
}

//   All-in-one
Optarg::Optarg(
    const String& name,
    opttype type,
    const String& value)
    : _name(name),
      _opttype(type),
      _value(value)
{
}

// Destructor
Optarg::~Optarg()
{
}

//-----------------------------------------------------------------------
//        Set the class members
//-----------------------------------------------------------------------

// Set the _name member
void Optarg::setName(const String& name)
{
    _name = name;
}

// Set the _opttype member
void Optarg::setType(opttype type)
{
    _opttype = type;
}

// Set the _value member
void Optarg::setValue(const String& value)
{
    _value = value;
}

//-----------------------------------------------------------------------
//      Retrieve class members
//-----------------------------------------------------------------------

//  Get the _name member
const String&
Optarg::getName() const { return _name; }

//  Get the _name member using getopt() terminology
const String& Optarg::getopt() const
{
    return _name;
}

//  Get the _type member
Optarg::opttype Optarg::getType() const
{
    return _opttype;
}

//-------------------------------------------------------------------
//             Ways to get the _value member
//-------------------------------------------------------------------

//  Return _value as const String ref
const String& Optarg::Value() const
{
    return _value;
}

//  Same thing as Value(), but using getopt() terminology
const String& Optarg::optarg() const
{
    return _value;
}

//  Fill in a caller-provided String
void Optarg::Value(String& s) const
{
    s = _value;
}

//  Fill in a caller-provided int with the integer conversion of the value.
void Optarg::Value(int& i) const
{
    CString cs = _value.getCString();
    const char* s = cs;
    Boolean valid = true;
    Uint32 j;
    for (j = 0; j < strlen (s); j++)
    {
        if ((!isdigit (s [j])) && (!isspace (s [j])) && (s [j] != '+') &&
            (s [j] != '-'))
        {
            valid = false;
            break;
        }
    }
    if (valid)
    {
        Sint64 i64;
        if ( !(sscanf (s, "%" PEGASUS_64BIT_CONVERSION_WIDTH "d", &i64)) ||
             (i64 != Sint64(Sint32(i64))) )
        {
            throw TypeMismatchException ();
        }

        i = Sint32(i64);
    }
    else
    {
        throw TypeMismatchException ();
    }
}

//  Fill in a caller-provided unsigned int
void Optarg::Value(unsigned int& i) const
{
    CString cs = _value.getCString();
    const char* s = cs;
    Boolean valid = true;
    Uint32 j;

    for (j = 0; j < strlen (s); j++)
    {
        if ((!isdigit (s [j])) && (!isspace (s [j])))
        {
            valid = false;
            break;
        }
    }
    if (valid)
    {
        Uint64 i64;
        if ( !(sscanf (s, "%" PEGASUS_64BIT_CONVERSION_WIDTH "u", &i64)) ||
             (i64 > 0xFFFFFFFF))
        {
            throw TypeMismatchException ();
        }

        i = Uint32(i64);
    }
    else
    {
        throw TypeMismatchException();
    }
}

//  Fill in a caller-provided long
void Optarg::Value(long& l) const
{
    l = (long)atoi(_value.getCString());
}

//  Fill in a caller-provided unsigned long
void Optarg::Value(unsigned long& l) const
{
    l = (unsigned long)atoi(_value.getCString());
}

//  Fill in a caller-provided double
void Optarg::Value(double& d) const
{
    d = (double)atof(_value.getCString());
}

//--------------------------------------------------------------------
//  Provide information about the flag, if any
//--------------------------------------------------------------------

// Is the option value is bound to a flag?
Boolean Optarg::isFlag() const
{
    return (_opttype == FLAG || _opttype == LONGFLAG);
}

// Is it bound to a long-named flag?
Boolean Optarg::isLongFlag() const
{
    return (_opttype == LONGFLAG);
}

//-----------------------------------------------------------------------
//  print the members as a formatted String
//-----------------------------------------------------------------------
ostream& Optarg::print(ostream& os) const
{
    os << "{name:(" << getName();
    os << ") type:(";
    switch (getType())
    {
    case FLAG: os << "FLAG"; break;
    case LONGFLAG: os << "LONGFLAG"; break;
    case REGULAR: os << "REGULAR"; break;
    }
    os << ") value:(" << Value() << ")}";
    return os;
}



//---------------------------------------------------------------------
//                   Implementation of class getoopt
//---------------------------------------------------------------------

// Constructors and destructor

// Default constructor.  The optional String is in the format of
// a getopt() optstring
getoopt::getoopt(const char* optstring)
{
    if (optstring)
    {
        addFlagspec(optstring);
    }
}

getoopt::~getoopt()
{
}

//----------------------------------------------------------------------
//  methods to register program-defined flags and their characteristics
//  The flags are encapsulated in the struct flagspec, an array of which
//  is a member of this class.
//  There are also methods to deregister flags. This is done by resetting
//  a flagspec's active flag.
//----------------------------------------------------------------------

// Parse through a getopt() optstring and create flagspecs from each
// short flag.
Boolean getoopt::addFlagspec(const String& opt)
{
    unsigned int size = opt.size();
    if (size == 0)
    {
        return false;
    }
    for (unsigned int i = 0; i < size; i++)
    {
        char c = static_cast<char>(opt[i]);
        if ( ((i + 1) < size) && (opt[i+1] == ':') )
        {
            if (!(addFlagspec(c, true)))
            {
                return false;
            }
            ++i;
        }
        else
        {
            if (!(addFlagspec(c, false)))
                return false;
        }
    }
    return true;
}

//  Create a filespec from a single short flag and push it onto the array
Boolean getoopt::addFlagspec(char flag, Boolean hasarg)
{
    if (flag == '*')
    {
        MessageLoaderParms parms(
            "getoopt.getoopt.CANT_NAME_FLAG",
            "You can't have a flag named '$0'",
            flag);
        addError(MessageLoader::getMessage(parms));
        return false;
    }
    flagspec fs;
    char c[2];
    c[0] = flag;
    c[1] = 0;
    fs.name = c;
    fs.argtype = hasarg ? 1 : 0;
    fs.islong = false;
    fs.active = true;
    _flagspecs.append(fs);
    return true;
}

// Create a flagspec from a single long flag and push it onto the array
Boolean getoopt::addLongFlagspec(
    const String& name,
    argtype type)
{
    flagspec fs;

    // Changing "fs.name = name" to the following line masks an ugly crash
    // which occurs when compiling with debug option on WIN32:

    fs.name = name;

    fs.argtype = type;
    fs.islong = true;
    fs.active = true;
    _flagspecs.append(fs);
    return true;
}

// Unregister a flagspec
Boolean getoopt::removeFlagspec(char opt)
{
    flagspec* fs = getFlagspecForUpdate(opt);
    if (fs)
    {
        fs->active = false;
        return true;
    }
    return false;
}

/**
    In the valid option definition string, following an option,
    indicates that the preceding option takes a required argument.
 */
const char getoopt::GETOPT_ARGUMENT_DESIGNATOR = ':';

//--------------------------------------------------------------------
//      Routines for parsing the command line
//--------------------------------------------------------------------

//------------------------
// Static functions
//------------------------

// Parse out the flagname and the value from a long flag option that
// may be in the form
//          --longflag=value
static void partsFromLongOpt(
    const String& s,
    String& name,
    String& value)
{
    for (unsigned int i = 0; i < s.size(); i++)
    {
        if (s[i] == '=')
        {
            name = s.subString(0, i);
            value = s.subString(i+1);
            return;
        }
    }
    name = s;
    value =  "";
}

// Create an Optarg instance from a long flag String like
//          --longflag=value
// (The =value is optional).
static void optargFromLongOpt(
    Optarg& o,
    const String& arg)
{
    String name;
    String value;
    partsFromLongOpt(arg, name, value);
    o.setName(name);
    o.setType(Optarg::LONGFLAG);
    o.setValue(value);
}

// Create an Optarg instance from a short flag String like
//      -fValue
// (The Value part is optional)
static void optargFromShortOpt(
    Optarg& o,
    const char* arg)
{
    char name[2];
    name[0] = arg[0];
    name[1] = 0;
    o.setName(name);
    o.setType(Optarg::FLAG);
    const char* p = arg + 1;
    o.setValue(p);
}

// Look at a command line option and determine whether it is a
// long flag, a short flag or an unflagged option.
static int catagorize(const char* s)
{
    if (s[0] != '-')
    {
        return 0;
    }
    else
    {
        if (s[1] == '-')
        {
            return 2;
        }
    }
    return 1;
}

// Push an Optarg onto our array
static void addarg(
    getoopt::Arg_List&list,
    const Optarg& o)
{
    //o.print(cout);
    list.append(o);
}

// Create an Optarg from its members and push it onto the array
static void addarg(
    getoopt::Arg_List& list,
    const String& name,
    Optarg::opttype type,
    const String& value)
{
    Optarg* o = new Optarg(name, type, value);
    addarg(list, *o);
    delete o;
}

// Take an array of arguments and append it to another
static void copyargs(
    getoopt::Arg_List& out,
    const getoopt::Arg_List& in)
{
    Uint32 size = in.size();
    for (Uint32 i = 0; i < size; i++)
    {
        addarg(out, in[i]);
    }
}

//------------------------------------
// The parse method:  Way too long.
// Note that flag args are pushed
// onto the stack, then the regular
// args are appended, sorting them
// to the rear the way getopt() does.
//------------------------------------
Boolean getoopt::parse(
    int argc,
    char** argv)
{
    Optarg o;
    int cat;
    const flagspec* fs;
    Arg_List nonflagargs;
    enum States {START, ARGEXPECTED};
    States state = START;
    for (unsigned int i = 1; i < (unsigned int)argc; i++)
    {
        unsigned int endsize = static_cast<unsigned int>(strlen(argv[i]));
        switch (state)
        {
        case START:
            cat = catagorize(argv[i]);
            switch (cat)
            {
            case 0: // non-flag command line argument
                addarg(nonflagargs, "", Optarg::REGULAR, argv[i]);
                break;
            case 1: // short (1-character) flag
            {
                if(endsize == 1)
                {
                    MessageLoaderParms parms(
                        "getoopt.getoopt.MISSING_OPTION",
                        "Missing required option for $0",
                        argv[i]);
                    addError(MessageLoader::getMessage(parms));
                    break;
                }
                unsigned int argpos = 1;
                while (argpos < endsize)
                {
                    char c = argv[i][argpos];
                    fs = getFlagspec(c);  // Short flag
                    String temp = argv[i];
                    String name = temp.subString(argpos, 1);
                    // See if we recognize it
                    if (!fs)
                    {
                        MessageLoaderParms parms(
                            "getoopt.getoopt.UNKNOWN_FLAG",
                            "Unknown flag $0$1",
                            "-",
                            name);
                        addError(MessageLoader::getMessage(parms));
                        argpos++;
                    }
                    else
                    {
                        // Should this flag be bound?
                        if (fs->argtype == NOARG)
                        {
                            // NO
                            addarg(_args, name, Optarg::FLAG,  "");
                            argpos++;
                        }
                        else
                        {
                            // YES -- the value is here or in the next arg
                            optargFromShortOpt(o, &argv[i][argpos]);
                            if (o.Value() == "")
                            {
                                // No value yet
                                state = ARGEXPECTED;
                            }
                            else
                            {
                                addarg(_args, o);
                            }
                            argpos = endsize;
                        }
                    }
                }
                break;
            } // end subcase 1
            case 2:  // long (--xyz) flag
            {
                String arg = &(argv[i][2]);
                optargFromLongOpt(o, arg);
                fs = getFlagspec(o.getName());
                // see if we recognize this flag
                if (!fs)
                {
                    MessageLoaderParms parms(
                        "getoopt.getoopt.UNKNOWN_FLAG",
                        "Unknown flag $0$1",
                        "",
                        o.getName());
                    addError(MessageLoader::getMessage(parms));
                }
                else
                {
                    // this is a long flag we know about
                    if (o.optarg() != ""  || fs->argtype != MUSTHAVEARG)
                    {
                        addarg(_args, o);
                        state = START;  // we have a completed long flag
                    }
                    else
                    {
                        // no value yet, and we expect one
                        if (fs->argtype == MUSTHAVEARG)
                        {
                            state = ARGEXPECTED;
                        }
                    }
                }
                break;
            } // end subcase 2
        } // end switch catagorize()
        break; // end of case START

        case ARGEXPECTED:
            if (argv[i][0] == '-')
            {
                MessageLoaderParms parms(
                    "getoopt.getoopt.MISSING_VALUE_FOR_FLAG",
                    "Missing required value for flag $0",
                    o.getopt());
                addError(MessageLoader::getMessage(parms));
                i--;
            }
            else
            {
                o.setValue(argv[i]);
            }
            addarg(_args, o);
            state = START;
            break;
        } // end switch
    } // end for
    if (state != START)
    {
        MessageLoaderParms parms(
            "getoopt.getoopt.MISSING_VALUE_FOR_FLAG",
            "Missing required value for flag $0",
            o.getName());
        addError(MessageLoader::getMessage(parms));
    }
    copyargs(_args, nonflagargs);
    return !_errorStrings.size();
}

//----------------------------------------------------------------------
//         Methods to retrieve the command line arguments
//----------------------------------------------------------------------

//----------------------------------------------
// Access the command line arguments by index
//----------------------------------------------

// Index operator
const Optarg& getoopt::operator[](unsigned int n)
{
    unsigned int lim = _args.size();
    if (n < lim)
    {
        return _args[n];
    }
    else
    {
        return _emptyopt;
}
}

// Return first index
unsigned int getoopt::first() const
{
    return 0;
}

// Return one past last index
unsigned int getoopt::last() const
{
    return _args.size();
}

//-----------------------------------------------
// Access the command line arguments ad-hoc
//-----------------------------------------------

// Return the number of times a short flag is set
// on the command line
unsigned int getoopt::isSet(char c) const
{
    unsigned int cnt = 0;
    for (unsigned int i = 0; i < _args.size(); i++)
    {
        const Optarg& o = _args[i];
        if (o.getType() == Optarg::FLAG)
        {
            const String& s = o.getopt();
            if (s[0] == c)
            {
                cnt++;
            }
        }
    }
    return cnt;
}

// Return the number of times any flag is set
// on the command line
unsigned int getoopt::isSet(const String& s) const
{
    unsigned int cnt = 0;
    for (unsigned int i = 0; i < _args.size(); i++)
    {
        const Optarg& o = _args[i];
        if (o.getopt() == s)
        {
            cnt++;
        }
    }
    return cnt;
}

// Return the String value of the nth instance of
// a particular short flag on the command line
const String& getoopt::value(
    char opt,
    unsigned int idx) const
{
    unsigned int cnt = 0;
    for (unsigned int i = 0; i < _args.size(); i++)
    {
        const Optarg& o = _args[i];
        if (o.getType() == Optarg::FLAG)
        {
            const String& s = o.getopt();
            if (s[0] == opt)
            {
                if (cnt == idx)
                {
                    return o.optarg();
                }
                else
                {
                    cnt++;
                }
            }
        }
    }
    return emptystring;
}

// Return the nth instance of any flag on the command line
const String& getoopt::value(
    const String& opt,
     unsigned int idx) const
{
    unsigned int cnt = 0;
    for (unsigned int i = 0; i < _args.size(); i++)
    {
        const Optarg& o = _args[i];
        if (o.optarg() == opt)
        {
            if (cnt == idx)
            {
                return o.getopt();
            }
            else
            {
                cnt++;
            }
        }
    }
    return emptystring;
}

// Of the command line arguments, how many are flags?
unsigned int getoopt::flagcnt() const
{
    unsigned int cnt = 0;
    for (Uint32 i = 0; i < _args.size(); i++)
    {
        if (_args[i].getType() != Optarg::REGULAR)
            cnt++;
    }
    return cnt;
}

// How many command line arguments were there?
unsigned int getoopt::size() const
{
    return _args.size();
}

// Return the list of command line arguments for use by
// the program.
const getoopt::Arg_List& getoopt::getArgs() const
{
    return _args;
}


//-----------------------------------------------------------
// Routines dealing with errors during parsing
// FIXME:  This needs to be reworked so that the error text
// is hidden and provided by the caller
//----------------------------------------------------------

// Add an error into the list
void getoopt::addError(const String& s)
{
    _errorStrings.append(s);
}

// Return a list of the errors
const getoopt::Error_List& getoopt::getErrorStrings() const
{
    return _errorStrings;
}

// Did any errors occur?
Boolean getoopt::hasErrors() const
{
    return _errorStrings.size() ? true : false;
}



flagspec* getoopt::getFlagspecForUpdate(const String& s)
{
    for (unsigned int i = 0; i < _flagspecs.size(); i++)
    {
        flagspec& o = _flagspecs[i];
        if (o.islong && s == o.name)
        {
            return &_flagspecs[i];
        }
    }
    return 0;
}

const flagspec* getoopt::getFlagspec(const String& s)
{
    return (const flagspec *)getFlagspecForUpdate(s);
}

ostream& getoopt::printErrors(ostream& os) const
{
    for (Uint32 i = 0; i < _errorStrings.size(); i++)
    {
        os << "> " << _errorStrings[i] << endl;
    }
    return os;
}

void getoopt::printErrors(String& s) const
{
    for (Uint32 i = 0; i < _errorStrings.size(); i++)
    {
        s.append("> " + _errorStrings[i] + "\n");
    }
}

//---------------------------------------------------------------
//              Private methods
//---------------------------------------------------------------
flagspec* getoopt::getFlagspecForUpdate(char c)
{
    for (unsigned int i = 0; i < _flagspecs.size(); i++)
    {
        flagspec& o = _flagspecs[i];
        if (!o.islong && c == o.name[0])
        {
            return &_flagspecs[i];
        }
    }
    return 0;
}

const flagspec* getoopt::getFlagspec(char c)
{
    return (const flagspec *)getFlagspecForUpdate(c);
}
