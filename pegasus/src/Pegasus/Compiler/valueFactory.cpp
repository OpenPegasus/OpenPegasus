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
// implementation of valueFactory
//

#include <cstring>
#include <cstdlib>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/StringConversion.h>
#include "cimmofMessages.h"
#include "cimmofParser.h"  /* unfortunately.  Now that valueFactory needs
                              to know about cimmofParser, it might as well
                              be rolled into it. */
#include "valueFactory.h"
#include "parser.h"

extern char *cimmof_text;

// put any debug include, I'd say about here

#define local_min(a,b) ( a < b ? a : b )
#define local_max(a,b) ( a > b ? a : b )

/* Fix up a string with embedded comma with extra
   escape character and return the result. This
   gets around the problem that arrays having strings
   with an embedded comma treat the embedded comma
   as an array item separator.
   NOTE: The correct solution is to add a new value factory
   function for arrays specifically that uses a different
   separator on an array of values, one that cannot be found
   in the normal input.  This is difficult because we do not have
   any set of truly illegal characters.
   BUG 497 fix
*/
String valueFactory::stringEscapeComma(String str)
{
    String rtn = String::EMPTY;
    Uint32 len;
    while((len = str.find(',')) != PEG_NOT_FOUND)
    {
        // if escape char found prepend escape character.
        rtn.append(str.subString(0,len));
        rtn.append("\\,");
        str = str.subString(len+1);
    }
    if (str.size() > 0)
    {
        rtn.append(str);
    }
    return(rtn);
}

Uint64 valueFactory::stringToUint(
    const String &val,
    CIMType type)
{
    Uint64 u64;
    CString valCString = val.getCString();

    Boolean success =
        (StringConversion::decimalStringToUint64(valCString, u64) ||
         StringConversion::hexStringToUint64(valCString, u64) ||
         StringConversion::octalStringToUint64(valCString, u64) ||
         StringConversion::binaryStringToUint64(valCString, u64)) &&
        StringConversion::checkUintBounds(u64, type);

    if (!success)
    {
        String message;
        cimmofMessages::arglist arglist;
        arglist.append(cimTypeToString(type));
        arglist.append(val);
        cimmofMessages::getMessage(
            message, cimmofMessages::INVALID_LITERAL_VALUE, arglist);

        cimmofParser::Instance()->log_parse_error(
           cimmof_text, message.getCString());
    }

    return u64;
}

Sint64 valueFactory::stringToSint(
    const String &val,
    CIMType type)
{
    Sint64 s64;
    CString valCString = val.getCString();

    Boolean success =
        (StringConversion::stringToSint64(
             valCString, StringConversion::decimalStringToUint64, s64) ||
         StringConversion::stringToSint64(
             valCString, StringConversion::hexStringToUint64, s64) ||
         StringConversion::stringToSint64(
             valCString, StringConversion::octalStringToUint64, s64) ||
         StringConversion::stringToSint64(
             valCString, StringConversion::binaryStringToUint64, s64)) &&
        StringConversion::checkSintBounds(s64, type);

    if (!success)
    {
        String message;
        cimmofMessages::arglist arglist;
        arglist.append(cimTypeToString(type));
        arglist.append(val);
        cimmofMessages::getMessage(
            message, cimmofMessages::INVALID_LITERAL_VALUE, arglist);

        cimmofParser::Instance()->log_parse_error(
           cimmof_text, message.getCString());
    }

    return s64;
}

Real64 valueFactory::stringToReal(
    const String &val,
    CIMType type)
{
    Real64 r64;
    Boolean success = StringConversion::stringToReal64(val.getCString(), r64);

    if (!success)
    {
        String message;
        cimmofMessages::arglist arglist;
        arglist.append(cimTypeToString(type));
        arglist.append(val);
        cimmofMessages::getMessage(
            message, cimmofMessages::INVALID_LITERAL_VALUE, arglist);

        cimmofParser::Instance()->log_parse_error(
           cimmof_text, message.getCString());
    }

    return r64;
}

/*
    Test for valid string value for boolean type and return boolean value.
    Generates log_pars_error if not valid string value.
    NOTE: The boolean type was forced to T or F string valuein the
    parser (cimmof.y)
*/
Boolean _stringToBoolean(const String& sval)
{
    Boolean rtn = false;

    if (sval == "T")
    {
            rtn = true;
    }
    else if ((sval == "F"))
    {
        rtn = false;
    }
    else
    {
        // Output message
        String message;
        cimmofMessages::arglist arglist;
        arglist.append(cimTypeToString(CIMTYPE_BOOLEAN));
        arglist.append("Value not valid boolean");
        cimmofMessages::getMessage(
            message, cimmofMessages::INVALID_LITERAL_VALUE, arglist);

        cimmofParser::Instance()->log_parse_error(
           cimmof_text, message.getCString());
    }
    return rtn;
}

//-------------------------------------------------------------------------
// Parser for a comma-separated value String.  It returns one
// value per call.  It handles quoted String and depends on the caller to
// tell it where the end of the String is.
// Returns value in value and return pointing to character after separator
// string
// Returns the next substring based on the separator character and returns
// the start state for the next call.
//-------------------------------------------------------------------------
static Uint32 nextcsv(const String &csv, int sep, const Uint32 start,
    const Uint32 end, String &value)
{
    enum parsestate {INDQUOTE, INSQUOTE, NOTINQUOTE};
    value = "";
    Uint32 maxend = local_min(csv.size(), end);
    Uint32 idx = start;
    parsestate state = NOTINQUOTE;
    // ATTN-RK-P3-071702: Added hack to check for null character because Strings
    // were sometimes getting created that included an extra null character.
    while (idx <= maxend && csv[idx])
    {
        char idxchar = csv[idx];
        switch (state)
        {
            case NOTINQUOTE:
                switch (idxchar)
                {
                    case '\\':
                        state = INSQUOTE;
                        break;
                    case '"':
                        state = INDQUOTE;
                        break;
                    default:
                        if (idxchar == sep)
                            return idx + 1;
                        else
                            value.append(idxchar);
                        break;
                }
                break;
            // Transient state that starts with the escape character
            // and appends the next character and returns to NOTINQUOTE.
            case INSQUOTE:
                value.append(idxchar);
                state = NOTINQUOTE;
                break;
            case INDQUOTE:
                switch (idxchar)
                {
                    case '"':
                        state = NOTINQUOTE;
                        break;
                    default:
                        value.append(idxchar);
                        break;
                }
        }
        idx++;
    }   // end while
    return idx;
}

// ------------------------------------------------------------------
// When the value to be built is of Array type, this routine
// parses out the comma-separated values and builds the array
// -----------------------------------------------------------------
CIMValue* valueFactory::_buildArrayValue(
    CIMType type,
    const String& rep)
{
    String sval;
    Uint32 start = 0;
    Uint32 strsize = rep.size();
    Uint32 end = strsize;

    switch (type)
    {
        case CIMTYPE_BOOLEAN:
            {
                Array<Boolean> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append(_stringToBoolean(sval));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_UINT8:
            {
                Array<Uint8> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Uint8)stringToUint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_SINT8:
            {
                Array<Sint8> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Sint8)stringToSint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_UINT16:
            {
                Array<Uint16> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Uint16)stringToUint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_SINT16:
            {
                Array<Sint16> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Sint16)stringToSint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_UINT32:
            {
                Array<Uint32> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Uint32)stringToUint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_SINT32:
            {
                Array<Sint32> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Sint32)stringToSint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_UINT64:
            {
                Array<Uint64> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Uint64)stringToUint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_SINT64:
            {
                Array<Sint64> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Sint64)stringToSint(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_REAL32:
            {
                Array<Real32> a;
                if (strsize != 0)
                {
                    do
                    {
                        start = nextcsv(rep, ',', start, end, sval);
                        a.append((Real32)stringToReal(sval, type));
                    } while (start < end);
                }
                return new CIMValue(a);
            }
        case CIMTYPE_REAL64:
             {
                 Array<Real64> a;
                 if (strsize != 0)
                 {
                     do
                     {
                         start =
                             nextcsv(rep, ',', start, end, sval);
                         a.append((Real64)stringToReal(sval, type));
                     } while (start < end);
                 }
                 return new CIMValue(a);
             }
        case CIMTYPE_CHAR16:
             {
                 Array<Char16> a;
                 if (strsize != 0)
                 {
                     do
                     {
                         start =
                            nextcsv(rep, ',', start, end, sval);
                         a.append(sval[0]);
                     } while (start < end);
                 }
                 return new CIMValue(a);
             }
        case CIMTYPE_STRING:
             {
                 Array<String> a;
                 if (strsize != 0)
                 {
                     do
                     {
                         start =
                            nextcsv(rep, ',', start, end, sval);
                         a.append(sval);
                     } while (start < end);
                 }
                 return new CIMValue(a);
             }
        case CIMTYPE_DATETIME:
             {
                 Array<CIMDateTime> a;
                 while (strsize &&
                         (start = nextcsv(rep, ',', start, end, sval)) < end )
                 {
                     a.append(CIMDateTime(sval));
                 }
                 return new CIMValue(a);
             }
        case CIMTYPE_REFERENCE:
             break;
             //  PEP 194:
             //  Note that "object" (ie. CIMTYPE_OBJECT) is not a real
             //  CIM datatype, just a Pegasus internal representation
             //  of an embedded object, so it won't be found here.
        case CIMTYPE_OBJECT:
        case CIMTYPE_INSTANCE:
             break;
    }  // end switch
    return 0;
}

/* ATTN: KS 20 Feb 02 - Think we need to account for NULL value here differently
   They come in as an empty string from devaultValue and if they are an empty
   string we need to create the correct type but without a value in it.
   Easiest may be to test in each converter since otherwise would have to
   create a second switch. Either that or if strlength = zero
   create an empty CIMValue and then put in the type
                CIMValue x;
            x.set(Uint16(9)
*/
//----------------------------------------------------------------

/*
    Test th expected parse type against the received parse type
    Return true if equal
*/
Boolean _testType(int expectedType, int parseValueType)
{
    return ((expectedType == parseValueType));
}

/*
    Compare the CIMType to the parse type for selected CIMTypes to assure
    that the type we parsed is the type we are expected to put into the
    result.  Note that this code was the result of bug 3574 where
    boolean and string types cannot be separated.
*/
Boolean valueFactory::compareTypeToParseType(CIMType type, int expectedType)
{
    Boolean rtn = true;
    switch(type)
    {
        case CIMTYPE_UINT8:
        case CIMTYPE_SINT8:
        case CIMTYPE_UINT16:
        case CIMTYPE_SINT16:
        case CIMTYPE_UINT32:
        case CIMTYPE_SINT32:
        case CIMTYPE_UINT64:
        case CIMTYPE_SINT64:
            {
                rtn = _testType(expectedType, strValTypeNS::INTEGER_VALUE);
            }
            break;
        case CIMTYPE_REAL32:
        case CIMTYPE_REAL64:
            {
                rtn =_testType(expectedType,strValTypeNS::REAL_VALUE);
            }
            break;
        case CIMTYPE_CHAR16:
            {
                //// TODO bypass this test because we apparently have other
                //// problems in system with this.  See bug 9774.
                ////rtn = _testType(expectedType,strValTypeNS::CHAR_VALUE);
            }
            break;
        case CIMTYPE_BOOLEAN:
            {
                rtn = _testType(expectedType,strValTypeNS::BOOLEAN_VALUE);
            }
            break;
        case CIMTYPE_STRING:
            {
                rtn = _testType(expectedType,strValTypeNS::STRING_VALUE);
            }
            break;
        // The remaining types do not matter at this point.
        case CIMTYPE_DATETIME:
            break;
        case CIMTYPE_REFERENCE:
            break;
        case CIMTYPE_OBJECT:
        case CIMTYPE_INSTANCE:
            break;
        default:
            PEGASUS_ASSERT(false);    // default
    }
    return rtn;
}
CIMValue * valueFactory::createValue(CIMType type, int arrayDimension,
      Boolean isNULL,
      int parseType,
      const String *repp)
{
  const String &rep = *repp;
  CIMDateTime dt;

  if (!isNULL)
  {
     // Compare the parseType enum against the type.  They must match.
     // This originated from bug 3574 which discovered that the parser
     // cannot really separate boolean and strings effectively since
     // the leading quote was removed from parsed strings.
     // Error generates error log (i.e. exception)
     if (!compareTypeToParseType(type, parseType))
     {
        String message;
        cimmofMessages::arglist arglist;
        String localMsg(" Does not match parsed type ");
        arglist.append(cimTypeToString(type));

        localMsg.append(strValTypeEnumToString(
           (strValTypeNS::strValTypeEnum)parseType));
        localMsg.append(". ");
        arglist.append(localMsg);
        cimmofMessages::getMessage(
            message, cimmofMessages::INVALID_LITERAL_VALUE, arglist);

        cimmofParser::Instance()->log_parse_error(
           cimmof_text, message.getCString());
     }
  }
  /****************************************************************
  Debug display for valueFactory.
  cout << " valueFactory::createValue " << cimTypeToString(type)
       << ". " << (isNULL? "NULL" : "nonNULL")
       << ". arrayDimension= " << arrayDimension;
  if (!isNULL)
  {
      cout << ". value " << rep << endl;
  }
  cout << endl;
  *****************************************************************/

  CIMValue * rtn;

  // if arrayDimension == -1 this is not an array type
  if (arrayDimension == CIMMOF_EMPTY_ARRAY)
  {
    if (isNULL)
    {
       rtn = new CIMValue(type, false);
    }

    else
    {
        switch(type)
        {
            case CIMTYPE_UINT8:
                rtn = new CIMValue((Uint8)  stringToUint(rep, type));
                break;
            case CIMTYPE_SINT8:
                rtn = new CIMValue((Sint8)  stringToSint(rep, type));
                break;
            case CIMTYPE_UINT16:return
                rtn = new CIMValue((Uint16) stringToUint(rep, type));
                break;
            case CIMTYPE_SINT16:
                rtn = new CIMValue((Sint16) stringToSint(rep, type));
                break;
            case CIMTYPE_UINT32:
                rtn = new CIMValue((Uint32) stringToUint(rep, type));
                break;
            case CIMTYPE_SINT32:
                rtn = new CIMValue((Sint32) stringToSint(rep, type));
                break;
            case CIMTYPE_UINT64:
                rtn = new CIMValue((Uint64) stringToUint(rep, type));
                break;
            case CIMTYPE_SINT64:
                rtn = new CIMValue((Sint64) stringToSint(rep, type));
                break;
            case CIMTYPE_REAL32:
                rtn = new CIMValue((Real32) stringToReal(rep, type));
                break;
            case CIMTYPE_REAL64:
                rtn = new CIMValue((Real64) stringToReal(rep, type));
                break;
            case CIMTYPE_CHAR16:
                rtn = new CIMValue((Char16) rep[0]);
                break;
            case CIMTYPE_BOOLEAN:
                rtn = new CIMValue((Boolean) _stringToBoolean(rep));
                break;
            case CIMTYPE_STRING:
                rtn = new CIMValue(rep);
                break;
            case CIMTYPE_DATETIME:
                rtn = new CIMValue(CIMDateTime(rep));
                break;
            case CIMTYPE_REFERENCE:
                rtn = new CIMValue(CIMObjectPath(rep));
                break;
            //  PEP 194:  CIMTYPE_OBJECT is not a DMTF CIM datatype,
            // just an OpenPegasus internal representation of an embedded
            // object, so it won't be  found here.
            case CIMTYPE_OBJECT:
            case CIMTYPE_INSTANCE:
                rtn = new CIMValue((Uint32) 0);    // return empty CIMValue
                break;
            default:
                rtn = new CIMValue((Uint32) 0);    // default
        }
    }
  }

  // else array type
  else
  {
      // If empty string set CIMValue type but Null attribute.
      if (isNULL)
      {
          rtn = new CIMValue(type, true, (Uint32)arrayDimension);
      }
      else
      {
        rtn = _buildArrayValue(type, rep);
      }
  }
  return rtn;
}
