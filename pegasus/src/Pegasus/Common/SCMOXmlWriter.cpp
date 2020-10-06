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
// This code implements part of PEP#348 - The CMPI infrastructure using SCMO
// (Single Chunk Memory Objects).
// The design document can be found on the OpenPegasus website openpegasus.org
// at https://collaboration.opengroup.org/pegasus/pp/documents/21210/PEP_348.pdf
//
//%/////////////////////////////////////////////////////////////////////////////

#include <Pegasus/Common/Config.h>
#include <cstdlib>
#include <cstdio>
#include <Pegasus/Common/SCMOXmlWriter.h>
#include "Tracer.h"

PEGASUS_NAMESPACE_BEGIN

void SCMOXmlWriter::buildPropertyFilterNodesArray(
     Array<Uint32> & nodes,
     const SCMOClass * classPtr,
     const CIMPropertyList & propertyList)
{
    for (Uint32 i=0,k=propertyList.size(); i<k; i++)
    {
        Uint32 node = 0;
        const CIMName & name = propertyList[i];
        SCMO_RC rc =
            classPtr->_getPropertyNodeIndex(
                node,
                (const char *)name.getString().getCString());
        if(rc == SCMO_OK)
        {
            nodes.append(node);
        }
    }
}

const Array<Uint32> & SCMOXmlWriter::getFilteredNodesArray(
     Array<propertyFilterNodesArray_t> & propFilterNodesArrays,
     const SCMOInstance& scmoInstance,
     const CIMPropertyList & propertyList)
{
    //First see if the class ptr is already stored in the propFilterNodesArrays
    const SCMOClass * classPtr = scmoInstance.inst.hdr->theClass.ptr;
    SCMBClass_Main * classPtrMemBlock = classPtr->cls.hdr;
    for (int i=0, k=propFilterNodesArrays.size(); i < k; i++)
    {
        if (classPtrMemBlock == propFilterNodesArrays[i].classPtrMemBlock)
        {
            return propFilterNodesArrays[i].nodes;
        }
    }

    // Could not find the class pointer of this SCMOInstance in the
    // property filter nodes array
    // --> need to create the new entry and return that
    propertyFilterNodesArray_t newEntry;
    newEntry.classPtrMemBlock = classPtrMemBlock;
    SCMOXmlWriter::buildPropertyFilterNodesArray(
        newEntry.nodes,
        classPtr,
        propertyList);
    propFilterNodesArrays.append(newEntry);

    // return the new nodes entry, but as a reference into the array
    return propFilterNodesArrays[propFilterNodesArrays.size()-1].nodes;
}

void SCMOXmlWriter::appendValueSCMOInstanceElements(
     Buffer& out,
     const Array<SCMOInstance> & _scmoInstances,
     const CIMPropertyList & propertyList)
{
    if (propertyList.isNull())
    {
        Array<Uint32> emptyNodes;
        for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
        {
            SCMOXmlWriter::appendValueSCMOInstanceElement(
                out,
                _scmoInstances[i],
                false,
                emptyNodes);
        }
    }
    else
    {
        Array<propertyFilterNodesArray_t> propFilterNodesArrays;

        for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
        {
            // This searches for an already created array of nodes,
            // if not found, creates it inside propFilterNodesArrays
            const Array<Uint32> & nodes=
                SCMOXmlWriter::getFilteredNodesArray(
                    propFilterNodesArrays,
                    _scmoInstances[i],
                    propertyList);

            SCMOXmlWriter::appendValueSCMOInstanceElement(
                out,
                _scmoInstances[i],
                true,
                nodes);
        }
    }
}

// EXP_PULL_BEGIN
void SCMOXmlWriter::appendValueSCMOInstanceWithPathElements(
     Buffer& out,
     const Array<SCMOInstance> & _scmoInstances,
     const CIMPropertyList & propertyList)
{
    if (propertyList.isNull())
    {
        Array<Uint32> emptyNodes;
        for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
        {
            SCMOXmlWriter::appendValueInstanceWithPathElement(
                out,
                _scmoInstances[i],
                false,
                emptyNodes);
        }
    }
    else
    {
        Array<propertyFilterNodesArray_t> propFilterNodesArrays;

        for (Uint32 i = 0, n = _scmoInstances.size(); i < n; i++)
        {
            // This searches for an already created array of nodes,
            // if not found, creates it inside propFilterNodesArrays
            const Array<Uint32> & nodes=
                SCMOXmlWriter::getFilteredNodesArray(
                    propFilterNodesArrays,
                    _scmoInstances[i],
                    propertyList);

            SCMOXmlWriter::appendValueInstanceWithPathElement(
                out,
                _scmoInstances[i],
                true,
                nodes);
        }
    }
}
//EXP_PULL_END

void SCMOXmlWriter::appendValueSCMOInstanceElement(
    Buffer& out,
    const SCMOInstance& scmoInstance,
    bool filtered,
    const Array<Uint32> & nodes)

{
    out << STRLIT("<VALUE.NAMEDINSTANCE>\n");

    appendInstanceNameElement(out, scmoInstance);
    appendInstanceElement(out, scmoInstance,filtered,nodes);

    out << STRLIT("</VALUE.NAMEDINSTANCE>\n");
}

void SCMOXmlWriter::appendInstanceNameElement(
    Buffer& out,
    const SCMOInstance& scmoInstance)
{
    out << STRLIT("<INSTANCENAME CLASSNAME=\"");
    Uint32 len;
    const char * className = scmoInstance.getClassName_l(len);
    out.append(className,len);
    // TODO: check performance impact
    out.append('"','>','\n');

    for (Uint32 i = 0, n = scmoInstance.getKeyBindingCount(); i < n; i++)
    {
        const char * kbName;
        const SCMBUnion * kbValue;
        CIMType kbType;
        Uint32 kbNameLen;

        SCMO_RC smrc = scmoInstance._getKeyBindingDataAtNodeIndex(
            i,
            &kbName,
            kbNameLen,
            kbType,
            &kbValue);

        out << STRLIT("<KEYBINDING NAME=\"");
        out.append(kbName,kbNameLen-1);
        out.append('"','>','\n');

        if (kbType == CIMTYPE_REFERENCE)
        {
            if (SCMO_OK == smrc)
            {
                SCMOInstance * ref = kbValue->extRefPtr;
                appendValueReferenceElement(out, *ref);
            }
        }
        else
        {
            out << STRLIT("<KEYVALUE VALUETYPE=\"");
            out << xmlWriterKeyTypeStrings(kbType);
            out.append('"','>');

            if (SCMO_OK == smrc)
            {
                SCMOXmlWriter::appendSCMBUnion(
                    out,
                    *kbValue,
                    kbType,
                    scmoInstance.inst.base);
            }
            out << STRLIT("</KEYVALUE>\n");
        }
        out << STRLIT("</KEYBINDING>\n");
    }
    out << STRLIT("</INSTANCENAME>\n");
}

//------------------------------------------------------------------------------
//
// appendInstanceElement()
//
//     <!ELEMENT INSTANCE
//         (QUALIFIER*,(PROPERTY|PROPERTY.ARRAY|PROPERTY.REFERENCE)*)>
//     <!ATTLIST INSTANCE
//         %ClassName;>
//
//------------------------------------------------------------------------------
void SCMOXmlWriter::appendInstanceElement(
    Buffer& out,
    const SCMOInstance& scmoInstance,
    bool filtered,
    const Array<Uint32> & nodes)
{
    // Class opening element:

    out << STRLIT("<INSTANCE CLASSNAME=\"");
    Uint32 len;
    const char * className = scmoInstance.getClassName_l(len);
    out.append(className,len);
    out.append('"',' ','>','\n');
    //out << STRLIT("\" >\n");

    // Append Instance Qualifiers:
    if (scmoInstance.inst.hdr->flags.includeQualifiers)
    {
        SCMBClass_Main *classMain=scmoInstance.inst.hdr->theClass.ptr->cls.hdr;
        char* clsbase = scmoInstance.inst.hdr->theClass.ptr->cls.base;

        SCMBQualifier *theArray =
            (SCMBQualifier*)&(clsbase[classMain->qualifierArray.start]);
        // need to iterate
        for (Uint32 i=0, n=classMain->numberOfQualifiers;i<n;i++)
        {
            SCMOXmlWriter::appendQualifierElement(out,theArray[i],clsbase);
        }
    }

    // Append Properties:
    if(!filtered)
    {
        for (Uint32 i=0,k=scmoInstance.getPropertyCount();i<k;i++)
        {
            SCMOXmlWriter::appendPropertyElement(out,scmoInstance,i);
        }
    }
    else
    {
        for (Uint32 i=0,k=nodes.size();i<k;i++)
        {
            SCMOXmlWriter::appendPropertyElement(out,scmoInstance,nodes[i]);
        }
    }
    // Instance closing element:
    out << STRLIT("</INSTANCE>\n");
}

//------------------------------------------------------------------------------
//
// appendQualifierElement()
//
//     <!ELEMENT QUALIFIER (VALUE|VALUE.ARRAY)>
//     <!ATTLIST QUALIFIER
//              %CIMName;
//              %CIMType;               #REQUIRED
//              %Propagated;
//              %QualifierFlavor;>
//
//------------------------------------------------------------------------------

void SCMOXmlWriter::appendQualifierElement(
    Buffer& out,
    const SCMBQualifier& theQualifier,
    const char* base)
{
    out << STRLIT("<QUALIFIER NAME=\"");
    if(theQualifier.name == QUALNAME_USERDEFINED)
    {
        if (theQualifier.userDefName.start)
        {
            out.append(
                &(base[theQualifier.userDefName.start]),
                theQualifier.userDefName.size-1);
        }
    }
    else
    {
        out << SCMOClass::qualifierNameStrLit(theQualifier.name);
    }
    out.append('"',' ');
    //out << STRLIT("\" ");

    // Append type
    out << xmlWriterTypeStrings(theQualifier.value.valueType);
    // append PROPAGATED if set
    if (theQualifier.propagated)
    {
        out << STRLIT(" PROPAGATED=\"true\"");
    }
    // append flavor of the qualifier
    SCMOXmlWriter::appendQualifierFlavorEntity(
        out,
        CIMFlavor(theQualifier.flavor));

    out.append('>','\n');
    //out << STRLIT(">\n");
    // append the value of the qualifier
    SCMOXmlWriter::appendValueElement(out, theQualifier.value, base);

    out << STRLIT("</QUALIFIER>\n");
}

//------------------------------------------------------------------------------
//
// appendPropertyElement()
//
//     <!ELEMENT PROPERTY (QUALIFIER*,VALUE?)>
//     <!ATTLIST PROPERTY
//              %CIMName;
//              %CIMType;           #REQUIRED
//              %ClassOrigin;
//              %Propagated;>
//
//     <!ELEMENT PROPERTY.ARRAY (QUALIFIER*,VALUE.ARRAY?)>
//     <!ATTLIST PROPERTY.ARRAY
//              %CIMName;
//              %CIMType;           #REQUIRED
//              %ArraySize;
//              %ClassOrigin;
//              %Propagated;>
//
//     <!ELEMENT PROPERTY.REFERENCE (QUALIFIER*,VALUE.REFERENCE?)>
//     <!ATTLIST PROPERTY.REFERENCE
//              %CIMName;
//              %ReferenceClass;
//              %ClassOrigin;
//              %Propagated;>
//
//------------------------------------------------------------------------------

void SCMOXmlWriter::appendPropertyElement(
    Buffer& out,
    const SCMOInstance& scmoInstance,
    Uint32 pos)
{

    // Absolute pointer to the property SCMBValue
    SCMBValue * propertyValue;
    const char * propertyValueBase;

    // Absolute pointer at a SCMBValue
    SCMBClassProperty * propertyDef;
    // Absolute pointer to the the class info for the given
    // instance starts
    const char* clsbase = scmoInstance.inst.hdr->theClass.ptr->cls.base;

    scmoInstance._getPropertyAt(
        pos,
        &propertyValue,
        &propertyValueBase,
        &propertyDef);

    bool noClassExists = scmoInstance.inst.hdr->flags.noClassForInstance;
    bool includeClassOrigin = scmoInstance.inst.hdr->flags.includeClassOrigin;
    bool includeQualifiers = scmoInstance.inst.hdr->flags.includeQualifiers;
    // This one either false or comes from class definition
    bool propagated = false;

    // Get property name and set any output parameter values
    const char * pName;
    Uint32 pLen;
    if (scmoInstance._isClassDefinedProperty(pos))
    {
        // Get name from class Definition
        pName = &(clsbase[propertyDef->name.start]);
        pLen = (propertyDef->name.size-1);
        propagated = propertyDef->flags.propagated;
    }
    else
    {
        // Get name from Instance
        SCMBUserPropertyElement* pElement =
            scmoInstance._getUserDefinedPropertyElementAt(pos);

        pName = _getCharString(pElement->name,scmoInstance.inst.base);
        pLen = pElement->name.size-1;
        includeQualifiers = false;
    }
    // KS_TODO clarify classOrigin acquisition.
    CIMType propertyType = propertyValue->valueType;

    if (propertyValue->flags.isArray)
    {
        Uint32 arraySize=propertyValue->valueArraySize;

        out << STRLIT("<PROPERTY.ARRAY NAME=\"");

        out.append(pName, pLen);

        out.append('"',' ');
        //out << STRLIT("\" ");
        if (propertyType == CIMTYPE_OBJECT)
        {
            // If the property array type is CIMObject, then
            //    encode the property in CIM-XML as a string array with the
            //    EmbeddedObject attribute (there is not currently a CIM-XML
            //    "object" datatype)
            out << STRLIT(" TYPE=\"string\"");
            // If the Embedded Object is an instance, always add the
            // EmbeddedObject attribute.
            SCMOInstance * instPtr = propertyValue->value.extRefPtr;
            if ((0 != instPtr) &&
                    (arraySize > 0) &&
                        !(instPtr->inst.hdr->flags.isClassOnly))
            {
                out << STRLIT(" EmbeddedObject=\"object\""
                                " EMBEDDEDOBJECT=\"object\"");
            }
        }
        else if (propertyType == CIMTYPE_INSTANCE)
        {
            // If the property array type is CIMInstance, then
            //   encode the property in CIM-XML as a string array with the
            //   EmbeddedObject attribute (there is not currently a CIM-XML
            //   "instance" datatype)
            out << STRLIT(" TYPE=\"string\"");
            // add the EmbeddedObject attribute
            if (arraySize > 0)
            {
                out << STRLIT(" EmbeddedObject=\"instance\""
                              " EMBEDDEDOBJECT=\"instance\"");
            }
        }
        // Every CYMType but CIMOBject, CIMInstance
        else
        {
            out.append(' ');
            out << xmlWriterTypeStrings(propertyType);
        }

        if (0 != arraySize)
        {
            out << STRLIT(" ARRAYSIZE=\"");
            SCMOXmlWriter::append(out, arraySize);
            out.append('"');
        }

        if (includeClassOrigin)
        {
            if (propertyDef->originClassName.start != 0)
            {
                out << STRLIT(" CLASSORIGIN=\"");
                out.append(
                    &(clsbase[propertyDef->originClassName.start]),
                    (propertyDef->originClassName.size-1));
                out.append('"');
            }
        }

        if (propagated)
        {
            out << STRLIT(" PROPAGATED=\"true\"");
        }

        out.append('>','\n');
        //out << STRLIT(">\n");

        // Append Instance Qualifiers:
        if (includeQualifiers)
        {
            SCMBQualifier * theArray=
                (SCMBQualifier*)
                    &(clsbase[propertyDef->qualifierArray.start]);
            // need to iterate
            for (Uint32 i=0, n=propertyDef->numberOfQualifiers;i<n;i++)
            {
                SCMOXmlWriter::appendQualifierElement(
                    out,
                    theArray[i],
                    clsbase);
            }
        }
        SCMOXmlWriter::appendValueElement(out,*propertyValue,propertyValueBase);
        out << STRLIT("</PROPERTY.ARRAY>\n");
    }

    // else Not an array type
    else if (propertyType == CIMTYPE_REFERENCE)
    {
        out << STRLIT("<PROPERTY.REFERENCE NAME=\"");
        out.append(pName, pLen);
        out.append('"',' ');
        //out << STRLIT("\" ");

        if ((!noClassExists) && (0 != propertyDef->refClassName.start))
        {
            out << STRLIT(" REFERENCECLASS=\"");
            out.append(
              &(clsbase[propertyDef->refClassName.start]),
              (propertyDef->refClassName.size-1));
            out.append('"');
        }

        if (includeClassOrigin)
        {
            if (propertyDef->originClassName.start != 0)
            {
                out << STRLIT(" CLASSORIGIN=\"");
                out.append(
                    &(clsbase[propertyDef->originClassName.start]),
                    (propertyDef->originClassName.size-1));
                out.append('"');
            }
        }
        if (propagated)
        {
            out << STRLIT(" PROPAGATED=\"true\"");
        }
        out.append('>','\n');
        //out << STRLIT(">\n");

        // Append Instance Qualifiers:
        if (includeQualifiers)
        {
            SCMBQualifier * theArray=
                (SCMBQualifier*)
                    &(clsbase[propertyDef->qualifierArray.start]);
            // need to iterate
            for (Uint32 i=0, n=propertyDef->numberOfQualifiers;i<n;i++)
            {
                SCMOXmlWriter::appendQualifierElement(
                    out,
                    theArray[i],
                    clsbase);
            }
        }
        SCMOXmlWriter::appendValueElement(out,*propertyValue,propertyValueBase);
        out << STRLIT("</PROPERTY.REFERENCE>\n");
    }
    else
    {
        out << STRLIT("<PROPERTY NAME=\"");

        out.append(pName, pLen);

        out.append('"',' ');
        //out << STRLIT("\" ");

        // User-defined properties will not includeClassOrigin
        if (includeClassOrigin)
        {
            if (propertyDef->originClassName.start != 0)
            {
                out << STRLIT(" CLASSORIGIN=\"");
                out.append(
                    &(clsbase[propertyDef->originClassName.start]),
                    (propertyDef->originClassName.size-1));
                out.append('"');
            }
        }
        if (propagated)
        {
            out << STRLIT(" PROPAGATED=\"true\"");
        }

        if (propertyType == CIMTYPE_OBJECT)
        {
            // If the property type is CIMObject, then
            //   encode the property in CIM-XML as a string with the
            //   EmbeddedObject attribute (there is not currently a CIM-XML
            //   "object" datatype)
            out << STRLIT(" TYPE=\"string\"");
            // If the Embedded Object is an instance, always add the
            // EmbeddedObject attribute.
            SCMOInstance * a = propertyValue->value.extRefPtr;
            if (a && !(a->inst.hdr->flags.isClassOnly))
            {
                out << STRLIT(" EmbeddedObject=\"object\""
                              " EMBEDDEDOBJECT=\"object\"");
            }
        }
        else if (propertyType == CIMTYPE_INSTANCE)
        {
            out << STRLIT(" TYPE=\"string\""
                          " EmbeddedObject=\"instance\""
                          " EMBEDDEDOBJECT=\"instance\"");
        }
        else
        {
            out.append(' ');
            out << xmlWriterTypeStrings(propertyType);
        }
        out.append('>','\n');
        //out << STRLIT(">\n");

        // Append Instance Qualifiers:
        if (includeQualifiers)
        {
            SCMBQualifier * theArray=
                (SCMBQualifier*)
                    &(clsbase[propertyDef->qualifierArray.start]);
            // Iterate through all qualifiers
            for (Uint32 i=0, n=propertyDef->numberOfQualifiers;i<n;i++)
            {
                SCMOXmlWriter::appendQualifierElement(
                    out,
                    theArray[i],
                    clsbase);
            }
        }
        SCMOXmlWriter::appendValueElement(out,*propertyValue,propertyValueBase);
        out << STRLIT("</PROPERTY>\n");
    }
}
//------------------------------------------------------------------------------
//
// appendValueElement()
//
//    <!ELEMENT VALUE (#PCDATA)>
//    <!ELEMENT VALUE.ARRAY (VALUE*)>
//    <!ELEMENT VALUE.REFERENCE
//        (CLASSPATH|LOCALCLASSPATH|CLASSNAME|INSTANCEPATH|LOCALINSTANCEPATH|
//         INSTANCENAME)>
//    <!ELEMENT VALUE.REFARRAY (VALUE.REFERENCE*)>
//
//------------------------------------------------------------------------------

void SCMOXmlWriter::appendValueElement(
    Buffer& out,
    const SCMBValue & value,
    const char * base)
{
    if (value.flags.isNull)
    {
        return;
    }
    if (value.flags.isArray)
    {
        appendSCMBUnionArray(
            out,
            value.value,
            value.valueType,
            value.valueArraySize,
            base);
    }
    else if (value.valueType == CIMTYPE_REFERENCE)
    {
        SCMOInstance * ref = value.value.extRefPtr;
        if (ref)
        {
            appendValueReferenceElement(out, *ref);
        }
    }
    else
    {
        out << STRLIT("<VALUE>");

        // Call function to write a SCMBUnion + type field
        appendSCMBUnion(out,value.value, value.valueType,base);

        out << STRLIT("</VALUE>\n");
    }
}

//------------------------------------------------------------------------------
//
// appendValueReferenceElement()
//
//    <!ELEMENT VALUE.REFERENCE
//        (CLASSPATH|LOCALCLASSPATH|CLASSNAME|INSTANCEPATH|LOCALINSTANCEPATH|
//         INSTANCENAME)>
//
//------------------------------------------------------------------------------
void SCMOXmlWriter::appendValueReferenceElement(
    Buffer& out,
    const SCMOInstance& ref)
{
    out << STRLIT("<VALUE.REFERENCE>\n");

    appendClassOrInstancePathElement(out, ref);

    out << STRLIT("</VALUE.REFERENCE>\n");
}

// Append either a class or instance Path Element
void SCMOXmlWriter::appendClassOrInstancePathElement(
    Buffer& out,
    const SCMOInstance& ref)
{
    // See if it is a class or instance reference (instance references have
    // key-bindings; class references do not).

    // differentiate between instance and class using the SCMO flag
    if (ref.inst.hdr->flags.isClassOnly)
    {
        // class
        if (0 != ref.inst.hdr->hostName.start)
        {
            appendClassPathElement(out, ref);
        }
        else if (0 != ref.inst.hdr->instNameSpace.start)
        {
            appendLocalClassPathElement(out, ref);
        }
        else
        {
            Uint32 classNameLength=0;
            const char* className = ref.getClassName_l(classNameLength);
            appendClassNameElement(out, className, classNameLength);
        }
    }
    else
    {
        // instance
        if (0 != ref.inst.hdr->hostName.start)
        {
            appendInstancePathElement(out, ref);
        }
        else if (0 != ref.inst.hdr->instNameSpace.start)
        {
            appendLocalInstancePathElement(out, ref);
        }
        else
        {
            appendInstanceNameElement(out, ref);
        }
    }
}

// appendLocalInstancePathElement()
//     <!ELEMENT LOCALINSTANCEPATH (LOCALNAMESPACEPATH, INSTANCENAME)>
void SCMOXmlWriter::appendLocalInstancePathElement(
    Buffer& out,
    const SCMOInstance& instancePath)
{
    out << STRLIT("<LOCALINSTANCEPATH>\n");
    Uint32 nsLength=0;
    const char* ns=instancePath.getNameSpace_l(nsLength);
    appendLocalNameSpacePathElement(out, ns, nsLength);
    appendInstanceNameElement(out, instancePath);
    out << STRLIT("</LOCALINSTANCEPATH>\n");
}

// appendInstancePathElement()
//     <!ELEMENT INSTANCEPATH (NAMESPACEPATH,INSTANCENAME)>
void SCMOXmlWriter::appendInstancePathElement(
    Buffer& out,
    const SCMOInstance& instancePath)
{
    out << STRLIT("<INSTANCEPATH>\n");

    Uint32 hostnameLength=0;
    const char* hostname=instancePath.getHostName_l(hostnameLength);
    Uint32 nsLength=0;
    const char* ns=instancePath.getNameSpace_l(nsLength);
    appendNameSpacePathElement(out,hostname,hostnameLength,ns,nsLength);

    appendInstanceNameElement(out, instancePath);
    out << STRLIT("</INSTANCEPATH>\n");
}

void SCMOXmlWriter::appendValueObjectWithPathElement(
    Buffer& out,
    const Array<SCMOInstance> & objectWithPath,
    const CIMPropertyList& propertyList)
{
    if (propertyList.isNull())
    {
        Array<Uint32> emptyNodes;
        for (Uint32 i = 0, n = objectWithPath.size(); i < n; i++)
        {
            SCMOXmlWriter::appendValueObjectWithPathElement(
                out,
                objectWithPath[i],
                false,
                emptyNodes);
        }
    }
    else
    {
        Array<propertyFilterNodesArray_t> propFilterNodesArrays;
        for (Uint32 i = 0, n = objectWithPath.size(); i < n; i++)
        {
            // This searches for an already created array of nodes,
            // if not found, creates it inside propFilterNodesArrays
            const Array<Uint32> & nodes=
                SCMOXmlWriter::getFilteredNodesArray(
                    propFilterNodesArrays,
                    objectWithPath[i],
                    propertyList);
            SCMOXmlWriter::appendValueObjectWithPathElement(
                out,
                objectWithPath[i],
                true,
                nodes);

        }
    }
}

//EXP_PULL_BEGIN
//------------------------------------------------------------------------------
//
// appendValueInstanceWithPathElement()
//
//     <!ELEMENT VALUE.INSTANCEWITHPATH (INSTANCEPATH,INSTANCE)>
//
//------------------------------------------------------------------------------
// EXP_PULL_TBD checkout the INSTANCEPATH vs NAMEDINSTANCE differences
// Can we create something more common
void SCMOXmlWriter::appendValueInstanceWithPathElement(
    Buffer& out,
    const SCMOInstance& namedInstance,
    bool filtered,
    const Array<Uint32> & nodes)
{
    out << STRLIT("<VALUE.INSTANCEWITHPATH>\n");

    appendInstancePathElement(out, namedInstance);
    appendInstanceElement(out, namedInstance, filtered, nodes);

    out << STRLIT("</VALUE.INSTANCEWITHPATH>\n");
}
//EXP_PULL_END

// appendValueObjectWithPathElement()
//     <!ELEMENT VALUE.OBJECTWITHPATH
//         ((CLASSPATH,CLASS)|(INSTANCEPATH,INSTANCE))>
void SCMOXmlWriter::appendValueObjectWithPathElement(
    Buffer& out,
    const SCMOInstance& objectWithPath,
    bool filtered,
    const Array<Uint32> & nodes)
{
    out << STRLIT("<VALUE.OBJECTWITHPATH>\n");

    appendClassOrInstancePathElement(out, objectWithPath);
    appendObjectElement(out, objectWithPath,filtered,nodes);

    out << STRLIT("</VALUE.OBJECTWITHPATH>\n");
}

// appendObjectElement()
// May refer to a CLASS or an INSTANCE
void SCMOXmlWriter::appendObjectElement(
    Buffer& out,
    const SCMOInstance& object,
    bool filtered,
    const Array<Uint32> & nodes)
{
    if (object.inst.hdr->flags.isClassOnly)
    {
        appendClassElement(out, object);
    }
    else
    {
        appendInstanceElement(out, object,filtered,nodes);
    }
}

//------------------------------------------------------------------------------
//
// appendClassElement()
//
//     <!ELEMENT CLASS
//         (QUALIFIER*,(PROPERTY|PROPERTY.ARRAY|PROPERTY.REFERENCE)*,METHOD*)>
//     <!ATTLIST CLASS
//         %CIMName;
//         %SuperClass;>
//
//------------------------------------------------------------------------------

void SCMOXmlWriter::appendClassElement(
    Buffer& out,
    const SCMOInstance& cimClass)
{

    SCMBClass_Main* ptrClass = cimClass.inst.hdr->theClass.ptr->cls.hdr;
    const char* clsBase = cimClass.inst.hdr->theClass.ptr->cls.base;

    // Class opening element:
    out << STRLIT("<CLASS NAME=\"");
    out.append(
        &(clsBase[ptrClass->className.start]),
        (ptrClass->className.size-1));

    out.append('"',' ');
    if (0 != ptrClass->superClassName.start)
    {
        out << STRLIT(" SUPERCLASS=\"");
        out.append(
            &(clsBase[ptrClass->superClassName.start]),
            (ptrClass->superClassName.size-1));
        out.append('"',' ');
    }
    out.append('>','\n');

    // Append class qualifiers
    SCMBQualifier *theArray =
        (SCMBQualifier*)&(clsBase[ptrClass->qualifierArray.start]);
    for (Uint32 i=0, n=ptrClass->numberOfQualifiers;i<n;i++)
    {
        SCMOXmlWriter::appendQualifierElement(out,theArray[i],clsBase);
    }

    // Append Property definitions:
    for (Uint32 i=0,k=cimClass.getPropertyCount();i<k;i++)
    {
            SCMOXmlWriter::appendPropertyElement(out,cimClass,i);
    }

    // ATTN: No method definitions with SCMO today, so do nothing with them
    //       Actually this code does not serve a purpose, but is kept here
    //       for completeness.

    // Class closing element:
    out << STRLIT("</CLASS>\n");
}

// appendLocalClassPathElement()
//     <!ELEMENT LOCALCLASSPATH (LOCALNAMESPACEPATH, CLASSNAME)>
void SCMOXmlWriter::appendLocalClassPathElement(
    Buffer& out,
    const SCMOInstance& classPath)
{
    out << STRLIT("<LOCALCLASSPATH>\n");
    Uint32 hostnameLength=0;
    const char* hostname=classPath.getHostName_l(hostnameLength);
    Uint32 nsLength=0;
    const char* ns=classPath.getNameSpace_l(nsLength);

    appendNameSpacePathElement(out,hostname,hostnameLength,ns,nsLength);

    Uint32 classNameLength=0;
    const char* className = classPath.getClassName_l(classNameLength);
    appendClassNameElement(out, className, classNameLength);
    out << STRLIT("</LOCALCLASSPATH>\n");
}

// appendClassPathElement()
//     <!ELEMENT CLASSPATH (NAMESPACEPATH,CLASSNAME)>
void SCMOXmlWriter::appendClassPathElement(
    Buffer& out,
    const SCMOInstance& classPath)
{
    out << STRLIT("<CLASSPATH>\n");

    Uint32 hostnameLength=0;
    const char* hostname=classPath.getHostName_l(hostnameLength);
    Uint32 nsLength=0;
    const char* ns=classPath.getNameSpace_l(nsLength);

    appendNameSpacePathElement(out,hostname,hostnameLength,ns,nsLength);

    Uint32 classNameLength=0;
    const char* className = classPath.getClassName_l(classNameLength);

    appendClassNameElement(out, className, classNameLength);
    out << STRLIT("</CLASSPATH>\n");
}

void SCMOXmlWriter::appendSCMBUnion(
    Buffer& out,
    const SCMBUnion & u,
    const CIMType & valueType,
    const char * base)
{
    switch (valueType)
    {
        case CIMTYPE_BOOLEAN:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.bin);
            }
            break;
        }

        case CIMTYPE_UINT8:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.u8);
            }
            break;
        }

        case CIMTYPE_SINT8:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.s8);
            }
            break;
        }

        case CIMTYPE_UINT16:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.u16);
            }
            break;
        }

        case CIMTYPE_SINT16:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.s16);
            }
            break;
        }

        case CIMTYPE_UINT32:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.u32);
            }
            break;
        }

        case CIMTYPE_SINT32:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.s32);
            }
            break;
        }

        case CIMTYPE_UINT64:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.u64);
            }
            break;
        }

        case CIMTYPE_SINT64:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.s64);
            }
            break;
        }

        case CIMTYPE_REAL32:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.r32);
            }
            break;
        }

        case CIMTYPE_REAL64:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::append(out, u.simple.val.r64);
            }
            break;
        }

        case CIMTYPE_CHAR16:
        {
            if (u.simple.hasValue)
            {
                SCMOXmlWriter::appendSpecial(
                    out,
                    Char16(u.simple.val.c16));
            }
            break;
        }

        case CIMTYPE_STRING:
        {
            if (u.stringValue.start)
            {
                SCMOXmlWriter::appendSpecial(
                    out,
                    &(base[u.stringValue.start]),
                    (u.stringValue.size-1));
            }
            break;
        }

        case CIMTYPE_DATETIME:
        {
            // an SCMBDateTime is a CIMDateTimeRep
            // this should help us to reuse existing optimized Datetime
            char buffer[26];
            _DateTimetoCStr(u.dateTimeValue, buffer);
            // datetime value is formatted with a \0 at end, ignore
            out.append(buffer,sizeof(buffer)-1);
            break;
        }
        // Object and Instance are both written the same way, namely as
        // object element which then is encoded using appendSpecial
        case CIMTYPE_OBJECT:
        case CIMTYPE_INSTANCE:
        {
            Buffer toEncodeObject(4000);
            SCMOInstance * obj = u.extRefPtr;
            if (obj)
            {
                appendObjectElement(toEncodeObject, *obj);
                SCMOXmlWriter::appendSpecial(
                    out,
                    toEncodeObject.getData(),
                    toEncodeObject.size());
            }
            break;
        }
        default:
            // CIMTYPE_REFERENCE has been handled upfront, do nothing here
            break;
    }
}

void SCMOXmlWriter::appendSCMBUnionArray(
    Buffer& out,
    const SCMBUnion & u,
    const CIMType & valueType,
    Uint32 numElements,
    const char * base)
{
    SCMBUnion* arr = (SCMBUnion*) &(base[u.arrayValue.start]);
    switch (valueType)
    {
        case CIMTYPE_BOOLEAN:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.bin);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }
        case CIMTYPE_UINT8:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.u8);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_SINT8:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.s8);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_UINT16:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.u16);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_SINT16:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.s16);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_UINT32:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.u32);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_SINT32:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.s32);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_UINT64:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.u64);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_SINT64:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.s64);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_REAL32:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.r32);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_REAL64:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::append(out, arr->simple.val.r64);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_CHAR16:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                SCMOXmlWriter::appendSpecial(out, Char16(arr->simple.val.c16));
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }

        case CIMTYPE_STRING:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                if (0!=arr->stringValue.start)
                {
                    SCMOXmlWriter::appendSpecial(
                        out,
                        &(base[arr->stringValue.start]),
                        (arr->stringValue.size-1));
                }
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");

            break;
        }

        case CIMTYPE_DATETIME:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            char buffer[26];
            while (numElements--)
            {
                out << STRLIT("<VALUE>");
                // an SCMBDateTime is a CIMDateTimeRep
                // this should help us to reuse existing optimized Datetime
                _DateTimetoCStr(arr->dateTimeValue, buffer);
                // datetime value is formatted with a \0 at end, ignore
                out.append(buffer,sizeof(buffer)-1);
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }
        case CIMTYPE_REFERENCE:
        {
            out << STRLIT("<VALUE.REFARRAY>\n");
            while (numElements--)
            {
                SCMOInstance * ref = arr->extRefPtr;
                if (ref)
                {
                    appendValueReferenceElement(out, *ref);
                }
                arr++;
            }
            out << STRLIT("</VALUE.REFARRAY>\n");
            break;
        }
        case CIMTYPE_OBJECT:
        case CIMTYPE_INSTANCE:
        {
            out << STRLIT("<VALUE.ARRAY>\n");
            Buffer toEncodeObject(4000);
            while (numElements--)
            {
                toEncodeObject.clear();
                out << STRLIT("<VALUE>");
                SCMOInstance * obj = arr->extRefPtr;
                if (obj)
                {
                    appendObjectElement(toEncodeObject, *obj);
                    SCMOXmlWriter::appendSpecial(
                        out,
                        toEncodeObject.getData(),
                        toEncodeObject.size());
                }
                arr++;
                out << STRLIT("</VALUE>\n");
            }
            out << STRLIT("</VALUE.ARRAY>\n");
            break;
        }
        default:
            PEGASUS_DEBUG_ASSERT(false);
    }

}



PEGASUS_NAMESPACE_END
