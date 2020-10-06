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

#include <cctype>
#include <cstdio>
#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/AutoPtr.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/HTTPConnection.h>
#include <Pegasus/Common/OperationContextInternal.h>
#include <Pegasus/Common/XmlReader.h>

#include "RsProcessor.h"
#include "JSONWriter.h"


PEGASUS_USING_STD;

PEGASUS_NAMESPACE_BEGIN

RsProcessor::RsProcessor(
    MessageQueue* cimOperationProcessorQueue,
    CIMRepository* repository)
    : MessageQueue("RsProcessor"),
      _rsRequestDecoder(this),
      _cimOperationProcessorQueue(cimOperationProcessorQueue),
      _repository(repository),
      _serverTerminating(false)
{
}


RsProcessor::~RsProcessor()
{
}

void RsProcessor::enqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
        "RsProcessor::enqueue()");
    handleEnqueue(message);
    PEG_METHOD_EXIT();
}

void RsProcessor::handleEnqueue()
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
        "RsProcessor::handleEnqueue()");
    Message* message = dequeue();
    handleEnqueue(message);
    PEG_METHOD_EXIT();
}

void RsProcessor::handleEnqueue(Message* message)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
        "RsProcessor::handleEnqueue(Message* message)");

    if (!message)
    {
        PEG_METHOD_EXIT();
        return;
    }

    PEGASUS_ASSERT(dynamic_cast<CIMResponseMessage*>(message) != 0);
    handleResponse(dynamic_cast<CIMResponseMessage*>(message));

    PEG_METHOD_EXIT();
}

Uint32 findEndBlock(
    String& contentStr,
    Uint32 startPos,
    Char16 delemStart,
    Char16 delemEnd)
{
    Uint32 retPos = PEG_NOT_FOUND;
    Uint32 indent = 1;

    Uint32 openPos = contentStr.find(startPos, delemStart);
    Uint32 closePos = contentStr.find(startPos, delemEnd);
    while(indent > 0)
    {
        if(closePos == PEG_NOT_FOUND)
            break; // did not find enough machting delem
        if(openPos < closePos)
        {
            indent++;
            startPos = openPos+1;
            openPos = contentStr.find(startPos, delemStart);
        }
        else
        {
            indent--;
            startPos = closePos+1;
            retPos = closePos;
            closePos = contentStr.find(startPos, delemEnd);
        }
    }

    return retPos;
}

String findPropValue(String contentStr, String property)
{
    Uint32 startPos = contentStr.find(property);
    if(startPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "RsProcessor::findPropValue() Failed to find 'property' name"));
        return String();
    }
    // now look for : which marks the start of the class name.
    startPos = contentStr.find(startPos, ':');
    if(startPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "RsProcessor::findPropValue() Failed "
                "to find class name : separator"));
        return String();
    }

    // The string between the next two double quotes is the class name
    startPos = contentStr.find(startPos, '"');
    if(startPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "RsProcessor::findPropValue() Failed to find class name"));
        return String();
    }
    Uint32 endPos = contentStr.find(startPos+1, '"');
    if(endPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "RsProcessor::findPropValue() Failed to find class name"));
        return String();
    }
    return contentStr.subString(startPos+1, endPos-startPos-1);
}

CIMInstance getEmbeddedInstance(String contentStr,
                                CIMRepository* repository,
                                RsURI& reqUri)
{
    // parse instance
    //{
    //    "kind": "instance",
    //    "class": "Square",
    //    "properties": {
    //        "Id": 1,
    //        "Color": "Yellow" }
    //}

    // look for the class name
    String classNameStr = findPropValue(contentStr, "\"class\"");
    if(classNameStr.size() == 0)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                "RsProcessor::getEmbeddedInstance() class name not found"));
        return CIMInstance();
    }
    CIMName className(classNameStr);

    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
            "RsProcessor::getEmbeddedInstance() class name: %s",
             (const char *)(className.getString().getCString())));

    CIMClass embeddedInstClass = repository->getClass(reqUri.getNamespaceName(),
                                                      className, false);

    CIMInstance result(className); // the resulting CIMInstance

    // first look for "properties"
    Uint32 startPos = contentStr.find("\"properties\"");
    // now look for { which marks the start of the properties.
    startPos = contentStr.find(startPos, '{');

    if(startPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getEmbeddedInstance() "
                    "Failed to find properties"));
        return CIMInstance();
    }

    Uint32 currentPos = startPos + 1;
    // find end of this open brace
    Uint32 endPos = findEndBlock(contentStr, currentPos, '{', '}');
    if(endPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getEmbeddedInstance() "
                    "Failed to find end of properties"));
        return CIMInstance();
    }
    Uint32 nextPos = contentStr.find(currentPos, ':');
    while ((nextPos != PEG_NOT_FOUND) && (nextPos < endPos))
    {
        // find the property name between the two double quotes
        Uint32 propStart = contentStr.find(currentPos, '\"');
        if(propStart == PEG_NOT_FOUND)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getEmbeddedInstance() "
                        "Failed to find start of property name"));
            return CIMInstance();
        }
        propStart++;
        Uint32 propEnd = contentStr.find(propStart, '\"');
        if(propEnd == PEG_NOT_FOUND)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getEmbeddedInstance() "
                        "Failed to find end of property name"));
            return CIMInstance();
        }
        CIMName pName(contentStr.subString(propStart, propEnd - propStart));
        cout << "Name = "<<pName.getString()<<endl;

        currentPos = contentStr.find(propEnd+1, ':');
        if(currentPos == PEG_NOT_FOUND)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getEmbeddedInstance() "
                        "Failed to find property value : separator"));
            return CIMInstance();
        }
        Uint32 startBlkPos = contentStr.find(currentPos,'{');
        Uint32 startArrayPos = contentStr.find(currentPos,'[');
        nextPos = contentStr.find(currentPos,',');

        Boolean isArrayValue = false;
        String arrayValue;

        Boolean isEmbeddedValue = false;
        String embeddedValue;

        if(startBlkPos < nextPos)
        {
            if(startArrayPos < startBlkPos)
                isArrayValue = true;
            else
                isEmbeddedValue = true;
        }
        else if(startArrayPos < nextPos)
            isArrayValue = true;

        if(isEmbeddedValue)
        {
            // there is an embedded block of {  } in the value
            // find end of this open brace
            Uint32 endBlkPos = findEndBlock(contentStr,
                                            startBlkPos+1, '{', '}');
            if(endBlkPos == PEG_NOT_FOUND)
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getEmbeddedInstance() "
                        "Failed to read embedded property value."
                        "End of blk not found."));
                return CIMInstance();
            }
            isEmbeddedValue = true;
            embeddedValue = contentStr.subString(startBlkPos,
                                                 endBlkPos-startBlkPos+1);
            currentPos = endBlkPos+1;
            nextPos = contentStr.find(currentPos,',');
        }
        else if(isArrayValue)
        {
            // there is an array block of [ ] in the value
            // find end of this open ]
            Uint32 endArrayPos = findEndBlock(contentStr,
                                              startArrayPos+1, '[', ']');
            if(endArrayPos == PEG_NOT_FOUND)
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getEmbeddedInstance()"
                        " Failed to read array property value."
                        " End of blk not found."));
                return CIMInstance();
            }
            isArrayValue = true;
            // Copy the contents of the array and drop [ ]
            arrayValue = contentStr.subString(startArrayPos + 1,
                                              endArrayPos-startArrayPos - 1);
            currentPos = endArrayPos+1;
            nextPos = contentStr.find(currentPos,',');
        }
        if (nextPos == PEG_NOT_FOUND || nextPos > endPos)
        {
            nextPos = endPos;
        }

        if(isEmbeddedValue)
        {
            CIMInstance embInst = getEmbeddedInstance(embeddedValue,
                                                      repository, reqUri);
            CIMValue pVal(embInst);
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getEmbeddedInstance() Name:"
                    " [%s], Value: [%s]",
                    (const char*)pName.getString().getCString(),
                    (const char*)pVal.toString().getCString()));
            CIMProperty prop(pName, pVal);
            result.addProperty(prop);
        }
        else if(isArrayValue)
        {
            CIMConstProperty p =
                    embeddedInstClass.getProperty(
                                      embeddedInstClass.findProperty(pName));
            CIMType pType = p.getType();

            // there is an array with [] in the value
            // parse the values from the array

            // see if there are embedded instances in the array:
            Boolean isEmbeddedArray = false;
            if(pType == CIMTYPE_STRING &&
               p.findQualifier(CIMName("EmbeddedInstance")) != PEG_NOT_FOUND)
                isEmbeddedArray = true;

            // split all array elements into an Array<const char *>
            Array<const char *> elements;
            Array<String> elementStrings;
            Array<CIMInstance> embInsts;

            Uint32 currPos = 0;
            while(currPos < arrayValue.size())
            {
                Uint32 valEndPos, valStartPos;
                if(isEmbeddedArray)
                {
                    valStartPos = arrayValue.find(currPos, '{');
                    if(valStartPos == PEG_NOT_FOUND)
                        break; // we have reached the end

                    valEndPos = findEndBlock(arrayValue, valStartPos+1,
                                             '{', '}');
                    currPos = valEndPos + 1;
                    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                           "RsProcessor::getEmbeddedInstance() EmbInst: [%s]",
                            (const char*)arrayValue.subString(valStartPos,
                              valEndPos-valStartPos+1).getCString()));
                    // get the embedded instance
                    CIMInstance embInst =
                            getEmbeddedInstance(arrayValue.subString(
                                                    valStartPos,
                                                    valEndPos-valStartPos+1),
                                                repository,
                                                reqUri);
                    embInsts.append(embInst);
                }
                else
                {
                    valStartPos = currPos;
                    valEndPos = arrayValue.find(currPos, ',');
                    if(valEndPos == PEG_NOT_FOUND)
                        valEndPos = arrayValue.size() - 1;
                    else
                        valEndPos--; // exclude the ,

                    // set currPos to after the ,
                    currPos = arrayValue.find(valEndPos, ',');
                    if(currPos != PEG_NOT_FOUND)
                        currPos++;

                    // Process this value further:
                    // 1. Remove leading and trailing whitespaces
                    // 2. If it is a string, remove double quotes

                    while(arrayValue[valStartPos] == ' ' ||
                            arrayValue[valStartPos] == '\t' ||
                            arrayValue[valStartPos] == '\r' ||
                            arrayValue[valStartPos] == '\n' ||
                            arrayValue[valStartPos] == '"')
                        valStartPos++;

                    while(arrayValue[valEndPos] == ' ' ||
                            arrayValue[valEndPos] == '\t' ||
                            arrayValue[valEndPos] == '\r' ||
                            arrayValue[valEndPos] == '\n' ||
                            arrayValue[valEndPos] == '"')
                        valEndPos--;

                    String strVal = arrayValue.subString(valStartPos,
                                                   valEndPos-valStartPos+1);
                    elementStrings.append(strVal);
                    const char* charPtr = strdup(strVal.getCString());
                    elements.append(charPtr);
                    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                            "RsProcessor::getEmbeddedInstance()"
                            " array element [%d]: [%s]",
                            elementStrings.size(),
                            (const char*)strVal.getCString()));
                }
            }
            if(!isEmbeddedArray)
            {
                CIMValue pVal = XmlReader::stringArrayToValue(0,
                                           elements, pType);

                // free elements
                for (Uint32 k=0; k<elements.size(); k++)
                {
                    free((void*)elements[k]);
                }
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getEmbeddedInstance() "
                        "array element: [%s]",
                        (const char*)pVal.toString().getCString()));

                CIMProperty prop(pName, pVal);
                result.addProperty(prop);
            }
            else
            {
                CIMValue pVal(embInsts);
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getEmbeddedInstance() "
                        "array element: [%s]",
                        (const char*)pVal.toString().getCString()));
                CIMProperty prop(pName, pVal);
                result.addProperty(prop);
            }
        }
        else
        {
            currentPos++; // skip over the :

            // Process this value further:
            // 1. Remove leading and trailing whitespaces
            // 2. If it is a string, remove double quotes
            // 3. ??
            const char * content = (const char *)(contentStr.getCString());
            while(content[currentPos] == ' ' ||
                    content[currentPos] == '\t' ||
                    content[currentPos] == '\r' ||
                    content[currentPos] == '\n' ||
                    content[currentPos] == '"')
                currentPos++;

            Uint32 newPos = nextPos - 1;
            while(content[newPos] == ' ' ||
                    content[newPos] == '\t' ||
                    content[newPos] == '\r' || 
                    content[newPos] == '\n' ||
                    content[newPos] == '"')
                newPos--;

            String pValue(contentStr.subString(currentPos,
                                               newPos-currentPos+1));
            cout << "Value = "<<pValue<<endl;

            CIMType pType = embeddedInstClass.getProperty(embeddedInstClass.
                                               findProperty(pName)).getType();
            CIMValue pVal = XmlReader::stringToValue(0,
                                                     pValue.getCString(),
                                                     pType);
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getEmbeddedInstance() "
                    "Name: [%s], Value: [%s]",
                    (const char*)pName.getString().getCString(),
                    (const char*)pVal.toString().getCString()));
            CIMProperty prop(pName, pVal);
            result.addProperty(prop);
        }

        currentPos = nextPos+1;
        if (currentPos < endPos)
        {
            nextPos = contentStr.find(currentPos,':');
        }
    }

    return result;
}

String RsProcessor::getParamValues(CIMConstMethod& method,
                                   Array<CIMParamValue>& inParms,
                                   const char *content,
                                   Uint32 contentSize,
                                   CIMRepository* repository,
                                   RsURI& reqUri)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,"getParamValues");

    if (contentSize == 0)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,"Content string empty"));

        PEG_METHOD_EXIT();
        return String();
    }
    String contentStr(content, contentSize); 

    // Sample content:
    // {
    //     "kind": " methodrequest",
    //     "self": "/cimrs/root%2Fcimv2/Square/1/GetArea",
    //     "method": "GetArea",
    //     "parameters": {
    //         "Side": 10
    //     }
    // }

    Array<CIMName> pNames;
    Array<String> pValues;
    Array<Boolean> pIsArray;

    // first look for "parameters"
    Uint32 startPos = contentStr.find("\"parameters\"");
    // now look for { which marks the start of the parameters.
    startPos = contentStr.find(startPos, '{');

    if(startPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getParamValues() Failed to find properties"));
        return String();
    }

    Uint32 currentPos = startPos + 1;
    // find end of this open brace
    Uint32 endPos = findEndBlock(contentStr, currentPos, '{', '}');
    //Uint32 endPos = contentStr.find(currentPos, '}');
    if(endPos == PEG_NOT_FOUND)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getParamValues() "
                    "Failed to find end of properties"));
        return String();
    }
    Uint32 nextPos = contentStr.find(currentPos, ':');
    while ((nextPos != PEG_NOT_FOUND) && (nextPos < endPos))
    {
        // find the property name between the two double quotes
        Uint32 propStart = contentStr.find(currentPos, '\"');
        if(propStart == PEG_NOT_FOUND)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getParamValues() "
                        "Failed to find start of property name"));
            return String();
        }
        propStart++;
        Uint32 propEnd = contentStr.find(propStart, '\"');
        if(propEnd == PEG_NOT_FOUND)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getParamValues() "
                        "Failed to find end of property name"));
            return String();
        }
        CIMName pName(contentStr.subString(propStart, propEnd - propStart));
        cout << "Name = "<<pName.getString()<<endl;
        pNames.append(pName);
        currentPos = contentStr.find(propEnd+1, ':');
        if(currentPos == PEG_NOT_FOUND)
        {
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getParamValues() "
                        "Failed to find property value : separator"));
            return String();
        }
        Uint32 startBlkPos = contentStr.find(currentPos,'{');
        Uint32 startArrayPos = contentStr.find(currentPos,'[');
        nextPos = contentStr.find(currentPos,',');

        Boolean isArrayValue = false;
        String arrayValue;

        Boolean isEmbeddedValue = false;
        String embeddedValue;

        if(startBlkPos < nextPos)
        {
            if(startArrayPos < startBlkPos)
                isArrayValue = true;
            else
                isEmbeddedValue = true;
        }
        else if(startArrayPos < nextPos)
            isArrayValue = true;

        if(isEmbeddedValue)
        {
            // there is an embedded block of {  } in the value
            // find end of this open brace
            Uint32 endBlkPos = findEndBlock(contentStr, startBlkPos+1,
                                            '{', '}');
            if(endBlkPos == PEG_NOT_FOUND)
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getParamValues() Failed to read "
                        "embedded property value. End of blk not found."));
                return String();
            }
            isEmbeddedValue = true;
            embeddedValue = contentStr.subString(startBlkPos,
                                                 endBlkPos-startBlkPos+1);
            currentPos = endBlkPos+1;
            nextPos = contentStr.find(currentPos,',');
        }
        else if(isArrayValue)
        {
            // there is an array block of [ ] in the value
            // find end of this open ]
            Uint32 endArrayPos = findEndBlock(contentStr, startArrayPos+1,
                                              '[', ']');
            if(endArrayPos == PEG_NOT_FOUND)
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getParamValues() Failed to read "
                        "array property value. End of blk not found."));
                return String();
            }
            isArrayValue = true;
            // Copy the contents of the array and drop [ ]
            arrayValue = contentStr.subString(startArrayPos + 1,
                                            endArrayPos-startArrayPos - 1);
            currentPos = endArrayPos+1;
            nextPos = contentStr.find(currentPos,',');
        }
        if (nextPos == PEG_NOT_FOUND || nextPos > endPos)
        {
            nextPos = endPos;
        }

        if(isEmbeddedValue)
        {
            pValues.append(embeddedValue);
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getParamValues() Name: [%s], Value: [%s]",
                    (const char*)pName.getString().getCString(),
                    (const char*)embeddedValue.getCString()));
            pIsArray.append(false);
        }
        else if(isArrayValue)
        {
            pValues.append(arrayValue);
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getParamValues() Name: [%s], Value: [%s]",
                    (const char*)pName.getString().getCString(),
                    (const char*)arrayValue.getCString()));
            pIsArray.append(true);
        }
        else
        {
            pIsArray.append(false);
            currentPos++; // skip over the :


            // Process this value further:
            // 1. Remove leading and trailing whitespaces
            // 2. If it is a string, remove double quotes
            // 3. ??

            while(content[currentPos] == ' ' ||
                    content[currentPos] == '\t' ||
                    content[currentPos] == '\r' ||
                    content[currentPos] == '\n' ||
                    content[currentPos] == '"')
            {
                if (content[currentPos] == '"' &&
                       currentPos>0 &&
                       content[currentPos-1] == '"')
                {
                    break;
                }
                currentPos++;
            }

            Uint32 newPos = nextPos - 1;
            while(content[newPos] == ' ' ||
                    content[newPos] == '\t' ||
                    content[newPos] == '\r' ||
                    content[newPos] == '\n' ||
                    content[newPos] == '"')
            {
                if (content[newPos] == '"' &&
                       newPos+1<contentSize &&
                       content[newPos+1] == '"')
                {
                    break;
                }
                newPos--;
            }

            String pValue(contentStr.subString(currentPos,
                                               newPos-currentPos+1));
            cout << "Value = "<<pValue<<endl;

            pValues.append(pValue);
            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::getParamValues() Name: [%s], Value: [%s]",
                    (const char*)pName.getString().getCString(),
                    (const char*)pValue.getCString()));
        }

        currentPos = nextPos+1;
        if (currentPos < endPos)
        {
            nextPos = contentStr.find(currentPos,':');
        }
    }

    for (Uint32 x=0; x<pNames.size(); x++)
    {
        Uint32 index = method.findParameter(pNames[x]);
        CIMConstParameter p = method.getParameter(index);
        CIMType pType = p.getType();

        if(pIsArray[x])
        {
            // there is an array with [] in the value
            // parse the values from the array

            // see if there are embedded instances in the array:
            Boolean isEmbeddedArray = false;
            if(pType == CIMTYPE_STRING &&
               p.findQualifier(CIMName("EmbeddedInstance")) != PEG_NOT_FOUND)
                isEmbeddedArray = true;

            // split all array elements into an Array<const char *>
            Array<const char *> elements;
            Array<String> elementStrings;
            Array<CIMInstance> embInsts;

            Uint32 currentPos = 0;
            while(currentPos < pValues[x].size())
            {
                Uint32 valEndPos, valStartPos;
                if(isEmbeddedArray)
                {
                    valStartPos = pValues[x].find(currentPos, '{');
                    if(valStartPos == PEG_NOT_FOUND)
                        break; // we have reached the end

                    valEndPos = findEndBlock(pValues[x], valStartPos+1,
                                             '{', '}');
                    currentPos = valEndPos + 1;
                    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                            "RsProcessor::getParamValues() EmbInst: [%s]",
                            (const char*)pValues[x].subString(valStartPos,
                                      valEndPos-valStartPos+1).getCString()));
                    // get the embedded instance
                    CIMInstance embInst =
                            getEmbeddedInstance(pValues[x].subString(
                                                 valStartPos, 
                                                 valEndPos-valStartPos+1),
                                                repository,
                                                reqUri);
                    embInsts.append(embInst);
                }
                else
                {
                    valStartPos = currentPos;
                    valEndPos = pValues[x].find(currentPos, ',');
                    if(valEndPos == PEG_NOT_FOUND)
                        valEndPos = pValues[x].size() - 1;
                    else
                        valEndPos--; // exclude the ,

                    // set currentPos to after the ,
                    currentPos = pValues[x].find(valEndPos, ',');
                    if(currentPos != PEG_NOT_FOUND)
                        currentPos++;

                    // Process this value further:
                    // 1. Remove leading and trailing whitespaces
                    // 2. If it is a string, remove double quotes

                    while(pValues[x][valStartPos] == ' ' ||
                            pValues[x][valStartPos] == '\t' ||
                            pValues[x][valStartPos] == '\r' ||
                            pValues[x][valStartPos] == '\n' ||
                            pValues[x][valStartPos] == '"')
                        valStartPos++;

                    while(pValues[x][valEndPos] == ' ' || 
                            pValues[x][valEndPos] == '\t' ||
                            pValues[x][valEndPos] == '\r' ||
                            pValues[x][valEndPos] == '\n' ||
                            pValues[x][valEndPos] == '"')
                        valEndPos--;

                    String strVal = pValues[x].subString(valStartPos,
                                      valEndPos-valStartPos+1);
                    elementStrings.append(strVal);
                    const char* charPtr = strdup(strVal.getCString());
                    elements.append(charPtr);
                    PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                            "RsProcessor::getParamValues() array element"
                                "[%d]: [%s]", elementStrings.size(),
                            (const char*)strVal.getCString()));
                }
            }
            if(!isEmbeddedArray)
            {
                CIMValue pVal = XmlReader::stringArrayToValue(0,
                                                elements, pType);

                // free elements
                for (Uint32 k=0; k<elements.size(); k++)
                {
                    free((void*)elements[k]);
                }
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getParamValues() array element "
                          "[%d]: [%s]",
                        x,
                        (const char*)pVal.toString().getCString()));

                inParms.append(CIMParamValue(pNames[x].getString(), pVal));
            }
            else
            {
                CIMValue pVal(embInsts);
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::getParamValues() array element "
                          "[%d]: [%s]",
                        x,
                        (const char*)pVal.toString().getCString()));
                inParms.append(CIMParamValue(pNames[x].getString(),pVal));
            }
        }
        // check to see if this is an embedded instance.
        else if(pType == CIMTYPE_STRING &&
           p.findQualifier(CIMName("EmbeddedInstance")) != PEG_NOT_FOUND)
        {

            // this is an embedded instance. process it here
            CIMInstance embInst = getEmbeddedInstance(pValues[x],
                                                      repository, reqUri);
            CIMValue pVal(embInst);
            inParms.append(CIMParamValue(pNames[x].getString(),pVal));
        }
        else
        {
            CIMValue pVal = XmlReader::stringToValue(0,
                                       pValues[x].getCString(),pType);
            inParms.append(CIMParamValue(pNames[x].getString(),pVal));
        }
    }

    PEG_METHOD_EXIT();
    return String();
}

void RsProcessor::handleRequest(RsHTTPRequest* request)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
        "RsProcessor::handleRequest(RsHTTPRequest* message)");

    AutoPtr<RsHTTPRequest> rsRequestDestroyer(request);
    AutoPtr<CIMOperationRequestMessage> cimRequest;

    RsURI& uri = request->getURI();
    RsHTTPResponse* response = request->response;

    try
    {
        const CIMNamespaceName& namespaceName =  uri.getNamespaceName();
        const CIMName& className = uri.getClassName();
        CIMClass cimClass;
        CIMObjectPath objectPath;

        if (String::equal(request->method, "OPTIONS"))
        {
            AutoPtr<HTTPMessage> httpMessage;
            MessageQueue* queue = MessageQueue::lookup(request->queueId);
            response->setStatus(STRLIT_ARGS(HTTP_STATUS_OK));
            httpMessage.reset(response->getHTTPMessage());
            queue->enqueue(httpMessage.release());
        }
        else
        {
            // Save the request until the response comes back.
            if(_requestTable.contains(request->queueId))
            {
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                        "RsProcessor::handleRequest() request table "
                        "contains stale request"));
                _requestTable.remove(request->queueId);
            }

            _requestTable.insert(request->queueId, request);

            if (request->hasRange())
            {
                response->setStatus(STRLIT_ARGS(HTTP_PARTIALCONTENT));
            }
            else
            {
                response->setStatus(STRLIT_ARGS(HTTP_STATUS_OK));
            }

            switch(request->getType())
            {
            case RS_METHOD_POST:
            {
                cimClass = _repository->getClass(
                        namespaceName,
                        className,
                        false /*localOnly*/);
                objectPath = uri.getMethodPath(cimClass);
                CIMName mName = uri.getMethodName();

                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() GET Method - "
                        "namespace [%s], methodPath [%s], Parameters: ",
                        (const char*)namespaceName.getString().getCString(),
                        (const char*)objectPath.toString().getCString()));


                Uint32 mIndex = cimClass.findMethod(mName);
                CIMConstMethod mthd = cimClass.getMethod(mIndex);

                // inparams are in the content of the request
                Array<CIMParamValue> inParameters_;
                getParamValues(mthd,
                               inParameters_,
                               request->getContentStart(),
                               request->contentLength,
                               _repository,
                               uri);

                cimRequest.reset( new CIMInvokeMethodRequestMessage(
                        CIMRS_MESSAGE_ID,
                        namespaceName,
                        objectPath,
                        mName,
                        inParameters_,
                        QueueIdStack(request->queueId)));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

                break;
            }
            case RS_ASSOCIATION_GET:
                cimClass = _repository->getClass(
                    namespaceName,
                    className,
                    false /*localOnly*/);
                objectPath = uri.getAssociationPath(cimClass);

                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() GET Association - "
                        "namespace [%s], associationPath [%s], Parameters: ",
                        (const char*)namespaceName.getString().getCString(),
                        (const char*)objectPath.toString().getCString()));

                cimRequest.reset(new CIMAssociatorsRequestMessage(
                    CIMRS_MESSAGE_ID,
                    namespaceName,
                    objectPath,
                    uri.getAssociationClassName(),
                    CIMName(),
                    String(),
                    uri.getAssociatedRoleName(),
                    false, // includeQualifiers
                    false, // includeClassOrigin
                    uri.getPropertyList(),
                    QueueIdStack(request->queueId)));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));
                break;

            case RS_REFERENCE_GET:
                cimClass = _repository->getClass(
                    namespaceName,
                    className,
                    false /*localOnly*/);
                objectPath = uri.getReferencePath(cimClass);

                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() GET References - "
                        "namespace [%s], objectPath [%s], ResultClass [%s] ",
                        (const char*)namespaceName.getString().getCString(),
                        (const char*)objectPath.toString().getCString(),
                        (const char*)uri.getNavString().getCString()));

                cimRequest.reset(new CIMReferencesRequestMessage(
                    CIMRS_MESSAGE_ID,
                    namespaceName,
                    objectPath,
                    uri.getNavString(),
                    String(),
                    false, // includeQualifiers
                    false, // includeClassOrigin
                    uri.getPropertyList(),
                    QueueIdStack(request->queueId)));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

                break;
            case RS_INSTANCE_MEMBER_GET:
                cimClass = _repository->getClass(
                    namespaceName,
                    className,
                    false /*localOnly*/);
                uri.setRepository(_repository);
                objectPath = uri.getInstancePath(cimClass);
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() GET Instance member - "
                        "namespace [%s], objectPath [%s]",
                        (const char*)namespaceName.getString().getCString(),
                        (const char*)objectPath.toString().getCString()));

                cimRequest.reset(new CIMGetInstanceRequestMessage(
                    CIMRS_MESSAGE_ID,
                    namespaceName,
                    objectPath,
                    false, // includeQualifiers
                    false, // includeclassorigin
                    uri.getPropertyList(),
                    QueueIdStack(request->queueId),
                    request->authType,
                    request->userName));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

                break;
            case RS_INSTANCE_COLLECTION_GET:
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() GET Instance collection"
                        " - namespace [%s], className [%s]; Parameters: "
                        "DeepInheritance: %d, Properties: %d",
                        (const char*)namespaceName.getString().getCString(),
                        (const char*)className.getString().getCString(),
                        uri.hasDeepInheritance(),
                        uri.getPropertyList().size()));

                cimRequest.reset(new CIMEnumerateInstancesRequestMessage(
                    CIMRS_MESSAGE_ID,
                    namespaceName,
                    className,
                    uri.hasDeepInheritance(),
                    false, // includeQualifiers
                    false, // includeClassOrigin
                    uri.getPropertyList(),
                    QueueIdStack(request->queueId),
                    request->authType,
                    request->userName));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

                break;
            case RS_CLASS_MEMBER_GET:
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() GET Class member - "
                        "namespace [%s], className [%s];",
                        (const char*)namespaceName.getString().getCString(),
                        (const char*)className.getString().getCString()));

                cimRequest.reset(new CIMGetClassRequestMessage(
                    CIMRS_MESSAGE_ID,
                    namespaceName,
                    className,
                    false, // localOnly
                    uri.hasQualifier(),
                    false, // includeClassOrigin
                    uri.getPropertyList(),
                    QueueIdStack(request->queueId)));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

                break;
            case RS_CLASS_COLLECTION_GET:
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() GET Class collection - "
                        "namespace [%s]",
                        (const char*)namespaceName.getString().getCString()));

                cimRequest.reset(new CIMEnumerateClassesRequestMessage(
                    CIMRS_MESSAGE_ID,
                    namespaceName,
                    CIMName(), // className,
                    uri.hasDeepInheritance(),
                    false, // localOnly,
                    uri.hasQualifier(),
                    false, // includeClassOrigin,
                    QueueIdStack(request->queueId)));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

                break;
            case RS_INSTANCE_CREATE_POST:
                PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                        "RsProcessor::handleRequest() POST Instance Create - "
                        "namespace [%s]",
                        (const char*)namespaceName.getString().getCString()));

                String contentStr(request->getContentStart(),
                                  request->contentLength);
                CIMInstance instance =
                        getEmbeddedInstance(contentStr, _repository, uri);

                cimRequest.reset(new CIMCreateInstanceRequestMessage(
                        CIMRS_MESSAGE_ID,
                        namespaceName,
                        instance,
                        QueueIdStack(request->queueId)));

                response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

                break;
            }

            if (cimRequest.get())
            {
                cimRequest->operationContext.insert(
                    IdentityContainer(request->userName));
#ifdef PEGASUS_OS_ZOS
                //TBD: Use a real SocketID here
                //cimRequest->operationContext.insert(
                //    SocketInfoContainer(4, true));
#endif
                cimRequest->operationContext.set(
                    AcceptLanguageListContainer(request->acceptLanguages));
                cimRequest->operationContext.set(
                    ContentLanguageListContainer(request->contentLanguages));
                cimRequest->setHttpMethod(request->httpMethod);
                cimRequest->setCloseConnect(request->httpCloseConnect);
            }
            cimRequest->queueIds.push(getQueueId());

            PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL4,
                    "RsProcessor::handleRequest()- Delegating Request - "
                    "Request Queue [%d], Processor Queue [%d]",
                    request->queueId, getQueueId()));

            _cimOperationProcessorQueue->enqueue(cimRequest.release());
            rsRequestDestroyer.release();
        }

        PEG_METHOD_EXIT();
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest() CIMException thrown: %s / %s",
                (const char*)e.getMessage().getCString(),
                cimStatusCodeToString( e.getCode() )));

        switch(e.getCode())
        {
        case CIM_ERR_INVALID_NAMESPACE:
        case CIM_ERR_NOT_FOUND:
        case CIM_ERR_INVALID_CLASS:
            request->response->setStatus(
                STRLIT_ARGS(HTTP_STATUS_NOTFOUND));
            break;
        default:
            request->response->setStatus(
                STRLIT_ARGS(HTTP_STATUS_INTERNALSERVERERROR));
        }
        response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

        JSONWriter* writer = request->response->getJSONWriter();
        writer->append(e, request->method, uri);

        MessageQueue* queue = MessageQueue::lookup(request->queueId);
        _requestTable.remove(queue->getQueueId());

        AutoPtr<HTTPMessage> httpMessage(request->response->getHTTPMessage());
        queue->enqueue(httpMessage.release());
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest()- Exception thrown: %s",
                (const char*)e.getMessage().getCString()));

        request->response->setStatus(
            STRLIT_ARGS(HTTP_STATUS_INTERNALSERVERERROR));
        response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

        JSONWriter* writer = request->response->getJSONWriter();
        writer->append(e, request->method, uri);

        MessageQueue* queue = MessageQueue::lookup(request->queueId);
        _requestTable.remove(queue->getQueueId());

        AutoPtr<HTTPMessage> httpMessage(request->response->getHTTPMessage());
        queue->enqueue(httpMessage.release());
    }
    catch (PEGASUS_STD(exception)& e)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest()- PEGASUS_STD thrown: %s",
                e.what()));
        PEGASUS_ASSERT(false);

    }
    catch (...)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest()- Other exception"));
        PEGASUS_ASSERT(false);
    }
}

void RsProcessor::handleResponse(CIMResponseMessage* cimResponse)
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
        "RsProcessor::handleResponse()");

    AutoPtr<CIMResponseMessage> cimResponseDestroyer(cimResponse);
    Uint32 queueId = cimResponse->queueIds.top();
    MessageQueue* queue = MessageQueue::lookup(queueId);
    CIMException& cimException = cimResponse->cimException;
    Boolean httpCloseConnect = cimResponse->getCloseConnect();
    Boolean complete = cimResponse->isComplete();

    RsHTTPRequest* request;
    PEGASUS_FCT_EXECUTE_AND_ASSERT(
       true, _requestTable.lookup(queueId, request));
    RsHTTPResponse *response = request->response;

    try
    {
        if (cimException.getCode() != CIM_ERR_SUCCESS)
        {
            throw cimException;
        }

        JSONWriter* writer = response->getJSONWriter();

        switch(request->getType())
        {
        case RS_METHOD_POST:
            writer->append(
                (CIMInvokeMethodResponseMessage*)cimResponse,
                _repository,
                request->getURI());
            break;
        case RS_ASSOCIATION_GET:
            writer->append(
                    (CIMAssociatorsResponseMessage*)cimResponse,
                    _repository,
                    request->getURI());
            break;
        case RS_REFERENCE_GET:
            writer->append(
                    (CIMReferencesResponseMessage*)cimResponse,
                    _repository,
                    request->getURI());
            break;
        case RS_INSTANCE_COLLECTION_GET:
            writer->append(
                (CIMEnumerateInstancesResponseMessage*)cimResponse,
                request->getRangeStart(),
                request->getRangeEnd(),
                _repository,
                request->getURI());

            if (complete && request->hasRange())
            {
                response->setRange(
                    request->getRangeStart(),
                    request->getRangeEnd(),
                    writer->getEnumerationCount());
            }
            break;
        case RS_INSTANCE_MEMBER_GET:
            writer->append(
                (CIMGetInstanceResponseMessage*)cimResponse,
                _repository,
                request->getURI());
            break;
        case RS_CLASS_MEMBER_GET:
            writer->append(
                (CIMGetClassResponseMessage*)cimResponse);
            break;
        case RS_CLASS_COLLECTION_GET:
           // writer->append(
           //     );
            break;
        case RS_INSTANCE_CREATE_POST:
            // On success CREATE does not need to send any content back!
            break;
        }

        if (complete)
        {
            AutoPtr<HTTPMessage> httpMessage(response->getHTTPMessage());
            httpMessage->setCloseConnect(httpCloseConnect);
            httpMessage->setComplete(complete);
            //httpMessage->setIndex(cimResponse->getIndex());
            httpMessage->setIndex(0);
            PEGASUS_FCT_EXECUTE_AND_ASSERT(true, _requestTable.remove(queueId));

            delete request;

            queue->enqueue(httpMessage.release());
        }

        PEG_METHOD_EXIT();
    }
    catch (CIMException& e)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest() CIMException thrown: %s / %s",
                (const char*)e.getMessage().getCString(),
                cimStatusCodeToString( e.getCode() )));

        switch(e.getCode())
        {
        case CIM_ERR_NOT_FOUND:
        case CIM_ERR_INVALID_CLASS:
        case CIM_ERR_INVALID_NAMESPACE:
            request->response->setStatus(
                STRLIT_ARGS(HTTP_STATUS_NOTFOUND));
            break;
        default:
            request->response->setStatus(
                STRLIT_ARGS(HTTP_STATUS_INTERNALSERVERERROR));
        }
        response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

        JSONWriter* writer = request->response->getJSONWriter();
        writer->append(e, request->method, request->getURI());

        MessageQueue* queue = MessageQueue::lookup(request->queueId);
        _requestTable.remove(queue->getQueueId());

        AutoPtr<HTTPMessage> httpMessage(request->response->getHTTPMessage());
        queue->enqueue(httpMessage.release());
    }
    catch (Exception& e)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest()- Exception thrown: %s",
                (const char*)e.getMessage().getCString()));

        request->response->setStatus(
            STRLIT_ARGS(HTTP_STATUS_INTERNALSERVERERROR));
        response->setContentType(STRLIT_ARGS(CIM_RS_CONTENT_TYPE));

        JSONWriter* writer = request->response->getJSONWriter();
        writer->append(e, request->method, request->getURI());

        MessageQueue* queue = MessageQueue::lookup(request->queueId);
        _requestTable.remove(queue->getQueueId());

        AutoPtr<HTTPMessage> httpMessage(request->response->getHTTPMessage());
        queue->enqueue(httpMessage.release());
    }
    catch (PEGASUS_STD(exception)& e)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest()- PEGASUS_STD thrown: %s",
                e.what()));
        PEGASUS_ASSERT(false);

    }
    catch (...)
    {
        PEG_TRACE((TRC_RSSERVER, Tracer::LEVEL1,
                "RsProcessor::handleRequest()- Other exception"));
        PEGASUS_ASSERT(false);
    }
}


Uint32 RsProcessor::getRsRequestDecoderQueueId()
{
    PEG_METHOD_ENTER(TRC_RSSERVER,
        "RsProcessor::getRsRequestDecoderQueueId()");
    PEG_METHOD_EXIT();
    return _rsRequestDecoder.getQueueId();
}


PEGASUS_NAMESPACE_END
