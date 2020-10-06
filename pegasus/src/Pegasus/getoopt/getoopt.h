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
// OpenPegasus, object-oriented, portable C++ command line parser.
//
// There are two classes involved:
//    getoopt which encapsulates three functions:
//        1. Registration of program-defined flags and their characteristics
//        2. Parsing of the command line IAW the flag definitions
//        3. Orderly retrieval of the command line components
//
//    Optarg which abstracts the idea of a command line argument.
//
// The details of these classes are discussed in the comments above each
// class.
//
// Note that this command line parser is tied to OpenPegasus at least in that
// it:
//     1. Uses the MessageLoaderParms to define specific error messages
//

#ifndef _GETOOPT_H_
#define _GETOOPT_H_

#include <stdio.h>
#include <iostream>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Exception.h>
#include <Pegasus/getoopt/Linkage.h>

PEGASUS_USING_STD;
PEGASUS_USING_PEGASUS;

/**
    The flagspec struct describes a program-defined command line option.
    The syntax of these options are the same as those understood by
    the standard C language routines getopt() and getopt_long()

    Command line options are named, and the name is prefaced by
    either a hyphen or by two hyphens.  Names prefixed by one
    hyphen are restricted to a length of one character.  There is
    no limit to the size of names prefixed by two hyphens.  The
    two-hyphen-named options are called "long" options.

    The argtype indicates whether the name should be bound to a value.

    The islong flag tells whether the option is long-named.

    The isactive flag tells whether the option should be considered
    during parsing.
 */
struct flagspec
{
    /** Name for the Option.  May be short-name (one character or
       long-name (multiple characters)
     */
    String name;

    /** The argtype indicates whether the name should be bound to a
        value:
            - 0 - Never has a value.
            - 1 - Always has a value.
            - 2 - Optionally has a value. Thistype is valid only with long-named
                options
    */
    int    argtype;

    /** Defines whether the option is long-named. True if is long-names
     */
    Boolean   islong;

    /** Defines whether option should be included in the parse. On
           unless specifically turned off by program
    */
    Boolean   active;
};

/** Class Optarg encapsulates a command line argument as it was
    parsed. If it has a name, it means that it is bound to that
    command line option. For example, if the command line were
         myprog --database=xyz mytable
    then the name "database" would be associated with the first argument.
    There would be no name associated with the second argument ("mytable").

    In the example above, the value property of the arguments would
    be "xyz" bound to the name "database" and "mytable" bound to a
    blank name.

    The option type further describes the binding:
      A FLAG means that the value is bound to a short-named option name (flag)
      A LONGFLAG means that the value is bound to a long-named option name
      REGULAR means that the argument value is not preceded by a flag
*/
class PEGASUS_GETOOPT_LINKAGE Optarg
{
public:
    /** Enumeration defining the binding */
    enum opttype
    {
        FLAG,          /**< Value bound to short-name option name */
        LONGFLAG,      /**< Value bound to a long-named option name */
        REGULAR        /**< Argument value is not preceded by a flag */
    };

private:
    String _name;
    opttype _opttype;
    String _value;

public:
    // Constructors and Destructor.  Default copying is OK for this class.
    Optarg();
    /** Constructor that defines each of the proerties of the Optarg
        @param name String defining long-name or short-name for this
                   Optarg
        @param type opttype defining whether this is a FLAG,
                   LONGFLAG or non-flag Optarg
        @param value String defining the value for this Optarg
     */
    Optarg(
        const String& name,
        opttype type,
        const String& value);
    /**Destructor*/
    ~Optarg();

    /** Method to set the Name property
        @param name String defining long or short name to set into
                    the Optarg
    */
    void setName(const String& name);

    /** Set the type property with the type argument value
        @param type opttype for this Optarg defining whether this a
           FLAG, LONGFLAG or non-flag Optarg
     */
    void setType(opttype type);

    /** Set the Value parameter of the OptArg
        @param value String containing value to be set
     */
    void setValue(const String& value);

    /** Get the name from the Optarg
       @return const String& Name of this Optarg
     */
    const String& getName() const;

    /** get the name from the Optarg using getopt terminology.
        Equilivant to getName().
        @sa getName()
        @return const String&
     */
    const String& getopt() const;

    /** get the type from the OptArg.
       @return opttype containing the type of this Optarg.
     */
    opttype getType() const;

    /** Test whether the opttype == "FLAG" or "LONGFLAG"
        @return Boolean true if opttype == "FLAG" or "LONGFLAG".
               Otherwise return false.
     */
    Boolean  isFlag() const;

    /** Test whether the opttype == "LONGFLAG"
        @return Boolean true if opttype == "LONGFLAG"
     */
    Boolean isLongFlag() const;

    /** Return the value component of the optarg as a String
        @return const String& containing the value for this Optarg
     */
    const String& Value() const;

    /** Exactly the same as the Value() method.  Returns the value\
        component of the optarg. This is just to use getopt
        terminology.
       @sa Value()
       @return const String& containing the value for this Optarg.
     */
    const String& optarg() const;

    /** Put the current value of the Optarg into the String
        parameter provided
        @param v String which contains the value of the Optarg upon
                return
     */
    void Value(String& v) const ;

    /** Return the Value of this Optarg converted to int or an
        exception if the Value string cannot be converted.
        @param v int containing the integer value if conversion
           successful
        @exception TypeMismatchException if the String contains non
           integer characters or is outside the range of int
     */
    void Value(int& v) const;

    /** Return the Value of this Optarg converted to  unsigned int
        or an exception if the Value string cannot be converted.
       @param v int containing the integer value if conversion
           successful
       @exception TypeMismatchException if the String contains non
           integer characters or is outside the range of unsigned
           int
     */
    void Value(unsigned int& v) const;

    /** Return the Value of this Optarg converted to  long
        or an exception if the Value string cannot be converted.
       @param v int containing the integer value if conversion
           successful
       @exception TypeMismatchException if the String contains non
           integer characters or is outside the range of long
     */
    void Value(long& v) const ;

    /** Return the Value of this Optarg converted to  unsigned long
        or an exception if the Value string cannot be converted.
       @param v int containing the integer value if conversion
           successful
       @exception TypeMismatchException if the String contains non
           integer characters or is outside the range of unsigned
           long
     */
    void Value(unsigned long& v) const;

    /** Return the Value of this Optarg converted to  unsigned long
        or an exception if the Value string cannot be converted.
       @param d double containing the integer value if conversion
           successful
       @exception TypeMismatchException if the String contains non
           integer characters or is outside the range of unsigned
           long
     */
    void Value(double& d) const;

    /** (DEBUG) Print the components of Optarg to the ostream
        defined in the os arguement
       @param os ostream containing the converted output.

       @return ostream&
     */
    ostream& print(ostream& os) const;
};


/** class getoopt (a portamentau of "getopt" and "oo") is a
    container for Optarg objects parsed from the command line.
    It provides methods for specifying command line options,
    executing the parse, and analyzing the results of the parse

    The idea is to be able to do getopt()-like things with it:
    <PRE>
        getoopt cmdline(optstring);
        cmdLine.parse (argc, argv);
        ... error handling
        for (getoopt::const_iterator it = cmdline.begin();
                it != cmdline.end();
                it++)
        {
          . . . (process an Optarg represented by *it.)
        }
    </PRE>
    or
    <PRE>
        getoopt cmdLineOpts = getoopt(optString);
        cmdLineOpts.addLongFlagspec("help", getoopt::NOARG);
        cmdLineOpts.parse (argc, argv);
        if (cmdLineOpts.hasErrors ())
        {
            throw CommandFormatException(getOpts.getErrorStrings()[0]);
        }
        for (it =  cmdLineOpts.first(); it < cmdLineOpts.last();it++)
        {
            . . .  (process the Optarg in cmdlineOpt[it]
        }
    </PRE>

    There are three steps in using this class to parse a set of
    input arguments:
      <OL>
      <LI> Initialize the getOpts class  and specify the
         command line options. Pass a String identical in format
         to the POSIX getopt() optstring to the object either in
         the constructor or by an explicit method to define
         short-name arguments. Long-named options to be included
         are inserted using the method addLongFilespec() to add
         them to the list of flagspecs.

      <LI> Parse the command line, normally defined by argc and
         argv with the same definition as C or C++ main(). This
         will almost always be cmdline.parse(argc, argv); You
         can check for errors (violations of the command line
         options specified) by calling hasErrors(). A
         description of the errors, is stored in a
         Array<String> which can be retrieved with getErrorStrings();
         You can also print the error strings with printErrors();

      <LI> Analyze the parsed data.  Either iterate through the
         the command line, or use indexes like this:
         <PRE>
         for (unsigned int i =  cmdLineOpts.first(); i < cmdLineOpts.last();i++)
         {
            if (cmdLineOpts[i].isLongFlag()) {
                . . . (test for and process particular longflag arg)
            }
            else if (!cmdLineOpts[i].isFlag())
            {
                . . . (Process REGULAR arg)
            }
            else         //cmdLineOpts[i].getType () == FLAG/
            {
                Uint32 c = cmdLineOpts[t].getopt()[0];
                switch (c)
                {
                    case 'a':
                        (process short arg a)
                        break;
                    . . .
                }
            }
         }
         </PRE>

         Any REGULAR arguments are moved to the end of the argument
         list as part of the parse operation.

         The parsed data can also be explored for named
         arguments in an adhoc fashion by calling:
         <PRE>
             isSet(flagName);
         </PRE>
         and
         <PRE>
             value(flagName);
         </PRE>

    </ol>
*/
class PEGASUS_GETOOPT_LINKAGE getoopt
{
public:
    /** typedef for Array of flagspecs defined for the parse */
    typedef Array<flagspec> Flagspec_List;

    /** typedef for Array containing errors generated by the
     *  parse */
    typedef Array<String>   Error_List;

    /** typedef for Array of arguments upon completion of the parse
       method. Note that REGULAR arguments in input command line
       arugment are at the end of the argument list
    */
    typedef Array<Optarg>   Arg_List;

    /**
        In the valid option definition string, following an option,
        indicates that the preceding option takes a required argument.
        Defined as ':' in getoopt.cpp
     */
    static const char GETOPT_ARGUMENT_DESIGNATOR;

public:
    /** Defines the argument type for options:

         - NOARG This option has no supplemental argument.
            equivalent to a single character withought the ":" in
            the opstring.

        -  MUSTHAVARG this option must have an argument on the
            command line (equivalent to the ":" in optstring.

        - OPTIONALARG - Used only with long-name arguments, it
            indicates that the argument is optional for this
            long-name option.
     */
    enum argtype
    {
        NOARG,        /**< option with no supplemental argument */
        MUSTHAVEARG,  /**< option MUST HAVE supplemental argument */
        OPTIONALARG   /**< option MAY have supplemental argument (long-name
                          argument only */
    };

    /** Constructor that can be initialized with an a optstring
        using the same format as POSIX getopt() to define the flags
        to define multiple short-name options
        @param optstring A string of recognized short-name option
           characters; if a character is followed by a colon, the
           option takes an argument. This String is the same form as
           the POSIX getopt() optString.
     */
    getoopt(const char* optstring = 0);

    /** Destructor
     */
    ~getoopt();

    /** Add one or more short-name flags to the optstring. Allows
        multiple addition of the same flagspec.
       @param opt String is a string of recognized option
           characters; if a character is followed by a colon, the
           option takes a supplemental argument
       @return Boolean true if successfully added
     */
    Boolean addFlagspec(const String& opt);

    /** Add a single short-name flag to the optstring. Allows adding
        all characters except '*' as the short-name
       @param opt char containing the short-name to be added
       @param hasarg Boolean, if true indicats that this flagspec
           has an additional argument.
       @return Boolean
     */
    Boolean addFlagspec(char opt, Boolean hasarg = false);

    /** Remove a short-name from the optstring
       @param opt char containing name to be removed.
       @return Boolean true if opt removed from optstring
     */
    Boolean removeFlagspec(char opt);

    /** Add a single long-name flag to the array of flagspecs
       @param name String containing name to be added to array of
                   flagspecs. Must be a long-name.
       @param type  argtype defining whether this long-name has
                    an argument and whether it is required or
                    optional

       @return True if added
     */
    Boolean addLongFlagspec(const String& name,  argtype type);

    /** Remove a single long-name from the array of flagspecs
       @param name String with name to be removed
       @return True if removed
     */
    Boolean removeLongFlagspec(const String& name);

    /** Get a pointer to the flagspec defined by the short-name
        defined on input
       @param c char containing the short-name for which the pointer
                is to be returned
       @return const flagspec* or zero if short-name not found
     */
    const flagspec* getFlagspec(char c);

    /** Get a pointer to the flagspec defined by the long-name
        defined on input
       @param s String containing the long-name for which the
                pointer is to be returned
       @return const flagspec* or zero if long-name not found
     */
    const flagspec* getFlagspec(const String& s);

    /** Parse the string of arguments defined by argc and argv based
        on the defined array of flagspecs. The parse produces a new
        list of arguments on output which can be analyzed in detail
        using a loop with the first() and last() methods or the
        iterator method to access each argument in the list. Note
        that flag args are pushed onto the stack, then regular args
        are appended, sorting them to the rear the way getopt()
        does.

        FUTURE: It would be nice to have something like getopt optind in
        getoopt that would tell the user where the regular arguments
        started in the produced argument list.

       @param argc int defining number of items in the array argv
       @param argv char** array of char* defining the arguments to
               be parsed (same as argv in C or c++ main)
       @return True if the parse was succesful or false if
               errors were found
     */
    Boolean parse(int argc, char** argv);

    /** Determine if there were errors in the parse executed with
        the parse method.
       @return Boolean returns true if there were no errors.
               Returns false if errors were found.  The errors can
               be explored or displayed with the printError
               or getErrorStrings() functions
     */
    Boolean hasErrors() const;

    /** Get a list of the Errors generated from the parse
       @return const Error_List&
     */
    const Error_List& getErrorStrings() const;

    /** Send the string of errors to the output stream defined by
        os. Note that each error is proceeded by the ">" character.
       @param os ostream to which the errors are sent
       @return ostream&
     */
    ostream& printErrors(ostream& os) const;

    /** Put the errors into the String defined by argument s.
       @param s String into which the errors are appended.
     */
    void printErrors(String& s) const;

    /** The number of arguments on the parsed input line to parse.
        used to define the for loop for parsing of the input line
        using indexes.
       @return The number of arguments on the parsed input line.
     */
    unsigned int size() const;

    /** The nth Optarg element in the arglist.
       @param n
       @return const Optarg& defined as the nth entry in the
               processed arglist
     */
    const Optarg& operator[](unsigned int n);

    /** The first element in the input line to be parsed (always 0).
        Sugar to make the for loop look clean
       @return unsigned int
     */
    unsigned int first() const;

    /** The last element to be processed on the input line (==
        size()
       @return unsigned int
     */
    unsigned int last() const;

    /** Returns number of times a short-name defined by opt appears
        in the argument set
       @param opt
       @return unsigned int
     */
    unsigned int  isSet(char opt) const;

    /** Returns number of times a long-name defined by opt appears
        in the argument set
       @param opt
       @return unsigned int
     */
    unsigned int  isSet(const String& opt) const;

    /** Returns the value contained in the nth instance of the flag
        defined by opt on the input line.
       @param opt char short-name for which the nth instance is
                  returned.
       @param idx counter defining which instance of the short-name
                  is to be returned

       @return const String& value found in the instance. Otherwise
               it returns an empty string.
     */
    const String& value(char opt, unsigned int idx = 0) const;

    /** Returns the value contained in the nth instance of the flag
        defined by opt on the input line.
       @param opt char short-name for which the nth instance is
                  returned.
       @param idx counter defining which instance of the short-name
                  is to be returned

       @return const String& value found in the instance. Otherwise
               it returns an empty string.
     */
    const String& value(const String& opt, unsigned int idx = 0) const;

    /** Returns the array of arguments
       @return const Arg_List&
     */
    const Arg_List& getArgs() const;

    /** Add an error string to the array of errors. Allows the user
        to define new errors.

        FUTURE: Have not really figured out how to use this
        effectively.

        @param errstr String defining error to be added
     */
    void  addError(const String& errstr);

    /** Returns the number of named arguments (flags)
        size() - flagent() == number of nonflag arguments.
       @return unsigned int
     */
    unsigned int flagcnt() const;

private:
    Flagspec_List  _flagspecs;
    Error_List     _errorStrings;
    Arg_List       _args;
    flagspec* getFlagspecForUpdate(char c);
    flagspec* getFlagspecForUpdate(const String& s);
    String emptystring;
    Optarg _emptyopt;
};

#endif
