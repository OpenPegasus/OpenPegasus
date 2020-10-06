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
//%////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StrLit.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMClassRep.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/ArrayInter.h>
#include <cctype>
#include <cstdio>

#include "JSONWriter.h"

PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN


// From XMLGenerator
// Note: we cannot use StrLit here since it has a constructor (forbids
// structure initialization).

struct SpecialChar
{
    const char* str;
    Uint32 size;
};

// Defines encodings of special characters. Just use a 7-bit ASCII character
// as an index into this array to retrieve its string encoding and encoding
// length in bytes.
/*From RFC 4627:
    All Unicode characters may be placed within the
    quotation marks except for the characters that must be escaped:
    quotation mark, reverse solidus, and the control characters (U+0000
    through U+001F).
    [...]
    string = quotation-mark *char quotation-mark

     char = unescaped /
            escape (
                %x22 /          ; "    quotation mark  U+0022
                %x5C /          ; \    reverse solidus U+005C
                %x2F /          ; /    solidus         U+002F
                %x62 /          ; b    backspace       U+0008
                %x66 /          ; f    form feed       U+000C
                %x6E /          ; n    line feed       U+000A
                %x72 /          ; r    carriage return U+000D
                %x74 /          ; t    tab             U+0009
                %x75 4HEXDIG )  ; uXXXX                U+XXXX

     escape = %x5C              ; \

     quotation-mark = %x22      ; "

     unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
 */
static const SpecialChar _specialChars[] =
{
    {STRLIT_ARGS("\\u0000")},
    {STRLIT_ARGS("\\u0001")},
    {STRLIT_ARGS("\\u0002")},
    {STRLIT_ARGS("\\u0003")},
    {STRLIT_ARGS("\\u0004")},
    {STRLIT_ARGS("\\u0005")},
    {STRLIT_ARGS("\\u0006")},
    {STRLIT_ARGS("\\u0007")},
    {STRLIT_ARGS("\\u0008")},
    {STRLIT_ARGS("\\u0009")},
    {STRLIT_ARGS("\\u000A")},
    {STRLIT_ARGS("\\u000B")},
    {STRLIT_ARGS("\\u000C")},
    {STRLIT_ARGS("\\u000D")},
    {STRLIT_ARGS("\\u000E")},
    {STRLIT_ARGS("\\u000F")},
    {STRLIT_ARGS("\\u0010")},
    {STRLIT_ARGS("\\u0011")},
    {STRLIT_ARGS("\\u0012")},
    {STRLIT_ARGS("\\u0013")},
    {STRLIT_ARGS("\\u0014")},
    {STRLIT_ARGS("\\u0015")},
    {STRLIT_ARGS("\\u0016")},
    {STRLIT_ARGS("\\u0017")},
    {STRLIT_ARGS("\\u0018")},
    {STRLIT_ARGS("\\u0019")},
    {STRLIT_ARGS("\\u001A")},
    {STRLIT_ARGS("\\u001B")},
    {STRLIT_ARGS("\\u001C")},
    {STRLIT_ARGS("\\u001D")},
    {STRLIT_ARGS("\\u001E")},
    {STRLIT_ARGS("\\u001F")},
    {STRLIT_ARGS(" ")},
    {STRLIT_ARGS("!")},
    {STRLIT_ARGS("\\u0022")}, // "
    {STRLIT_ARGS("#")},
    {STRLIT_ARGS("$")},
    {STRLIT_ARGS("%")},
    {STRLIT_ARGS("&")},
    {STRLIT_ARGS("'")},
    {STRLIT_ARGS("(")},
    {STRLIT_ARGS(")")},
    {STRLIT_ARGS("*")},
    {STRLIT_ARGS("+")},
    {STRLIT_ARGS(",")},
    {STRLIT_ARGS("-")},
    {STRLIT_ARGS(".")},
    {STRLIT_ARGS("\\/")}, // slash
    {STRLIT_ARGS("0")},
    {STRLIT_ARGS("1")},
    {STRLIT_ARGS("2")},
    {STRLIT_ARGS("3")},
    {STRLIT_ARGS("4")},
    {STRLIT_ARGS("5")},
    {STRLIT_ARGS("6")},
    {STRLIT_ARGS("7")},
    {STRLIT_ARGS("8")},
    {STRLIT_ARGS("9")},
    {STRLIT_ARGS(":")},
    {STRLIT_ARGS(";")},
    {STRLIT_ARGS("<")},
    {STRLIT_ARGS("=")},
    {STRLIT_ARGS(">")},
    {STRLIT_ARGS("?")},
    {STRLIT_ARGS("@")},
    {STRLIT_ARGS("A")},
    {STRLIT_ARGS("B")},
    {STRLIT_ARGS("C")},
    {STRLIT_ARGS("D")},
    {STRLIT_ARGS("E")},
    {STRLIT_ARGS("F")},
    {STRLIT_ARGS("G")},
    {STRLIT_ARGS("H")},
    {STRLIT_ARGS("I")},
    {STRLIT_ARGS("J")},
    {STRLIT_ARGS("K")},
    {STRLIT_ARGS("L")},
    {STRLIT_ARGS("M")},
    {STRLIT_ARGS("N")},
    {STRLIT_ARGS("O")},
    {STRLIT_ARGS("P")},
    {STRLIT_ARGS("Q")},
    {STRLIT_ARGS("R")},
    {STRLIT_ARGS("S")},
    {STRLIT_ARGS("T")},
    {STRLIT_ARGS("U")},
    {STRLIT_ARGS("V")},
    {STRLIT_ARGS("W")},
    {STRLIT_ARGS("X")},
    {STRLIT_ARGS("Y")},
    {STRLIT_ARGS("Z")},
    {STRLIT_ARGS("[")},
    {STRLIT_ARGS("\\\\")},
    {STRLIT_ARGS("]")},
    {STRLIT_ARGS("^")},
    {STRLIT_ARGS("_")},
    {STRLIT_ARGS("`")},
    {STRLIT_ARGS("a")},
    {STRLIT_ARGS("b")},
    {STRLIT_ARGS("c")},
    {STRLIT_ARGS("d")},
    {STRLIT_ARGS("e")},
    {STRLIT_ARGS("f")},
    {STRLIT_ARGS("g")},
    {STRLIT_ARGS("h")},
    {STRLIT_ARGS("i")},
    {STRLIT_ARGS("j")},
    {STRLIT_ARGS("k")},
    {STRLIT_ARGS("l")},
    {STRLIT_ARGS("m")},
    {STRLIT_ARGS("n")},
    {STRLIT_ARGS("o")},
    {STRLIT_ARGS("p")},
    {STRLIT_ARGS("q")},
    {STRLIT_ARGS("r")},
    {STRLIT_ARGS("s")},
    {STRLIT_ARGS("t")},
    {STRLIT_ARGS("u")},
    {STRLIT_ARGS("v")},
    {STRLIT_ARGS("w")},
    {STRLIT_ARGS("x")},
    {STRLIT_ARGS("y")},
    {STRLIT_ARGS("z")},
    {STRLIT_ARGS("{")},
    {STRLIT_ARGS("|")},
    {STRLIT_ARGS("}")},
    {STRLIT_ARGS("~")},
    {STRLIT_ARGS("\\u007F")},
};

// From XMLGenerator
static const int _isSpecialChar7[] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
};


// From XMLWriter
static StrLit _JsonWriterTypeStrings[] =
{
    STRLIT("boolean"),   STRLIT("uint8"),
    STRLIT("sint8"),     STRLIT("uint16"),
    STRLIT("sint16"),    STRLIT("uint32"),
    STRLIT("sint32"),    STRLIT("uint64"),
    STRLIT("sint64"),    STRLIT("real32"),
    STRLIT("real64"),    STRLIT("char16"),
    STRLIT("string"),    STRLIT("datetime"),
    STRLIT("reference"), STRLIT("object"),
    STRLIT("instance")
};


JSONWriter::JSONWriter(Buffer& buf) :
    _buffer(buf), _propCount(0), _numObjectsEnumerated(0)
{
}

JSONWriter::~JSONWriter()
{
    _deletePropertyNames();
}

void JSONWriter::append(CIMInvokeMethodResponseMessage* methodResult,
                        CIMRepository* repository,
                        RsURI& requestUri)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
            "JSONWriter::append(CIMInvokeMethodResponseMessage)");

    /*
    {
        "kind": " methodresponse",
        "self": "/cimrs/root%2Fcimv2/Square/1/GetArea",
        "method": "GetArea",
        "returnvalue": 100,
        "parameters": {}
    }

    */
    Array<CIMParamValue>& outParms = methodResult->outParameters;

    _buffer.append('{');
    _append(String("kind"));
    _buffer.append(':');
    _append(String("methodresponse"));
    _buffer.append(',');

    _append(String("self"));
    _buffer.append(':');
    _buffer.append('"');
    _buffer.append(requestUri.getString().getCString(),
                    requestUri.getString().size());
    _buffer.append('"');
    _buffer.append(',');

    _append(String("method"));
    _buffer.append(':');
    _append(methodResult->methodName.getString());
    _buffer.append(',');

    _append(String("returnvalue"));
    _buffer.append(':');
    _append(methodResult->retValue, repository, requestUri);
    _buffer.append(',');

    _append(String("parameters"));
    _buffer.append(':');
    _buffer.append('{');
    for (Uint32 x = 0; x < outParms.size(); ++x)
    {
        _append(outParms[x].getParameterName());
        _buffer.append(':');
        _append(outParms[x].getValue(), repository, requestUri);

        if( x + 1 < outParms.size())
        {
            _buffer.append(',');
        }
    }
    _buffer.append('}');
    _buffer.append('}');


    PEG_METHOD_EXIT();
}

void JSONWriter::append(CIMReferencesResponseMessage* referencesResult,
                        CIMRepository* repository,
                        RsURI& requestUri)
{

    PEG_METHOD_ENTER(TRC_RSSERVER,
            "JSONWriter::append(CIMAssociatorsResponseMessage*"
            "enumResult, Uint32 firstInstance, Uint32 lastInstance)");

    Uint32 numInstances = referencesResult->
                            getResponseData().getObjects().size();

    if (numInstances == 0)
    {
        return;
    }

    Uint32 firstInstance = 0;
    Uint32 lastInstance = numInstances;

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "JSONWriter::append() _numObjectsEnumerated = %d, "
            "firstInstance = %d, lastInstance = %d, numInstances = %d",
            _numObjectsEnumerated, firstInstance, lastInstance, numInstances));

    if (_numObjectsEnumerated == 0)
    {
        _buffer.append('{'); // start the response
        _buffer.append(STRLIT_ARGS("\"kind\":\"instance\""));

        CIMClass cimClass = repository->getClass(
                requestUri.getNamespaceName(),
                requestUri.getClassName(),
                false /*localOnly*/);
        CIMObjectPath objectPath = requestUri.getReferencePath(cimClass);

        Buffer instanceUri = RsURI::fromObjectPath(objectPath, true);
        _buffer.append(STRLIT_ARGS(",\"self\":\""));
        _buffer.append(instanceUri.getData(), instanceUri.size());
        _buffer.append('"');

        // the class name
        _buffer.append(STRLIT_ARGS(",\"class\":"));
        _append(requestUri.getClassName().getString());

        // the key properties
        _buffer.append(STRLIT_ARGS(",\"properties\":{"));
        _buffer.append('}'); // end properties

        // now provide paths to all methods
        _appendMethods(cimClass, instanceUri, CIMInstance(), true);

        _buffer.append(',');
        // assoc nav name here
        _append(requestUri.getNavString());

        _buffer.append(STRLIT_ARGS(": {\"kind\":\"instancecollection\","
                          "\"instances\":"));

        _buffer.append('[');
    }
    else if (_buffer.size() > 0 &&
             _buffer.get(_buffer.size()-1) == '}' &&
             _buffer.get(_buffer.size()-2) == '}' &&
             _buffer.get(_buffer.size()-3) == ']')
    {
        _buffer.set(_buffer.size()-3, ',');
        _buffer.set(_buffer.size()-2, ' ');
        _buffer.set(_buffer.size()-1, ' ');
    }

    _append(referencesResult->getResponseData().getObjects(),
            repository,
            requestUri);

    if (_buffer.size() > 0 && _buffer.get(_buffer.size()-1) == ',')
    {
        _buffer.remove(_buffer.size()-1);
    }

    _buffer.append(']');

    _buffer.append('}'); // end nav property
    _buffer.append('}'); // end the response
    _numObjectsEnumerated += numInstances;

    PEG_METHOD_EXIT();
}

void JSONWriter::append(CIMAssociatorsResponseMessage* enumResult,
                        CIMRepository* repository,
                        RsURI& requestUri)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
            "JSONWriter::append(CIMAssociatorsResponseMessage*"
            "enumResult, Uint32 firstInstance, Uint32 lastInstance)");

    Uint32 numInstances = enumResult->getResponseData().getObjects().size();

    if (numInstances == 0)
    {
        return;
    }

    Uint32 firstInstance = 0;
    Uint32 lastInstance = numInstances;

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "JSONWriter::append() _numObjectsEnumerated = %d, "
            "firstInstance = %d, lastInstance = %d, numInstances = %d",
            _numObjectsEnumerated, firstInstance, lastInstance, numInstances));

    if (_numObjectsEnumerated == 0)
    {
        _buffer.append('{'); // start the response
        _buffer.append(STRLIT_ARGS("\"kind\":\"instance\""));

        CIMClass cimClass = repository->getClass(
                requestUri.getNamespaceName(),
                requestUri.getClassName(),
                false /*localOnly*/);
        CIMObjectPath objectPath = requestUri.getAssociationPath(cimClass);

        Buffer instanceUri = RsURI::fromObjectPath(objectPath, true);
        _buffer.append(STRLIT_ARGS(",\"self\":\""));
        _buffer.append(instanceUri.getData(), instanceUri.size());
        _buffer.append('"');

        // the class name
        _buffer.append(STRLIT_ARGS(",\"class\":"));
        _append(requestUri.getClassName().getString());

        // the key properties
        _buffer.append(STRLIT_ARGS(",\"properties\":{"));
        _buffer.append('}'); // end properties

        // now provide paths to all methods
        _appendMethods(cimClass, instanceUri, CIMInstance(), true);

        _buffer.append(',');
        // assoc nav name here
        _append(requestUri.getNavString());

        _buffer.append(STRLIT_ARGS(": {\"kind\":\"instancecollection\","
                           "\"instances\":"));
        _buffer.append('[');
    }
    else if (_buffer.size() > 0 &&
             _buffer.get(_buffer.size()-1) == '}' &&
             _buffer.get(_buffer.size()-2) == '}' &&
             _buffer.get(_buffer.size()-3) == ']')
    {
        _buffer.set(_buffer.size()-3, ',');
        _buffer.set(_buffer.size()-2, ' ');
        _buffer.set(_buffer.size()-1, ' ');
    }

    _append(enumResult->getResponseData().getObjects(),
            repository,
            requestUri);

    if (_buffer.size() > 0 && _buffer.get(_buffer.size()-1) == ',')
    {
        _buffer.remove(_buffer.size()-1);
    }

    _buffer.append(']');

    _buffer.append('}'); // end nav property
    _buffer.append('}'); // end the response
    _numObjectsEnumerated += numInstances;

    PEG_METHOD_EXIT();
}

void JSONWriter::append(CIMEnumerateInstancesResponseMessage* enumResult,
    Uint32 firstInstance, Uint32 lastInstance, CIMRepository* repository,
    RsURI& requestUri)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
            "JSONWriter::append(CIMEnumerateInstancesResponseMessage*"
            "enumResult, Uint32 firstInstance, Uint32 lastInstance)");

    Uint32 numInstances =
        enumResult->getResponseData().getInstances().size();

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "JSONWriter::append()"
            "firstInstance = %d, lastInstance = %d, numInstances = %d, "
            "_numObjectsEnumerated = %d",
            firstInstance, lastInstance, numInstances, _numObjectsEnumerated));

    // If instances are available through more than one provider
    // make sure that they are contained in one array
    Uint32 bufferSize = _buffer.size();
    if (_numObjectsEnumerated == 0 &&
        // bufferSize > 2 && _buffer.get(bufferSize-2) == '[' &&
        _buffer.get(bufferSize-2) == ']' &&
        _buffer.get(bufferSize-1) == '}')
    {
        _buffer.remove(_buffer.size()-1);
        if (bufferSize > 2 && _buffer.get(bufferSize-3) == '[')
        {
            _buffer.remove(_buffer.size()-1);
        }
    }
    else if (_numObjectsEnumerated == 0)
    {
        /// this is the start of the array
        _buffer.append(STRLIT_ARGS("{\"kind\":\"instancecollection\","
                                  "\"self\":\""));
        _buffer.append(requestUri.getString().getCString(),
                                  requestUri.getString().size());
        _buffer.append(STRLIT_ARGS("\",\"class\":"));
        _append(requestUri.getClassName().getString());
        _buffer.append(STRLIT_ARGS(",\"instances\":["));
    }

    if (firstInstance == PEG_NOT_FOUND || lastInstance == PEG_NOT_FOUND)
    {
        firstInstance = 0;
        lastInstance = numInstances;
    }
    else
    {
        // Called for ranged requests

        // _numObjectsEnumerated tracks the number
        // of instances that went by.
        // Example: Assume a result set of 1000 instances,
        // and a request for instances 299-501 of that set.
        // The append method is called 10 times (numInstances = 100)
        // with firstInstance = 299 and lastInstance = 501.
        //
        // The if statements below normalize firstInstance and
        // lastInstance to fit the current call.
        if (_numObjectsEnumerated + numInstances <= firstInstance)
        {
            // firstInstance is larger than the sum of
            // objects enumerated to this point and the number
            // of objects in this iteration.
            // For the above example of firstInstance == 299 and
            // lastInstance == 501 this is the case for the
            // first two iterations (_numObjectsEnumerated == 0,
            // _numObjectsEnumerated == 100) --> nothing to append
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "JSONWriter::append() nothing to do - "
                    "firstInstance = %d, lastInstance = %d, numInstances = %d",
                    firstInstance, lastInstance, numInstances));

            _numObjectsEnumerated += numInstances;
            PEG_METHOD_EXIT();
            return;
        }
        else if (_numObjectsEnumerated < firstInstance)
        {
            // firstInstance is within the range of the current
            // iteration, e.g. firstInstance == 299,
            // normalized to 299 - 200 = 99 (third iteration)
            firstInstance -= _numObjectsEnumerated;
        }
        else
        {
            // _numObjectsEnumerated is larger than firstInstance.
            firstInstance = 0;
        }


        if (_numObjectsEnumerated < lastInstance)
        {
            // _numObjectsEnumerated is within current
            // iteration, e.g. lastInstance == 501,
            // normalized to 501 - 500 = 1 (sixth iteration)
            lastInstance -= _numObjectsEnumerated;
        }
        else
        {
            // _numObjectsEnumerated is larger than current iteration
            // --> nothing to append
            _numObjectsEnumerated += numInstances;
            PEG_METHOD_EXIT();
            return;
        }
    }

    if (_numObjectsEnumerated > 0 && _buffer.get(_buffer.size()-1) == ']')
    {
        _buffer.set(_buffer.size()-1, ',');
    }

    for (Uint32 i = firstInstance; i <= lastInstance && i < numInstances; ++i)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "JSONWriter::append() firstInstance = %d,"
                "EnumerateInstance i = %d, _numObjectsEnumerated = %d, "
                "Buffer size: %d",
                firstInstance, i, _numObjectsEnumerated, _buffer.size()));

        _append(enumResult->getResponseData().getInstances()[i],
                true, true, repository, requestUri);

        if (i < lastInstance)
            _buffer.append(',');

        if (i == firstInstance && _numObjectsEnumerated == 0)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "JSONWriter::append() Adjusting Buffer by %d * %d",
                lastInstance, _buffer.size()));

            _buffer.reserveCapacity(_buffer.size() + ((lastInstance *
                _buffer.size()) + 2));
        }
    }

    if (_buffer.get(_buffer.size()-1) == ',')
    {
        _buffer.remove(_buffer.size()-1);
    }

    _buffer.append(']'); // end the array of instance
    _buffer.append('}'); // end the instancecollection

    _numObjectsEnumerated += numInstances;

    PEG_METHOD_EXIT();
}


void JSONWriter::append(CIMGetClassResponseMessage* result)
{
    _append(result->cimClass);
}

void JSONWriter::append(CIMGetInstanceResponseMessage* result,
                        CIMRepository* repository,
                        RsURI& requestUri)
{
    _append(result->getResponseData().getInstance(),
            true, true, repository, requestUri);
}

void JSONWriter::append(CIMException& e, String& httpMethod, RsURI& reqURI)
{

//    {
//        "kind": "errorresponse",
//        "self": "/cimrs/root%2Fcimv2/Square/1",
//        "httpmethod": "GET",
//        "statuscode": 12,
//        "statusdescription": "Program exited abnormally.",
//        "errors": [
//            {
//                "kind": "instance",
//                "class": "CIM_Error",
//                "properties": {
//                    "ErrorType": 4,
//                    "PerceivedSeverity": 5,
//                    "ProbableCause": 48,
//                    "Message": "Program exited abnormally.",
//                    "MessageArguments": [ "42" ],
//                    "MessageID": "1234",
//                    "OwningEntity": "ACME"
//                }
//            }
//        ]
//    }

    const char* message = cimStatusCodeToString( e.getCode());

    _buffer.append(STRLIT_ARGS("{"));

    _buffer.append(STRLIT_ARGS("\"kind\": \"errorresponse\",\"self\":\""));
    _buffer.append(reqURI.getString().getCString(), reqURI.getString().size());
    _buffer.append(STRLIT_ARGS("\","));
    _buffer.append(STRLIT_ARGS("\"httpmethod\":"));
    _append(httpMethod);
    _buffer.append(',');
    _buffer.append(STRLIT_ARGS("\"statuscode\":"));
    _append((Uint32)e.getCode());
    _buffer.append(',');
    _buffer.append(STRLIT_ARGS("\"statusdescription\":"));
    _append(String(message));
    _buffer.append(',');

    _buffer.append(STRLIT_ARGS("\"message\":\""));
    _buffer.append(message, strlen(message));
    _buffer.append(STRLIT_ARGS("\""));
    _buffer.append(',');


    _buffer.append(STRLIT_ARGS("\"errors\":["));
    for(Uint32 i = 0; i < e.getErrorCount(); i++)
    {
        CIMConstInstance cimError = e.getError(i);
        _append(cimError, false, true, NULL, reqURI, false);
        if(i < e.getErrorCount()-1)
            _buffer.append(',');
    }
    _buffer.append(']');

    _buffer.append(STRLIT_ARGS("}"));
}

void JSONWriter::append(Exception& e, String& httpMethod, RsURI& reqURI)
{
    CIMException cimException(CIM_ERR_FAILED,
                              cimStatusCodeToString(CIM_ERR_FAILED));
    append(cimException, httpMethod, reqURI);
}



void JSONWriter::_append(Array<CIMObject>& objArray,
                         CIMRepository* repository,
                         RsURI& requestUri)
{
    for (Uint32 i = 0, n = objArray.size(); i < n; ++i)
    {
        const CIMObject& cimObj = objArray[i];

        if (cimObj.isInstance())
        {
            CIMConstInstance inst(cimObj);
            _append(inst, true, true, repository, requestUri);
            if(i < n - 1)
                _buffer.append(',');
        }
    }
}

void JSONWriter::_append(const CIMConstClass& cimClass)
{
    _buffer.append(STRLIT_ARGS("{\"name\":"));
    _append(cimClass.getClassName().getString());
    _buffer.append(STRLIT_ARGS(",\"superclass\":"));

    if (cimClass.getSuperClassName().isNull())
    {
        _buffer.append(STRLIT_ARGS("null"));
    }
    else
    {
        _append(cimClass.getSuperClassName().getString());
    }

    _buffer.append(STRLIT_ARGS(",\"properties\":{"));
    Uint32 propertyCount = cimClass.getPropertyCount();
    for (Uint32 i = 0; i < propertyCount; i++)
    {
        _append(cimClass.getProperty(i));

        if (i < propertyCount - 1)
            _buffer.append(',');
    }

    _buffer.append(STRLIT_ARGS("},\"qualifiers\":{"));
    Uint32 qualifierCount = cimClass.getQualifierCount();
    for (Uint32 i = 0; i < qualifierCount; i++)
    {
        _append(cimClass.getQualifier(i));

        if (i < qualifierCount - 1)
            _buffer.append(',');
    }
    _buffer.append(STRLIT_ARGS("}"));

    _buffer.append('}');
}

void JSONWriter::_appendMethods(const CIMClass &cimClass, Buffer instanceUri,
                  const CIMConstInstance& cimInstance, Boolean useAbsoluteUri)
{
    // now provide paths to all methods
    _buffer.append(STRLIT_ARGS(",\"methods\":{"));
    if (!cimClass.isUninitialized())
    {
        // for each method produce a method URI
        if (instanceUri.size() == 0)
        {
            CIMObjectPath objPath;
            // for embedded instances we might not get a good object path
            if (cimInstance.getPath().getKeyBindings().size() == 0) 
                  objPath = cimInstance.buildPath(
                    cimInstance.getClassName());
            else objPath = cimInstance.getPath();
            instanceUri = RsURI::fromObjectPath(objPath, useAbsoluteUri);
        }

        for (Uint32 i = 0; i < cimClass.getMethodCount(); i++)
        {
            _append(cimClass.getMethod(i).getName().getString());
            _buffer.append(':');

            _buffer.append('"');
            _buffer.append(instanceUri.getData(), instanceUri.size());
            _buffer.append('/');
            _buffer.append(cimClass.getMethod(i).getName().
                                    getString().getCString(),
                           cimClass.getMethod(i).getName().getString().size());
            _buffer.append('"');
            // append a , if this is not the last method
            if (i != cimClass.getMethodCount() - 1) _buffer.append(',');
        }
    }
    else
    {
        PEG_TRACE(
                (TRC_RSSERVER, Tracer::LEVEL4,
                 "JSONWriter::append instance could not get to class def."
                 " Leaving method list empty."));
    }
    _buffer.append(STRLIT_ARGS("}"));
}

void JSONWriter::_append(const CIMConstInstance& cimInstance,
    Boolean includeUri, Boolean useAbsoluteUri, CIMRepository* repository,
    RsURI& requestUri, Boolean includeMethods)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
            "JSONWriter::append(const CIMInstance& cimInstance, "
            "Boolean includeUri)");

    Uint32 propertyCount = cimInstance.getPropertyCount();

    // get the class def
    CIMClass cimClass;
    if(repository != NULL)
    {
        try{
            cimClass = repository->getClass(
                    requestUri.getNamespaceName(),
                    cimInstance.getClassName(),
                    false /*localOnly*/);
        }
        catch (CIMException& e)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                    "JSONWriter::_append() CIMException thrown: %s / %s",
                    (const char*)e.getMessage().getCString(),
                    cimStatusCodeToString( e.getCode() )));
            // No repository so log a message and leave the methods empty.
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "JSONWriter::append instance"
                    " could not find class definition."));
            throw;
        }
    }
    else
    {
        // No repository so log a message and leave the methods empty.
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "JSONWriter::append instance did not receive a repository."));
    }

    _buffer.append('{');
    _buffer.append(STRLIT_ARGS("\"kind\":\"instance\""));

    Buffer instanceUri;
    if (includeUri)
    {
        _buffer.append(STRLIT_ARGS(",\"self\":\""));

        CIMObjectPath objPath;
        // for embedded instances we might not get a good object path
        if(cimInstance.getPath().getKeyBindings().size() == 0)
            objPath = cimInstance.buildPath(cimClass);
        else
            objPath = cimInstance.getPath();

        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "JSONWriter::append object path:%s",
                 (const char*)(objPath.toString().getCString())));

        instanceUri = RsURI::fromObjectPath(objPath, useAbsoluteUri);
        _buffer.append(instanceUri.getData(), instanceUri.size());
        _buffer.append('"');
    }

    // the class name
    _buffer.append(STRLIT_ARGS(",\"class\":"));
    _append(cimInstance.getClassName().getString());


    _buffer.append(STRLIT_ARGS(",\"properties\":{"));

    // Invalid optimization. Code may be removed
    /*if (_propClass != cimInstance.getClassName())
    {
        _deletePropertyNames();
        _loadPropertyNames(cimInstance);
    }*/

    for (Uint32 i = 0; i < propertyCount; ++i)
    {
        _append(cimInstance.getProperty(i).getName().getString());
        _buffer.append(':');
        _append(cimInstance.getProperty(i).getValue(), repository, requestUri);

        if (i < propertyCount - 1)
            _buffer.append(',');
    }

    _buffer.append('}');  // end of properties

    if(includeMethods)
    {
        // now provide paths to all methods
        _appendMethods(cimClass, instanceUri, cimInstance, useAbsoluteUri);
    }
    _buffer.append(STRLIT_ARGS("}")); // end of instance
    PEG_METHOD_EXIT();
}

void JSONWriter::_append(const CIMConstProperty& property)
{
    _append(property.getName().getString());

    _buffer.append(STRLIT_ARGS(":{\"type\":\""));
    _buffer << _JsonWriterTypeStrings[property.getValue().getType()];

    Uint32 qualifierCount = property.getQualifierCount();
    _buffer.append(STRLIT_ARGS("\",\"qualifiers\":{"));
    for (Uint32 i = 0; i < qualifierCount; i++)
    {
        _append(property.getQualifier(i));

        if (i < qualifierCount - 1)
            _buffer.append(',');
    }
    _buffer.append(STRLIT_ARGS("}}"));
}


void JSONWriter::_append(const CIMConstQualifier& qualifier)
{
    //CheckRep(qualifier._rep);
    //const CIMQualifierRep* rep = qualifier._rep;

    _append(qualifier.getName().getString());
    _buffer.append(STRLIT_ARGS(":"));
    RsURI emptyURI("");
    _append(qualifier.getValue(), NULL, emptyURI);
}


void JSONWriter::_append(const CIMValue& value, CIMRepository* repository,
                         RsURI& requestUri)
{
    if (value.isNull())
    {
        _buffer.append(STRLIT_ARGS("null"));
        return;
    }

    if (value.isArray())
    {
        _buffer.append('[');
        switch (value.getType())
        {
        case CIMTYPE_BOOLEAN:
        {
            Array<Boolean> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_UINT8:
        {
            Array<Uint8> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_SINT8:
        {
            Array<Sint8> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_UINT16:
        {
            Array<Uint16> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_SINT16:
        {
            Array<Sint16> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_UINT32:
        {
            Array<Uint32> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_SINT32:
        {
            Array<Sint32> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_UINT64:
        {
            Array<Uint64> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_SINT64:
        {
            Array<Sint64> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_REAL32:
        {
            Array<Real32> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_REAL64:
        {
            Array<Real64> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }

        case CIMTYPE_CHAR16:
        {
            Array<Char16> a;
            value.get(a);
            String s;

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                s = String(&a[i]);
                _append(s);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }

            break;
        }

        case CIMTYPE_STRING:
        {
            Array<String> a;
            value.get(a);

            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }

            break;
        }
        case CIMTYPE_REFERENCE:
        {
            Array<CIMObjectPath> a;
            value.get(a);
            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }
        case CIMTYPE_DATETIME:
        {
            Array<CIMDateTime> a;
            value.get(a);
            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i]);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }
        case CIMTYPE_OBJECT:
        {
            Array<CIMObject> a;
            value.get(a);
            _append(a, repository, requestUri);
            break;
        }
        case CIMTYPE_INSTANCE:
        {
            Array<CIMInstance> a;
            value.get(a);
            for (Uint32 i = 0; i < a.size(); ++i)
            {
                _append(a[i], true, true, repository, requestUri);
                if (i < a.size() - 1)
                    _buffer.append(',');
            }
            break;
        }
        default:
            PEGASUS_ASSERT(false);
        }
        _buffer.append(']');
        return;
    }
    else
    {

        // from XmlWriter::appendValueElement
        switch (value.getType())
        {
            case CIMTYPE_BOOLEAN:
            {
                Boolean v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_UINT8:
            {
                Uint8 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_SINT8:
            {
                Sint8 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_UINT16:
            {
                Uint16 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_SINT16:
            {
                Sint16 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_UINT32:
            {
                Uint32 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_SINT32:
            {
                Sint32 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_UINT64:
            {
                Uint64 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_SINT64:
            {
                Sint64 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_REAL32:
            {
                Real32 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_REAL64:
            {
                Real64 v;
                value.get(v);
                _append(v);
                break;
            }
            case CIMTYPE_CHAR16:
            {
                Char16 v;
                value.get(v);
                String s = String(&v);

                _append(s);
                break;
            }
            case CIMTYPE_STRING:
            {
                String v;
                value.get(v);
                _append(v, true);
                break;
            }
            case CIMTYPE_DATETIME:
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "JSONWriter::appendValue()- DateTime"));
                CIMDateTime v;
                value.get(v);
                _append(v);

                break;
            }
            case CIMTYPE_REFERENCE:
            {
                CIMObjectPath v;
                value.get(v);
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "JSONWriter::appendValue()- Reference"));
                _append(v);
                break;
            }
            case CIMTYPE_OBJECT:
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "JSONWriter::appendValue()- Object"));
                CIMObject v;
                value.get(v);
                _append("CIMTYPE_OBJECT is TODO!");
                break;
            }
            case CIMTYPE_INSTANCE:
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "JSONWriter::appendValue()- Instance"));
                CIMInstance v;
                value.get(v);
                _append(v, true, true, repository, requestUri);
                break;
            }
            default:
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                    "JSONWriter::appendValue()- Unknown type"));
                PEGASUS_ASSERT(false);
            }
        }
    }

    return;
}


void JSONWriter::_append(Boolean v)
{
    if (v)
    {
        _buffer.append(STRLIT_ARGS("true"));
    }
    else
    {
        _buffer.append(STRLIT_ARGS("false"));
    }
}

void JSONWriter::_append(Uint8 v)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint8ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Sint8 v)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint8ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Sint16 v)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint16ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Sint32 v)
{

    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint32ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Sint64 v)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Sint64ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Uint16 v)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint32ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Uint32 v)
{

    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint32ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Uint64 v)
{
    Uint32 outputLength=0;
    char buffer[22];
    const char * output = Uint64ToString(buffer, v, outputLength);

    _buffer.append(output, outputLength);
}

void JSONWriter::_append(Real32 v)
{
    char buffer[128];
    // %.7e gives '[-]m.ddddddde+/-xx', which seems compatible with the format
    // given in the CIM/XML spec, and the precision required by the CIM 2.2 spec
    // (4 byte IEEE floating point)
    sprintf(buffer, "%.7e", v);

    _buffer.append(buffer, sizeof(buffer));
}

void JSONWriter::_append(Real64 v)
{
    char buffer[128];
    // %.16e gives '[-]m.dddddddddddddddde+/-xx', which seems compatible
    // with the format given in the CIM/XML spec, and the precision required
    // by the CIM 2.2 spec (8 byte IEEE floating point)
    sprintf(buffer, "%.16e", v);

    _buffer.append(buffer, sizeof(buffer));
}


void JSONWriter::_append(const CIMDateTime& v)
{
    // see w3.org/TR/NOTE-datetime
    // and ECMAScript 3.1 spec draft
    String s = v.toString();
    _append(s);
}

void JSONWriter::_append(const CIMObjectPath& v)
{
    Buffer uri = RsURI::fromObjectPath(v, true);
    _append(String(uri.getData()));
}

void JSONWriter::_append(const String& str, Boolean uriEncoded)
{
    if(uriEncoded)
    {
        // we need to print an un-encoded version of the string
        _buffer.append('"');
        String uriEncodedStr = XmlGenerator::encodeURICharacters(str);
        _buffer.append((const char *)uriEncodedStr.getCString(),
                        uriEncodedStr.size());
        _buffer.append('"');
    }
    else
        _append(_buffer, str);
}


void JSONWriter::_append(Buffer& out, const String& str)
{

    out.append('"');
    // COPY FROM XmlGenerator::appendSpecial(Buffer& out, const String& str)
    const Uint16* p = (const Uint16*)str.getChar16Data();

    Uint16 c;
    while ((c = *p++) != 0)
    {
        if (c < 128)
        {
            if (_isSpecialChar7[c])
            {
                // Write the character reference for the special character
                out.append(
                    _specialChars[int(c)].str, _specialChars[int(c)].size);
            }
            else
            {
                out.append(c);
            }
        }
        /*
        else
        {

            if ((((c >= FIRST_HIGH_SURROGATE) && (c <= LAST_HIGH_SURROGATE)) ||
                 ((c >= FIRST_LOW_SURROGATE) && (c <= LAST_LOW_SURROGATE))) &&
         *p)
            {
                _appendSurrogatePair(out, c, *p++);
            }
            else
            {
                _appendChar(out, c);
            }

            prevCharIsSpace = false;

        }*/
    }
    out.append('"');
}

Uint32 JSONWriter::getEnumerationCount()
{
    return _numObjectsEnumerated;
}


void JSONWriter::_loadPropertyNames(const CIMConstInstance& prototype)
{
    _propCount = prototype.getPropertyCount();
    _props = new Buffer*[_propCount];
    _propClass = prototype.getClassName();

    for (Uint32 i = 0; i < _propCount; ++i)
    {
        _props[i] = new Buffer(2048);
        _append(*_props[i], prototype.getProperty(i).getName().getString());
    }

}

void JSONWriter::_deletePropertyNames()
{
    for (Uint32 i = 0; i < _propCount; i++)
    {
        delete _props[i];
    }
    if (_propCount > 0)
    {
        delete[] _props;
    }

    _propCount = 0;
}


PEGASUS_NAMESPACE_END
