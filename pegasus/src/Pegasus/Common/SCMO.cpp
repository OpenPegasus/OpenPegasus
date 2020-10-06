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

#include <Pegasus/Common/SCMO.h>
#include <Pegasus/Common/SCMOClass.h>
#include <Pegasus/Common/SCMOInstance.h>
#include <Pegasus/Common/SCMODump.h>
#include <Pegasus/Common/SCMOClassCache.h>
#include <Pegasus/Common/CharSet.h>
#include <Pegasus/Common/CIMDateTimeRep.h>
#include <Pegasus/Common/CIMPropertyRep.h>
#include <Pegasus/Common/CIMInstanceRep.h>
#include <Pegasus/Common/CIMObjectPathRep.h>
#include <Pegasus/Common/CIMNameCast.h>
#include <Pegasus/Common/CommonUTF.h>
#include <Pegasus/Common/StrLit.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/XmlWriter.h>
#include <Pegasus/Common/System.h>
#include <Pegasus/Common/FileSystem.h>
#include <Pegasus/Common/StringConversion.h>
#include <Pegasus/Common/ArrayIterator.h>
#include <Pegasus/Common/PegasusAssert.h>
#include <Pegasus/Common/CIMValueRep.h>

# if defined PEGASUS_OS_ZOS
#  include <strings.h>
# else
#  include <string.h>
# endif

#ifdef PEGASUS_OS_ZOS
  #include <Pegasus/General/SetFileDescriptorToEBCDICEncoding.h>
#endif

#ifdef PEGASUS_HAS_ICU
# include <unicode/platform.h>
# include <unicode/urename.h>
# include <unicode/ures.h>
# include <unicode/ustring.h>
# include <unicode/uchar.h>
# include <unicode/ucnv.h>
#endif

PEGASUS_USING_STD;

#define SCMB_INITIAL_MEMORY_CHUNK_SIZE 4096

/**
 * This macro is used at the SCMODump class
 * for generating C/C++ runtime independend output.
 * For example on Linux if fprintf got a NULL pointer
 * for a string format specification, the string "(null)" is
 * substituted. On other platforms no string "" is substituded.
 */
#define NULLSTR(x) ((x) == 0 ? "" : (x))

PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T SCMOInstance
# include "ArrayImpl.h"
#undef PEGASUS_ARRAY_T

// KS_FUTURE bug 9930 this is not really complete list of all qualifiers with
// latest  CIM_Schemas. At least struct and reference should be in this list.
const StrLit SCMOClass::_qualifierNameStrLit[72] =
{
    STRLIT(""),
    STRLIT("ABSTRACT"),
    STRLIT("AGGREGATE"),
    STRLIT("AGGREGATION"),
    STRLIT("ALIAS"),
    STRLIT("ARRAYTYPE"),
    STRLIT("ASSOCIATION"),
    STRLIT("BITMAP"),
    STRLIT("BITVALUES"),
    STRLIT("CLASSCONSTRAINT"),
    STRLIT("COMPOSITION"),
    STRLIT("CORRELATABLE"),
    STRLIT("COUNTER"),
    STRLIT("DELETE"),
    STRLIT("DEPRECATED"),
    STRLIT("DESCRIPTION"),
    STRLIT("DISPLAYDESCRIPTION"),
    STRLIT("DISPLAYNAME"),
    STRLIT("DN"),
    STRLIT("EMBEDDEDINSTANCE"),
    STRLIT("EMBEDDEDOBJECT"),
    STRLIT("EXCEPTION"),
    STRLIT("EXPENSIVE"),
    STRLIT("EXPERIMENTAL"),
    STRLIT("GAUGE"),
    STRLIT("IFDELETED"),
    STRLIT("IN"),
    STRLIT("INDICATION"),
    STRLIT("INVISIBLE"),
    STRLIT("ISPUNIT"),
    STRLIT("KEY"),
    STRLIT("LARGE"),
    STRLIT("MAPPINGSTRINGS"),
    STRLIT("MAX"),
    STRLIT("MAXLEN"),
    STRLIT("MAXVALUE"),
    STRLIT("METHODCONSTRAINT"),
    STRLIT("MIN"),
    STRLIT("MINLEN"),
    STRLIT("MINVALUE"),
    STRLIT("MODELCORRESPONDENCE"),
    STRLIT("NONLOCAL"),
    STRLIT("NONLOCALTYPE"),
    STRLIT("NULLVALUE"),
    STRLIT("OCTETSTRING"),
    STRLIT("OUT"),
    STRLIT("OVERRIDE"),
    STRLIT("PROPAGATED"),
    STRLIT("PROPERTYCONSTRAINT"),
    STRLIT("PROPERTYUSAGE"),
    STRLIT("PROVIDER"),
    STRLIT("PUNIT"),
    STRLIT("READ"),
    STRLIT("REQUIRED"),
    STRLIT("REVISION"),
    STRLIT("SCHEMA"),
    STRLIT("SOURCE"),
    STRLIT("SOURCETYPE"),
    STRLIT("STATIC"),
    STRLIT("SYNTAX"),
    STRLIT("SYNTAXTYPE"),
    STRLIT("TERMINAL"),
    STRLIT("TRIGGERTYPE"),
    STRLIT("UMLPACKAGEPATH"),
    STRLIT("UNITS"),
    STRLIT("UNKNOWNVALUES"),
    STRLIT("UNSUPPORTEDVALUES"),
    STRLIT("VALUEMAP"),
    STRLIT("VALUES"),
    STRLIT("VERSION"),
    STRLIT("WEAK"),
    STRLIT("WRITE")
};

#define _NUM_QUALIFIER_NAMES \
           (sizeof(_qualifierNameStrLit)/sizeof(_qualifierNameStrLit[0]))


/*****************************************************************************
 * The static declaration of the common SCMO memory functions.
 *****************************************************************************/

static Uint64 _getFreeSpace(
    SCMBDataPtr& ptr,
    Uint32 size,
    SCMBMgmt_Header** pmem);

static void _setString(
    const String& theString,
    SCMBDataPtr& ptr,
    SCMBMgmt_Header** pmem);

static void _setBinary(
    const void* theBuffer,
    Uint32 bufferSize,
    SCMBDataPtr& ptr,
    SCMBMgmt_Header** pmem);



/*****************************************************************************
 * Internal inline functions.
 *****************************************************************************/

inline String _newCimString(const SCMBDataPtr & ptr, const char * base)
{
    if (ptr.size > 0)
    {
        return String(&(base[ptr.start]),ptr.size-1);
    }
    else
    {
        return String();
    }
}

inline void _deleteArrayExtReference(
    SCMBDataPtr& theArray,
    SCMBMgmt_Header** pmem )
{
    SCMBUnion* ptr;
    // if the array was already set,
    // the previous references has to be deleted
    if(theArray.size != 0)
    {
        Uint32 oldArraySize=(theArray.size/sizeof(SCMBUnion));

        ptr = (SCMBUnion*)&(((char*)*pmem)[theArray.start]);
        for (Uint32 i = 0 ; i < oldArraySize ; i++)
        {
            delete ptr[i].extRefPtr;
            ptr[i].extRefPtr = 0;
        }
    }
}

static void _deleteExternalReferenceInternal(
    SCMBMgmt_Header* memHdr, SCMOInstance *extRefPtr)
{
    Uint32 nuExtRef = memHdr->numberExtRef;
    char * base = ((char*)memHdr);
    Uint64* array =
        (Uint64*)&(base[memHdr->extRefIndexArray.start]);
    Uint32 extRefIndex = PEG_NOT_FOUND;

    for (Uint32 i = 0; i < nuExtRef; i++)
    {
         if (((SCMBUnion*)(&(base[array[i]])))->extRefPtr == extRefPtr)
         {
             extRefIndex = i;
             break;
         }
    }
    PEGASUS_ASSERT (extRefIndex != PEG_NOT_FOUND);

   // Shrink extRefIndexArray

    for (Uint32 i = extRefIndex + 1; i < nuExtRef; i++)
    {
        array[i-1] = array[i];
    }

    array[nuExtRef-1] = 0;
    memHdr->numberExtRef--;

    delete extRefPtr;
}

/*****************************************************************************
 * The SCMOClass methods
 *****************************************************************************/
SCMOClass::SCMOClass()
{
    _initSCMOClass();

    _setBinary("",1,cls.hdr->className,&cls.mem );
    _setBinary("",1,cls.hdr->nameSpace,&cls.mem );
    cls.hdr->flags.isEmpty=true;
}

inline void SCMOClass::_initSCMOClass()
{
    PEGASUS_ASSERT(SCMB_INITIAL_MEMORY_CHUNK_SIZE
        - sizeof(SCMBClass_Main)>0);

    cls.base = (char*)malloc(SCMB_INITIAL_MEMORY_CHUNK_SIZE);
    if (cls.base == 0)
    {
        // Not enough memory!
        throw PEGASUS_STD(bad_alloc)();
    }

    memset(cls.base,0,sizeof(SCMBClass_Main));

    // initalize eye catcher
    cls.hdr->header.magic=PEGASUS_SCMB_CLASS_MAGIC;
    cls.hdr->header.totalSize=SCMB_INITIAL_MEMORY_CHUNK_SIZE;
    // The # of bytes free
    cls.hdr->header.freeBytes=
        SCMB_INITIAL_MEMORY_CHUNK_SIZE-sizeof(SCMBClass_Main);

    // Index to the start of the free space in this instance
    cls.hdr->header.startOfFreeSpace=sizeof(SCMBClass_Main);

    cls.hdr->refCount=1;
}

SCMOClass::SCMOClass(const char* className, const char* nameSpaceName )
{
    Uint32 clsNameLen = strlen(className);
    Uint32 nsNameLen = strlen(nameSpaceName);

    if (0 == className )
    {
        String message("SCMOClass: Class name not set (null pointer)!");
        throw CIMException(CIM_ERR_FAILED,message );
    }

    if (0 == nameSpaceName)
    {
        String message("SCMOClass: Name Space not set (null pointer)!");
        throw CIMException(CIM_ERR_FAILED,message );
    }

    _initSCMOClass();

    _setBinary(className,clsNameLen+1,cls.hdr->className,&cls.mem );

    _setBinary(nameSpaceName,nsNameLen+1,cls.hdr->nameSpace,&cls.mem );

    cls.hdr->flags.isEmpty=true;

}

SCMOClass::SCMOClass(
    const CIMClass& theCIMClass,
    const char* nameSpaceName)
{
    _initSCMOClass();

    try
    {
        _setString(theCIMClass.getSuperClassName().getString(),
                   cls.hdr->superClassName,
                   &cls.mem );
    }
    catch (UninitializedObjectException&)
    {
        // there is no Super ClassName
        cls.hdr->superClassName.start=0;
        cls.hdr->superClassName.size=0;
    }

    CIMObjectPath theObjectPath=theCIMClass.getPath();

    //set name space
    if (nameSpaceName)
    {
        _setBinary(nameSpaceName,
                   strlen(nameSpaceName)+1,
                   cls.hdr->nameSpace,
                   &cls.mem );
    }
    else
    {
        _setString(theObjectPath.getNameSpace().getString(),
                  cls.hdr->nameSpace,
                  &cls.mem );
    }

    //set class name
    _setString(theObjectPath.getClassName().getString(),
               cls.hdr->className,
               &cls.mem );

    //set class Qualifiers
    _setClassQualifers(theCIMClass._rep->_qualifiers);

    //set properties
    _setClassProperties(theCIMClass._rep->_properties);

}

void SCMOClass::_destroyExternalReferences()
{
    _destroyExternalReferencesInternal(cls.mem);
}

const char* SCMOClass::getSuperClassName() const
{
    return _getCharString(cls.hdr->superClassName,cls.base);
}

const char* SCMOClass::getSuperClassName_l(Uint32 & length) const
{
    length = cls.hdr->superClassName.size;
    if (0 == length)
    {
        return 0;
    }
    else
    {
        length--;
    }
    return _getCharString(cls.hdr->superClassName,cls.base);
}

void  SCMOClass::getCIMClass(CIMClass& cimClass) const
{
    CIMClass newCimClass(
        CIMNameCast(_newCimString(cls.hdr->className,cls.base)),
        CIMNameCast(_newCimString(cls.hdr->superClassName,cls.base)));

    // set the name space
    newCimClass._rep->_reference._rep->_nameSpace=
        CIMNamespaceNameCast(_newCimString(cls.hdr->nameSpace,cls.base));

    // Add class qualifier if exist
    if (0 != cls.hdr->numberOfQualifiers)
    {
        SCMBQualifier* qualiArray =
            (SCMBQualifier*)&(cls.base[cls.hdr->qualifierArray.start]);

        CIMQualifier theCimQualifier;

        Uint32 i, k = cls.hdr->numberOfQualifiers;
        for ( i = 0 ; i < k ; i++)
        {
            _getCIMQualifierFromSCMBQualifier(
                theCimQualifier,
                qualiArray[i],
                cls.base);

            newCimClass._rep->_qualifiers.addUnchecked(theCimQualifier);
        }
    }

    // If properties are in that class
    if (0 != cls.hdr->propertySet.number)
    {
        Uint32 i, k = cls.hdr->propertySet.number;
        for ( i = 0 ; i < k ; i++)
        {
           newCimClass._rep->_properties.append(
               _getCIMPropertyAtNodeIndex(i));
        }
    }

    cimClass = newCimClass;
}

CIMProperty SCMOClass::_getCIMPropertyAtNodeIndex(Uint32 nodeIdx) const
{
    CIMValue theCimValue;
    CIMProperty retCimProperty;

    SCMBClassPropertyNode& clsProp =
        ((SCMBClassPropertyNode*)
         &(cls.base[cls.hdr->propertySet.nodeArray.start]))[nodeIdx];

    // get the default value
    SCMOInstance::_getCIMValueFromSCMBValue(
        theCimValue,
        clsProp.theProperty.defaultValue,
        cls.base);

    // have to check if there is the origin class name set.
    // An empty origin class name is differnt then a NULL class name
    if (0 != clsProp.theProperty.originClassName.start)
    {
        retCimProperty = CIMProperty(
            CIMNameCast(_newCimString(clsProp.theProperty.name,cls.base)),
            theCimValue,
            theCimValue.getArraySize(),
            CIMNameCast(
                _newCimString(clsProp.theProperty.refClassName,cls.base)),
            CIMNameCast(
                _newCimString(clsProp.theProperty.originClassName,cls.base)),
            clsProp.theProperty.flags.propagated);
    }
    else
    {
         retCimProperty = CIMProperty(
            CIMNameCast(_newCimString(clsProp.theProperty.name,cls.base)),
            theCimValue,
            theCimValue.getArraySize(),
            CIMNameCast(
                _newCimString(clsProp.theProperty.refClassName,cls.base)),
            CIMName(),
            clsProp.theProperty.flags.propagated);
    }

    SCMBQualifier* qualiArray =
        (SCMBQualifier*)
             &(cls.base[clsProp.theProperty.qualifierArray.start]);

    CIMQualifier theCimQualifier;
    Uint32 i, k = clsProp.theProperty.numberOfQualifiers;
    for ( i = 0 ; i < k ; i++)
    {
       _getCIMQualifierFromSCMBQualifier(
            theCimQualifier,
            qualiArray[i],
            cls.base);

        retCimProperty._rep->_qualifiers.addUnchecked(theCimQualifier);
    }

    return retCimProperty;

}

void SCMOClass::_getCIMQualifierFromSCMBQualifier(
    CIMQualifier& theCimQualifier,
    const SCMBQualifier& scmbQualifier,
    const char* base)
{
    CIMName theCimQualiName;
    CIMValue theCimValue;

    SCMOInstance::_getCIMValueFromSCMBValue(
        theCimValue,
        scmbQualifier.value,
        base);

    if (scmbQualifier.name == QUALNAME_USERDEFINED)
    {
        theCimQualiName = _newCimString(scmbQualifier.userDefName,base);
    }
    else
    {
        theCimQualiName = String(
            SCMOClass::qualifierNameStrLit(scmbQualifier.name).str,
            SCMOClass::qualifierNameStrLit(scmbQualifier.name).size);
    }

    theCimQualifier = CIMQualifier(
        theCimQualiName,
        theCimValue,
        scmbQualifier.flavor,
        scmbQualifier.propagated);
}

void SCMOClass::getKeyNamesAsString(Array<String>& keyNames) const
{
    SCMBKeyBindingNode* nodeArray =
    (SCMBKeyBindingNode*)&(cls.base[cls.hdr->keyBindingSet.nodeArray.start]);

    keyNames.clear();

    for (Uint32 i = 0, k = cls.hdr->keyBindingSet.number; i < k; i++)
    {
        // Append the key property name.
        keyNames.append(_newCimString(nodeArray[i].name,cls.base));
    }
}

const char* SCMOClass::_getPropertyNameAtNode(Uint32 propNode) const
{
    SCMBClassPropertyNode* nodeArray =
        (SCMBClassPropertyNode*)
            &(cls.base[cls.hdr->propertySet.nodeArray.start]);

    return(_getCharString(nodeArray[propNode].theProperty.name,cls.base));
}

SCMO_RC SCMOClass::_getKeyBindingNodeIndex(Uint32& node, const char* name) const
{
    Uint32 tag,len,hashIdx;

    len = strlen(name);
    tag = _generateStringTag((const char*)name, len);
    // get the node index of the hash table
    hashIdx =
      cls.hdr->keyBindingSet.hashTable[tag&(PEGASUS_KEYBINDIG_SCMB_HASHSIZE-1)];
    // there is no entry in the hash table on this hash table index.
    if (hashIdx == 0)
    {
        // property name not found
        return SCMO_NOT_FOUND;
    }

    // get the Key Binding node array
    SCMBKeyBindingNode* nodeArray =
        (SCMBKeyBindingNode*)
            &(cls.base[cls.hdr->keyBindingSet.nodeArray.start]);

    // set property node array index.
    // The hash table index to the node arra in one larger!
    node = hashIdx - 1;

    do
    {
        // do they have the same hash tag ?
        if (nodeArray[node].nameHashTag == tag)
        {
            // Now it is worth to compare the two names
            if (_equalNoCaseUTF8Strings(nodeArray[node].name,cls.base,name,len))
            {
                // we found the property !
                return SCMO_OK;
            }
        }
        // Are we at the end of the chain ?
        if (!nodeArray[node].hasNext)
        {
            // property name not found
            return SCMO_NOT_FOUND;
        }

        // get next node index.
        node = nodeArray[node].nextNode;

    } while ( true );

    // this should never be reached
    return SCMO_NOT_FOUND;

}
/*
    Gets the property Node for class-defined properties. Returns the
    node and SCMO_OK or the error SCMO_NOT_FOUND
    This function does NOT handle user-defined properties.
*/
SCMO_RC SCMOClass::_getPropertyNodeIndex(Uint32& node, const char* name) const
{
    Uint32 tag,len,hashIdx;

    len = strlen(name);
    tag = _generateStringTag((const char*)name, len);
    // get the node index of the hash table
    hashIdx =
      cls.hdr->propertySet.hashTable[tag&(PEGASUS_PROPERTY_SCMB_HASHSIZE -1)];
    // there is no entry in the hash table on this hash table index.
    if (hashIdx == 0)
    {
        // property name not found
        return SCMO_NOT_FOUND;
    }

    // get the property node array
    SCMBClassPropertyNode* nodeArray =
        (SCMBClassPropertyNode*)
            &(cls.base[cls.hdr->propertySet.nodeArray.start]);

    // set property node array index.
    // The hash table index to the node arra in one larger!
    node = hashIdx - 1;

    do
    {
        // do they have the same hash tag ?
        if (nodeArray[node].theProperty.nameHashTag == tag)
        {
            // Now it is worth to compare the two names
            if (_equalNoCaseUTF8Strings(
                nodeArray[node].theProperty.name,cls.base,name,len))
            {
                // we found the property !
                return SCMO_OK;
            }
        }
        // Are we at the end of the chain ?
        if (!nodeArray[node].hasNext)
        {
            // property name not found
            return SCMO_NOT_FOUND;
        }

        // get next node index.
        node = nodeArray[node].nextNode;

    } while ( true );

    // this should never be reached
    PEGASUS_UNREACHABLE(return SCMO_NOT_FOUND;)
}

void SCMOClass::_setClassProperties(PropertySet& theCIMProperties)
{
    Uint32 noProps = theCIMProperties.size();
    Uint64 start, startKeyIndexList;
    Uint32 noKeys = 0;
    Boolean isKey = false;

    Array<Uint32> keyIndex(noProps);

    cls.hdr->propertySet.number=noProps;

    // allocate the keyIndexList
    startKeyIndexList = _getFreeSpace(
        cls.hdr->keyIndexList,
        noProps*sizeof(Uint32),
        &cls.mem);

    if(noProps != 0)
    {
        // Allocate key property mask.
        // Each property needs a bit in the mask.
        // The number of Uint64 in the key mask is :
        // Decrease the number of properties by 1 since
        // the array starts at 0!
        // Divide by the number of bits in a Uint64.
        // e.g. number of Properties = 68
        // (68 - 1) / 64 = 1 --> The mask consists of two Uint64 values.
        _getFreeSpace(cls.hdr->keyPropertyMask,
              sizeof(Uint64)*(((noProps-1)/64)+1),
              &cls.mem);

        // allocate property array and save the start index of the array.
        start = _getFreeSpace(cls.hdr->propertySet.nodeArray,
                      sizeof(SCMBClassPropertyNode)*noProps,
                      &cls.mem);

        // clear the hash table
        memset(cls.hdr->propertySet.hashTable,
               0,
               PEGASUS_PROPERTY_SCMB_HASHSIZE*sizeof(Uint32));


        for (Uint32 i = 0; i < noProps; i++)
        {
            _setProperty(start,&isKey ,theCIMProperties[i]);
            if(isKey)
            {
                // if the property is a key
                // save the key index
                keyIndex[noKeys]=i;
                noKeys++;
                _setPropertyAsKeyInMask(i);
            }
            // Adjust ordered set management structures.
            _insertPropertyIntoOrderedSet(start,i);

            start = start + sizeof(SCMBClassPropertyNode);
        }

        cls.hdr->keyBindingSet.number = noKeys;

        if (noKeys != 0)
        {
            // allocate key binding array and save the start index of the array.
            start = _getFreeSpace(cls.hdr->keyBindingSet.nodeArray,
                          sizeof(SCMBKeyBindingNode)*noKeys,
                          &cls.mem);

            // clear the hash table
            memset(cls.hdr->keyBindingSet.hashTable,
                   0,
                   PEGASUS_KEYBINDIG_SCMB_HASHSIZE*sizeof(Uint32));

            // fill the key index list
            memcpy(
                &(cls.base[startKeyIndexList]),
                keyIndex.getData(),
                noKeys*sizeof(Uint32));

            for (Uint32 i = 0 ; i < noKeys; i++)
            {

                _setClassKeyBinding(start,theCIMProperties[keyIndex[i]]);
                // Adjust ordered set management structures.
                _insertKeyBindingIntoOrderedSet(start,i);

                start = start + sizeof(SCMBKeyBindingNode);
            }

        }
        else
        {
            cls.hdr->keyBindingSet.nodeArray.start=0;
            cls.hdr->keyBindingSet.nodeArray.size=0;
        }
    }
    else
    {
        cls.hdr->propertySet.nodeArray.start=0;
        cls.hdr->propertySet.nodeArray.size=0;
        cls.hdr->keyPropertyMask.start=0;
        cls.hdr->keyPropertyMask.size=0;
        cls.hdr->keyBindingSet.nodeArray.start=0;
        cls.hdr->keyBindingSet.nodeArray.size=0;
    }
}

void SCMOClass::_insertKeyBindingIntoOrderedSet(Uint64 start, Uint32 newIndex)
{

    SCMBKeyBindingNode* newKeyNode =
        (SCMBKeyBindingNode*)&(cls.base[start]);

    SCMBKeyBindingNode* scmoKeyNodeArray =
        (SCMBKeyBindingNode*)
            &(cls.base[cls.hdr->keyBindingSet.nodeArray.start]);

    Uint32 *hashTable = cls.hdr->keyBindingSet.hashTable;

    // calculate the new hash index of the new property.
    Uint32 hash = newKeyNode->nameHashTag &
        (PEGASUS_KEYBINDIG_SCMB_HASHSIZE - 1);

    // 0 is an invalid index in the hash table
    if (hashTable[hash] == 0)
    {
        hashTable[hash] = newIndex + 1;
        return;
    }

    // get the first node of the hash chain.
    Uint32 nodeIndex = hashTable[hash]-1;

    do
    {
        // is this the same note ?
        if (nodeIndex == newIndex)
        {
            // The node is already in the ordered set
            return;
        }

        // Are we at the end of the chain ?
        if (!scmoKeyNodeArray[nodeIndex].hasNext)
        {
            // link the new element at the end of the chain
            scmoKeyNodeArray[nodeIndex].nextNode = newIndex;
            scmoKeyNodeArray[nodeIndex].hasNext = true;
            return;
        }

        nodeIndex = scmoKeyNodeArray[nodeIndex].nextNode;

    } while ( true );

    // this should never be reached
}

void SCMOClass::_insertPropertyIntoOrderedSet(Uint64 start, Uint32 newIndex)
{
    SCMBClassPropertyNode* newPropNode =
        (SCMBClassPropertyNode*)&(cls.base[start]);

    SCMBClassPropertyNode* scmoPropNodeArray =
        (SCMBClassPropertyNode*)
             &(cls.base[cls.hdr->propertySet.nodeArray.start]);

    Uint32 *hashTable = cls.hdr->propertySet.hashTable;

    // calcuate the new hash index of the new property.
    Uint32 hash = newPropNode->theProperty.nameHashTag &
        (PEGASUS_PROPERTY_SCMB_HASHSIZE -1);

    // 0 is an invalid index in the hash table
    if (hashTable[hash] == 0)
    {
        hashTable[hash] = newIndex + 1;
        return;
    }

    // get the first node of the hash chain.
    Uint32 nodeIndex = hashTable[hash]-1;

    do
    {
        // is this the same note ?
        if (nodeIndex == newIndex)
        {
            // The node is already in the ordered set
            return;
        }

        // Are we at the end of the chain ?
        if (!scmoPropNodeArray[nodeIndex].hasNext)
        {
            // link the new element at the end of the chain
            scmoPropNodeArray[nodeIndex].nextNode = newIndex;
            scmoPropNodeArray[nodeIndex].hasNext = true;
            return;
        }

        // get next node index.
        nodeIndex = scmoPropNodeArray[nodeIndex].nextNode;

    } while ( true );

    // this should never be reached
}

void SCMOClass::_setClassKeyBinding(
    Uint64 start,
    const CIMProperty& theCIMProperty)
{
    CIMPropertyRep* propRep = theCIMProperty._rep;

    // First do all _setString(). Can cause reallocation.
    _setString(propRep->_name.getString(),
        ((SCMBKeyBindingNode*)&(cls.base[start]))->name,
        &cls.mem);

    SCMBKeyBindingNode* scmoKeyBindNode =
        (SCMBKeyBindingNode*)&(cls.base[start]);

    // calculate the new hash tag
    scmoKeyBindNode->nameHashTag =
        _generateSCMOStringTag(scmoKeyBindNode->name,cls.base);
    scmoKeyBindNode->type = propRep->_value.getType();
    scmoKeyBindNode->hasNext=false;
    scmoKeyBindNode->nextNode=0;

}

void SCMOClass::_setPropertyAsKeyInMask(Uint32 i)
{
    Uint64 *keyMask;

    // In which Uint64 of key mask is the bit for property i ?
    // Divide with the number of bits in a Uint64.
    // 47 / 64 = 0 --> The key bit for property i is in in keyMask[0].
    Uint32 idx = i/64 ;

    // Create a filter to set the bit.
    // Modulo division with 64. Shift left a bit by the remainder.
    Uint64 filter = ( (Uint64)1 << (i & 63));

    // Calculate the real pointer to the Uint64 array
    keyMask = (Uint64*)&(cls.base[cls.hdr->keyPropertyMask.start]);

    keyMask[idx] = keyMask[idx] | filter ;
}

Boolean SCMOClass::_isPropertyKey(Uint32 i)
{
    Uint64 *keyMask;

    // In which Uint64 of key mask is the bit for property i ?
    // Divide with the number of bits in a Uint64.
    // e.g. number of Properties = 68
    // 47 / 64 = 0 --> The key bit for property i is in in keyMask[0].
    Uint32 idx = i/64 ;

    // Create a filter to check if the bit is set:
    // Modulo division with 64. Shift left a bit by the remainder.
    Uint64 filter = ( (Uint64)1 << (i & 63));

    // Calculate the real pointer to the Uint64 array
    keyMask = (Uint64*)&(cls.base[cls.hdr->keyPropertyMask.start]);

    return keyMask[idx] & filter ;

}


void SCMOClass::_setProperty(
    Uint64 start,
    Boolean* isKey,
    const CIMProperty& theCIMProperty)
{
   *isKey = false;

    CIMPropertyRep* propRep = theCIMProperty._rep;

    // First do all _setString(). Can cause reallocation.

    _setString(propRep->_name.getString(),
        ((SCMBClassPropertyNode*)&(cls.base[start]))->theProperty.name,
        &cls.mem);

    _setString(propRep->_classOrigin.getString(),
        ((SCMBClassPropertyNode*)
             &(cls.base[start]))->theProperty.originClassName,
        &cls.mem);

    _setString(propRep->_referenceClassName.getString(),
        ((SCMBClassPropertyNode*)&(cls.base[start]))->theProperty.refClassName,
        &cls.mem);


    SCMBClassPropertyNode* scmoPropNode =
        (SCMBClassPropertyNode*)&(cls.base[start]);

    // generate new hash tag
    scmoPropNode->theProperty.nameHashTag =
        _generateSCMOStringTag(scmoPropNode->theProperty.name,cls.base);

    scmoPropNode->theProperty.flags.propagated = propRep->_propagated;

    // just for complete intialization.
    scmoPropNode->hasNext=false;
    scmoPropNode->nextNode=0;

    // calculate the relative pointer for the default value
    Uint64 valueStart =
        (char*)&scmoPropNode->theProperty.defaultValue - cls.base;

    _setValue(valueStart,propRep->_value);

    *isKey = _setPropertyQualifiers(start,propRep->_qualifiers);
    // reset property node pointer
    scmoPropNode = (SCMBClassPropertyNode*)&(cls.base[start]);
    scmoPropNode->theProperty.flags.isKey=*isKey;
}

Boolean SCMOClass::_setPropertyQualifiers(
    Uint64 start,
    const CIMQualifierList& theQualifierList)
{
    Uint32 noQuali = theQualifierList.getCount();
    Uint64 startArray;
    QualifierNameEnum propName;
    Boolean isKey = false;

    SCMBClassPropertyNode* scmoPropNode =
        (SCMBClassPropertyNode*)&(cls.base[start]);

    scmoPropNode->theProperty.numberOfQualifiers = noQuali;


    if (noQuali != 0)
    {
        // allocate qualifier array and save the start of the array.
        startArray = _getFreeSpace(scmoPropNode->theProperty.qualifierArray,
                         sizeof(SCMBQualifier)*noQuali,
                         &cls.mem);
        for (Uint32 i = 0; i < noQuali; i++)
        {
            propName = _setQualifier(
                startArray,
                theQualifierList.getQualifier(i));
            // Only set once if the property is identified as key.
            if(!isKey)
            {
                isKey = propName==QUALNAME_KEY;
            }

            startArray = startArray + sizeof(SCMBQualifier);
        }
    }
    else
    {
        scmoPropNode->theProperty.qualifierArray.start=0;
        scmoPropNode->theProperty.qualifierArray.size=0;
    }

    return isKey;
}

void SCMOClass::_setClassQualifers(const CIMQualifierList& theQualifierList)
{

    Uint32 noQuali = theQualifierList.getCount();
    Uint64 start;

    cls.hdr->numberOfQualifiers = noQuali;

    if (noQuali != 0)
    {
        // allocate qualifier array and save the start of the array.
        start = _getFreeSpace(cls.hdr->qualifierArray,
                      sizeof(SCMBQualifier)*noQuali,
                      &cls.mem);
        for (Uint32 i = 0; i < noQuali; i++)
        {
            _setQualifier(start,theQualifierList.getQualifier(i));
            start = start + sizeof(SCMBQualifier);

        }

    }
    else
    {
        cls.hdr->qualifierArray.start=0;
        cls.hdr->qualifierArray.size=0;
    }
}

QualifierNameEnum SCMOClass::_setQualifier(
    Uint64 start,
    const CIMQualifier& theCIMQualifier)
{
    Uint64 valueStart;
    QualifierNameEnum name;

    SCMBQualifier* scmoQual = (SCMBQualifier*)&(cls.base[start]);
    name=_getSCMOQualifierNameEnum(theCIMQualifier.getName());

    scmoQual->propagated = theCIMQualifier.getPropagated();
    scmoQual->name = name;
    scmoQual->flavor = theCIMQualifier.getFlavor().cimFlavor;

    valueStart = (char*)&scmoQual->value - cls.base;

    _setValue(valueStart,theCIMQualifier.getValue());

    // reset the pointer, because the base pointer could be reallocated.
    scmoQual = (SCMBQualifier*)&(cls.base[start]);
    if (name == QUALNAME_USERDEFINED)
    {
        _setString(theCIMQualifier.getName().getString(),
                   scmoQual->userDefName,&cls.mem);
    }

    return name;
}

void SCMOClass::_setValue(
    Uint64 start,
    const CIMValue& theCIMValue)
{
    Uint64 valueStart;

    CIMValueRep* rep = *((CIMValueRep**)&theCIMValue);

    SCMBValue* scmoValue = (SCMBValue*)&(cls.base[start]);
    scmoValue->valueType = rep->type;
    scmoValue->valueArraySize = 0;
    scmoValue->flags.isNull = rep->isNull;
    scmoValue->flags.isArray = rep->isArray;
    scmoValue->flags.isSet = false;

    if (rep->isNull)
    {
        return;
    }

    valueStart = (char*)&scmoValue->value - cls.base;

    if (scmoValue->flags.isArray)
    {
        SCMOInstance::_setUnionArrayValue(
            valueStart,
            &cls.mem,
            rep->type,
            // Is set to the number of array members by the function.
            scmoValue->valueArraySize,
            cls.hdr->nameSpace.start,
            cls.hdr->nameSpace.size,
            rep->u);
    }
    else
    {
        SCMOInstance::_setUnionValue(
            valueStart,
            &cls.mem,
            rep->type,
            cls.hdr->nameSpace.start,
            cls.hdr->nameSpace.size,
            rep->u);
    }
}

QualifierNameEnum SCMOClass::_getSCMOQualifierNameEnum(
    const CIMName& theCIMName)
{
    // Get the UTF8 CString
    CString theCString=theCIMName.getString().getCString();
    // Get the real size of the UTF8 sting.
    Uint32 length = strlen((const char*)theCString);


    // The start index is 1, because the at index 0 is a place holder for
    // the user defined qualifier name which is not part of the qualifier name
    // list.
    for (Uint32 i = 1; i < _NUM_QUALIFIER_NAMES; i++)
    {
        if (qualifierNameStrLit(i).size == length)
        {
            // TBD: Make it more efficent...
            if(String::equalNoCase(
                theCIMName.getString(),
                qualifierNameStrLit(i).str))
            {
                return (QualifierNameEnum)i;
            }
        }
    }

    return QUALNAME_USERDEFINED;
}

Boolean SCMOClass::_isSamePropOrigin(Uint32 node, const char* origin) const
{
   Uint32 len = strlen(origin);

   SCMBClassPropertyNode* nodeArray =
       (SCMBClassPropertyNode*)
           &(cls.base[cls.hdr->propertySet.nodeArray.start]);

   return(_equalNoCaseUTF8Strings(
       nodeArray[node].theProperty.originClassName,
       cls.base,
       origin,
       len));
}

SCMO_RC SCMOClass::_isNodeSameType(
    Uint32 node,
    CIMType type,
    Boolean isArray,
    CIMType& realType) const
{

    SCMBClassPropertyNode* nodeArray =
        (SCMBClassPropertyNode*)
            &(cls.base[cls.hdr->propertySet.nodeArray.start]);

    // The type stored in the class information is set on realType.
    // It must be used in further calls to guaranty consistence.
    realType = nodeArray[node].theProperty.defaultValue.valueType;

    if(nodeArray[node].theProperty.defaultValue.valueType != type)
    {
        // Accept a property of type instance also
        // for an CIMTYPE_OBJECT property.
        if (!(type == CIMTYPE_INSTANCE &&
              nodeArray[node].theProperty.defaultValue.valueType
              == CIMTYPE_OBJECT))
        {
            return SCMO_WRONG_TYPE;
        }
    }

    if (isArray)
    {
        if (nodeArray[node].theProperty.defaultValue.flags.isArray)
        {
            return SCMO_OK;
        }
        else
        {
            return SCMO_NOT_AN_ARRAY;
        }

    }

    if (nodeArray[node].theProperty.defaultValue.flags.isArray)
    {
        return SCMO_IS_AN_ARRAY;
    }

    return SCMO_OK;

}

/*****************************************************************************
 * The SCMOInstance methods
 *****************************************************************************/

SCMOInstance::SCMOInstance()
{
    inst.base = 0;
}

SCMOInstance::SCMOInstance(SCMOClass& baseClass)
{
    _initSCMOInstance(new SCMOClass(baseClass));
}

SCMOInstance::SCMOInstance(
    SCMOClass& baseClass,
    Boolean includeQualifiers,
    Boolean includeClassOrigin)
{
    _initSCMOInstance(new SCMOClass(baseClass));

    inst.hdr->flags.includeQualifiers=includeQualifiers;
    inst.hdr->flags.includeClassOrigin=includeClassOrigin;
}

SCMOInstance::SCMOInstance(SCMOClass& baseClass, const CIMObjectPath& cimObj)
{
    _initSCMOInstance(new SCMOClass(baseClass));

    _setCIMObjectPath(cimObj);
}

SCMOInstance::SCMOInstance(SCMOClass& baseClass, const CIMInstance& cimInstance)
{
    _initSCMOInstance(new SCMOClass(baseClass));

    _setCIMInstance(cimInstance);
}

SCMOInstance::SCMOInstance(CIMClass& theCIMClass, const char* altNameSpace)
{
    _initSCMOInstance(new SCMOClass(theCIMClass,altNameSpace));
}

/*
    Construct a new SCMOInstance from a CIMInstance. This constructor
    gets the SCMOClass from the cache
*/
SCMOInstance::SCMOInstance(
    const CIMInstance& cimInstance,
    const char* altNameSpace,
    Uint32 altNSLen)
{
    SCMOClass theSCMOClass = _getSCMOClass(
        cimInstance._rep->_reference,
        altNameSpace,
        altNSLen);

    _initSCMOInstance( new SCMOClass(theSCMOClass));

    // If class empty, the name info not set into the instance
    if(theSCMOClass.isEmpty())
    {
        // flag the instance as compromized
        inst.hdr->flags.isCompromised=true;
        inst.hdr->flags.noClassForInstance=true;

        // Copy class name of the class from the
        // CIMInstance

        _setString(cimInstance.getClassName().getString(),
            inst.hdr->instClassName,
            &inst.mem);

        _setBinary(altNameSpace, altNSLen,
                   inst.hdr->instNameSpace, &inst.mem);
    }

    _setCIMInstance(cimInstance);

}

SCMOInstance::SCMOInstance(
    const CIMObject& cimObject,
    const char* altNameSpace,
    Uint32 altNSLen)
{
    if (cimObject.isClass())
    {
        CIMClass cimClass(cimObject);

        _initSCMOInstance(new SCMOClass(cimClass,altNameSpace));

        inst.hdr->flags.isClassOnly=true;
    }
    else
    {
        CIMInstance cimInstance(cimObject);

        SCMOClass theSCMOClass = _getSCMOClass(
            cimInstance._rep->_reference,
            altNameSpace,
            altNSLen);

        _initSCMOInstance( new SCMOClass(theSCMOClass));

        if(theSCMOClass.isEmpty())
        {
            // flag the instance as compromized
            inst.hdr->flags.isCompromised=true;
            inst.hdr->flags.noClassForInstance=true;
        }

        _setCIMInstance(cimInstance);
    }
}

SCMOInstance::SCMOInstance(
    const CIMObjectPath& cimObj,
    const char* altNameSpace,
    Uint32 altNSLen)
{
    SCMOClass theSCMOClass = _getSCMOClass(
        cimObj,
        altNameSpace,
        altNSLen);

    _initSCMOInstance( new SCMOClass(theSCMOClass));

    if(theSCMOClass.isEmpty())
    {
        // flag the instance as compromized
        inst.hdr->flags.isCompromised=true;
        // KS_TODO should we set the noClassForInstance
        // and set the path for this one??

        inst.hdr->flags.noClassForInstance=true;
    }

     _setCIMObjectPath(cimObj);
}

void SCMOInstance::_destroyExternalReferences()
{
    _destroyExternalReferencesInternal(inst.mem);
}

SCMOClass SCMOInstance::_getSCMOClass(
    const CIMObjectPath& theCIMObj,
    const char* altNS,
    Uint32 altNSlength)
{
    SCMOClass theClass;

    if (theCIMObj.getClassName().isNull())
    {
        return SCMOClass();
    }

    if (theCIMObj.getNameSpace().isNull())
    {
        // the name space of the object path is empty,
        // use alternative name space.
        CString clsName = theCIMObj.getClassName().getString().getCString();

        SCMOClassCache* theCache = SCMOClassCache::getInstance();
        theClass = theCache->getSCMOClass(
            altNS,
            altNSlength,
            (const char*)clsName,
            strlen(clsName));
    }
    else
    {
        CString nameSpace = theCIMObj.getNameSpace().getString().getCString();
        CString clsName = theCIMObj.getClassName().getString().getCString();

        SCMOClassCache* theCache = SCMOClassCache::getInstance();
        theClass = theCache->getSCMOClass(
            (const char*)nameSpace,
            strlen(nameSpace),
            (const char*)clsName,
            strlen(clsName));
    }

    return theClass;
}

#define PEGASUS_SIZE_REFERENCE_INDEX_ARRAY 8

void SCMOInstance::_setExtRefIndex(SCMBUnion* pInst, SCMBMgmt_Header** pmem)
{
    Uint64 refPtr =(((char *)pInst) - (char *)(*pmem));
    SCMBMgmt_Header* memHdr = (*pmem);
    // Save the number of external references in the array
    Uint32 noExtRef = memHdr->numberExtRef;

    // Allocate the external reference array
    // if it is full or empty ( 0 == 0 ).
    if (noExtRef == memHdr->sizeExtRefIndexArray)
    {
        Uint64 oldArrayStart = memHdr->extRefIndexArray.start;
        Uint32 newSize =
            memHdr->sizeExtRefIndexArray + PEGASUS_SIZE_REFERENCE_INDEX_ARRAY;

        // Allocate the external reference index array
        _getFreeSpace(
              memHdr->extRefIndexArray,
              sizeof(Uint64)*newSize,
              pmem);

        // reset the pointer. It could be changed due to reallocation !
        memHdr = (*pmem);

        // Assign new size.
        memHdr->sizeExtRefIndexArray=newSize;

        // Get absolute pointer to old index array.
        Uint64* oldArray = (Uint64*)&(((char*)(*pmem))[oldArrayStart]);
        // Get absolute pointer to new index array
        Uint64* newArray =
            (Uint64*)&(((char*)(*pmem))[memHdr->extRefIndexArray.start]);

        // Copy all elements of the old array to the new.
        // If noExtRef = 0, no elements are copied.
        for (Uint32 i = 0 ; i < noExtRef ; i++)
        {
            newArray[i] = oldArray[i];
        }
    }

    // Get absolute pointer to the array
    Uint64* array =
        (Uint64*)&(((char*)(*pmem))[memHdr->extRefIndexArray.start]);
    // look in the table if the index is already in the array
    for (Uint32 i = 0 ; i < noExtRef ; i++)
    {
        // is the index already part of the array
        if (array[i] == refPtr)
        {
            // leave.
            return;
        }
    }
    // It is not part of the array -> set the new index.
    array[noExtRef] = refPtr;
    // increment the nuber of external references of this instance.
    memHdr->numberExtRef++;

}

SCMOInstance* SCMOInstance::getExtRef(Uint32 idx) const
{
    Uint64* array =
        (Uint64*)&(inst.base[inst.mem->extRefIndexArray.start]);
    SCMBUnion* pUnion;
    pUnion = (SCMBUnion*)(&(inst.base[array[idx]]));
    return pUnion->extRefPtr;
}

void SCMOInstance::putExtRef(Uint32 idx,SCMOInstance* ptr)
{
    Uint64* array =
        (Uint64*)&(inst.base[inst.mem->extRefIndexArray.start]);
    SCMBUnion* pUnion;
    pUnion = (SCMBUnion*)(&(inst.base[array[idx]]));
    pUnion->extRefPtr = ptr;
}

void SCMOInstance::_copyExternalReferences()
{
    Uint32 number = inst.mem->numberExtRef;

    if (0 != number)
    {
        SCMBUnion* pUnion;
        Uint64* array =
            (Uint64*)&(inst.base[inst.mem->extRefIndexArray.start]);
        for (Uint32 i = 0; i < number; i++)
        {
            pUnion = (SCMBUnion*)(&(inst.base[array[i]]));
            if (0 != pUnion)
            {
                pUnion->extRefPtr = new SCMOInstance(*(pUnion->extRefPtr));
            }
        }

    }

}

void SCMOInstance::_destroyExternalKeyBindings()
{
    // Create a pointer to keybinding node array of the class.
    Uint64 idx = inst.hdr->theClass.ptr->cls.hdr->keyBindingSet.nodeArray.start;
    SCMBKeyBindingNode* theClassKeyBindNodeArray =
        (SCMBKeyBindingNode*)&((inst.hdr->theClass.ptr->cls.base)[idx]);

    // create a pointer to instance key binding array.
    SCMBKeyBindingValue* theInstanceKeyBindingNodeArray =
        (SCMBKeyBindingValue*)&(inst.base[inst.hdr->keyBindingArray.start]);

    for (Uint32 i = 0; i < inst.hdr->numberKeyBindings; i++)
    {
        if (theInstanceKeyBindingNodeArray[i].isSet)
        {
            // only references can be a key binding
            if (theClassKeyBindNodeArray[i].type == CIMTYPE_REFERENCE)
            {
               _deleteExternalReferenceInternal(
                   inst.mem,
                   theInstanceKeyBindingNodeArray[i].data.extRefPtr);
            }
        }
    }// for all key bindings

    // Are there user defined key bindings ?
    if (0 != inst.hdr->numberUserKeyBindings)
    {
        SCMBUserKeyBindingElement* theUserDefKBElement =
            (SCMBUserKeyBindingElement*)
                 &(inst.base[inst.hdr->userKeyBindingElement.start]);

        for(Uint32 i = 0; i < inst.hdr->numberUserKeyBindings; i++)
        {
            if (theUserDefKBElement->value.isSet)
            {
                // only references can be a key binding.
                if (theUserDefKBElement->type == CIMTYPE_REFERENCE)
                {
                   _deleteExternalReferenceInternal(
                       inst.mem,
                       theUserDefKBElement->value.data.extRefPtr);
                }
            }

            theUserDefKBElement =
                (SCMBUserKeyBindingElement*)
                     &(inst.base[theUserDefKBElement->nextElement.start]);
        } // for all user def. key bindings.
    }
}

SCMO_RC SCMOInstance::getCIMInstance(CIMInstance& cimInstance) const
{
    SCMO_RC rc = SCMO_OK;
    CIMObjectPath objPath;

    // For better usability define pointers to SCMO Class data structures.
    SCMBClass_Main* clshdr = inst.hdr->theClass.ptr->cls.hdr;
    char* clsbase = inst.hdr->theClass.ptr->cls.base;

    getCIMObjectPath(objPath);

    CIMInstance newInstance;
    newInstance._rep = new CIMInstanceRep(objPath);

    if (inst.hdr->flags.includeQualifiers)
    {
        SCMBQualifier* qualiArray =
            (SCMBQualifier*)&(clsbase[clshdr->qualifierArray.start]);


        CIMQualifier theCimQualifier;
        Uint32 i, k = clshdr->numberOfQualifiers;

        for ( i = 0 ; i < k ; i++)
        {
            SCMOClass::_getCIMQualifierFromSCMBQualifier(
                theCimQualifier,
                qualiArray[i],
                clsbase);

            newInstance._rep->_qualifiers.addUnchecked(theCimQualifier);
        }
    }

    // User-defined properties and exportSetOnly
    // are mutually exclusive. Do not appear in same instance.
    // KS_TODO insure that this is true or change to export all.
    // This flag is only set by _setCIMInstance

    if (inst.hdr->flags.exportSetOnly)
    {
        for(Uint32 i = 0, k = inst.hdr->numberProperties; i<k; i++)
        {
            SCMBValue* theInstPropArray =
                (SCMBValue*)&(inst.base[inst.hdr->propertyArray.start]);

            // was the property set by the provider ?
            if(theInstPropArray[i].flags.isSet)
            {
                // no filtering. Counter is node index
                CIMProperty theProperty=_getCIMPropertyAtNodeIndex(i);

                newInstance._rep->_properties.append(theProperty);
            }
        }
    }
    else
    {
        // export all properties
        Uint32 propertyCount = getPropertyCount();
        if (!noClassForInstance())
        {
            for(Uint32 i = 0, k = propertyCount; i<k; i++)
            {
                // Set all properties in the CIMInstance gegarding they
                // are part of the SCMOInstance or the SCMOClass.
                CIMProperty theProperty = _getCIMPropertyAtNodeIndex(i);

                newInstance._rep->_properties.append(theProperty);
            }
        }
        else
        {
            // Create cim property for all properties in SCMO Instance.
            // Work end to beginning only because User-defined properties
            // are inserted in reverse order and that upsets instance
            // identical testing
            if (propertyCount != 0)
            {
                Uint32 i = propertyCount - 1;
                do
                {
                    // Set all properties in the CIMInstance whether they
                    // are part of the SCMOInstance or the SCMOClass.
                    CIMProperty theProperty = _getCIMPropertyAtNodeIndex(i);

                    newInstance._rep->_properties.append(theProperty);
                }
                while (i-- != 0);
            }
        }
    }

    cimInstance = newInstance;

    return rc;
}

void SCMOInstance::getCIMObjectPath(CIMObjectPath& cimObj) const
{
    Array<CIMKeyBinding> keys;

    // For better usability define pointers to SCMO Class data structures.
    SCMBClass_Main* clshdr = inst.hdr->theClass.ptr->cls.hdr;
    char* clsbase = inst.hdr->theClass.ptr->cls.base;

    // Address the class keybinding information
    SCMBKeyBindingNode* scmoClassArray =
        (SCMBKeyBindingNode*)&(clsbase[clshdr->keyBindingSet.nodeArray.start]);

    // Address the instance keybinding information
    SCMBKeyBindingValue* scmoInstArray =
        (SCMBKeyBindingValue*)&(inst.base[inst.hdr->keyBindingArray.start]);

    Uint32 numberKeyBindings = inst.hdr->numberKeyBindings;

    CIMValue theKeyBindingValue;

    for (Uint32 i = 0; i < numberKeyBindings; i ++)
    {
        if (scmoInstArray[i].isSet)
        {
            _getCIMValueFromSCMBUnion(
                theKeyBindingValue,
                scmoClassArray[i].type,
                false, // can never be a null value
                false, // can never be an array
                0,
                scmoInstArray[i].data,
                inst.base);
            keys.append(
                CIMKeyBinding(
                    CIMNameCast(_newCimString(scmoClassArray[i].name,clsbase)),
                    theKeyBindingValue
                    ));
        }
    }

    // Are there user defined key bindings ?
    if (0 != inst.hdr->numberUserKeyBindings)
    {
        SCMBUserKeyBindingElement* theUserDefKBElement =
            (SCMBUserKeyBindingElement*)
                 &(inst.base[inst.hdr->userKeyBindingElement.start]);

        for(Uint32 i = 0; i < inst.hdr->numberUserKeyBindings; i++)
        {
            if (theUserDefKBElement->value.isSet)
            {
                _getCIMValueFromSCMBUnion(
                    theKeyBindingValue,
                    theUserDefKBElement->type,
                    false, // can never be a null value
                    false, // can never be an array
                    0,
                    theUserDefKBElement->value.data,
                    inst.base);

                keys.append(
                    CIMKeyBinding(
                        CIMNameCast(
                            _newCimString(theUserDefKBElement->name,inst.base)),
                    theKeyBindingValue));
            }
            theUserDefKBElement =
                (SCMBUserKeyBindingElement*)
                     &(inst.base[theUserDefKBElement->nextElement.start]);
        } // for all user def. key bindings.
    }

    String host = _newCimString(inst.hdr->hostName,inst.base);

    // Use name space and class name of the instance
    CIMNamespaceName nameSpace =
        CIMNamespaceNameCast(_newCimString(inst.hdr->instNameSpace,inst.base));

    CIMName className =
        CIMNameCast(_newCimString(inst.hdr->instClassName,inst.base));

    cimObj.set(host,nameSpace,className,keys);
}

CIMProperty SCMOInstance::_getCIMPropertyAtNodeIndex(Uint32 nodeIdx) const
{
    CIMProperty retProperty;

    // For better usability define pointers to SCMO Class data structures.
    SCMBClass_Main* clshdr = inst.hdr->theClass.ptr->cls.hdr;
    char* clsbase = inst.hdr->theClass.ptr->cls.base;

    if (_isClassDefinedProperty(nodeIdx))
    {
        // For better usability define pointers to SCMO Class data structures.

        SCMBClassPropertyNode& clsProp =
            ((SCMBClassPropertyNode*)
             &(clsbase[clshdr->propertySet.nodeArray.start]))[nodeIdx];

        SCMBValue& instValue =
            ((SCMBValue*)&(inst.base[inst.hdr->propertyArray.start]))[nodeIdx];

        // if set, get value. Else get value from class
        CIMValue theCIMValue;
        if (instValue.flags.isSet)
        {
            _getCIMValueFromSCMBValue(theCIMValue,instValue,inst.base);
        }
        else
        {
            _getCIMValueFromSCMBValue(
                theCIMValue,
                clsProp.theProperty.defaultValue,
                clsbase);
        }

        // Redo this into single call KS_TODO

        if (inst.hdr->flags.includeClassOrigin)
        {
            retProperty = CIMProperty(
                CIMNameCast(_newCimString(clsProp.theProperty.name,clsbase)),
                theCIMValue,
                theCIMValue.getArraySize(),
                CIMNameCast(
                    _newCimString(clsProp.theProperty.refClassName,clsbase)),
                CIMNameCast(
                    _newCimString(clsProp.theProperty.originClassName,clsbase)),
                clsProp.theProperty.flags.propagated);
        }
        else
        {
             retProperty = CIMProperty(
                CIMNameCast(_newCimString(clsProp.theProperty.name,clsbase)),
                theCIMValue,
                theCIMValue.getArraySize(),
                CIMNameCast(
                    _newCimString(clsProp.theProperty.refClassName,clsbase)),
                CIMName(),
                clsProp.theProperty.flags.propagated);
        }

        if (inst.hdr->flags.includeQualifiers)
        {
            SCMBQualifier* qualiArray =
                (SCMBQualifier*)
                     &(clsbase[clsProp.theProperty.qualifierArray.start]);

            CIMQualifier theCimQualifier;

            Uint32 i, k = clsProp.theProperty.numberOfQualifiers;
            for ( i = 0 ; i < k ; i++)
            {
                SCMOClass::_getCIMQualifierFromSCMBQualifier(
                    theCimQualifier,
                    qualiArray[i],
                    clsbase);

                retProperty._rep->_qualifiers.addUnchecked(theCimQualifier);
            }
        }
    }
    else   // User-defined property
    {
        SCMBUserPropertyElement* pElement =
            _getUserDefinedPropertyElementAt(nodeIdx);

        SCMBValue& instValue = pElement->value;

        CIMValue theCimValue;
        if (instValue.flags.isSet)
        {
            // There is No default so
            _getCIMValueFromSCMBValue(theCimValue,instValue,inst.base);
        }
        else
        {
            // There is no default. isSet MUST be set
            PEGASUS_ASSERT(false);
        }

        retProperty = CIMProperty(
           CIMNameCast(_newCimString(pElement->name, inst.base)),
           theCimValue,
           theCimValue.getArraySize(),
           CIMName(),          // referenceClassName
           CIMName(),          // ClassOrigin
           false);             // isPropagated == false
    }
    return retProperty;
}

void SCMOInstance::_getCIMValueFromSCMBUnion(
    CIMValue& cimV,
    const CIMType type,
    const Boolean isNull,
    const Boolean isArray,
    const Uint32 arraySize,
    const SCMBUnion& scmbUn,
    const char * base)
{

    const SCMBUnion* pscmbArrayUn = 0;

    if (isNull)
    {
        cimV.setNullValue(type,isArray,arraySize);
        return;
    }

    if (isArray)
    {
        pscmbArrayUn =(SCMBUnion*)&(base[scmbUn.arrayValue.start]);
    }

    switch (type)
    {

    case CIMTYPE_UINT8:
        {
            if (isArray)
            {
                Array<Uint8> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.u8);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.u8);
            }
            break;
        }

    case CIMTYPE_UINT16:
        {
            if (isArray)
            {
                Array<Uint16> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.u16);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.u16);
            }
            break;
        }

    case CIMTYPE_UINT32:
        {
            if (isArray)
            {
                Array<Uint32> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.u32);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.u32);
            }
            break;
        }

    case CIMTYPE_UINT64:
        {
            if (isArray)
            {
                Array<Uint64> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.u64);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.u64);
            }
            break;
        }

    case CIMTYPE_SINT8:
        {
            if (isArray)
            {
                Array<Sint8> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.s8);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.s8);
            }
            break;
        }

    case CIMTYPE_SINT16:
        {
            if (isArray)
            {
                Array<Sint16> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.s16);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.s16);
            }
            break;
        }

    case CIMTYPE_SINT32:
        {
            if (isArray)
            {
                Array<Sint32> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.s32);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.s32);
            }
            break;
        }

    case CIMTYPE_SINT64:
        {
            if (isArray)
            {
                Array<Sint64> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.s64);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.s64);
            }
            break;
        }

    case CIMTYPE_REAL32:
        {
            if (isArray)
            {
                Array<Real32> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.r32);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.r32);
            }
            break;
        }

    case CIMTYPE_REAL64:
        {
            if (isArray)
            {
                Array<Real64> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.r64);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.r64);
            }
            break;
        }

    case CIMTYPE_CHAR16:
        {
            if (isArray)
            {
                Array<Char16> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(Char16(pscmbArrayUn[i].simple.val.c16));
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(Char16(scmbUn.simple.val.c16));
            }
            break;
        }

    case CIMTYPE_BOOLEAN:
        {
            if (isArray)
            {
                Array<Boolean> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(pscmbArrayUn[i].simple.val.bin);
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(scmbUn.simple.val.bin);
            }
            break;
        }

    case CIMTYPE_STRING:
        {
            if (isArray)
            {

                Array<String> x;

                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(_newCimString(pscmbArrayUn[i].stringValue,base));
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(_newCimString(scmbUn.stringValue,base));
            }
            break;
        }

    case CIMTYPE_DATETIME:
        {
            if (isArray)
            {
                Array<CIMDateTime> x;

                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    x.append(CIMDateTime(&(pscmbArrayUn[i].dateTimeValue)));
                }
                cimV.set(x);
            }
            else
            {
                cimV.set(CIMDateTime(&scmbUn.dateTimeValue));
            }
            break;

        }

    case CIMTYPE_REFERENCE:
        {
            CIMObjectPath theObjPath;

            if(isArray)
            {
                Array<CIMObjectPath> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    if (0 != pscmbArrayUn[i].extRefPtr)
                    {
                        pscmbArrayUn[i].extRefPtr->getCIMObjectPath(theObjPath);
                        x.append(theObjPath);
                    }
                    else
                    {
                        // set an empty object
                        x.append(CIMObjectPath());
                    }
                }
                cimV.set(x);
            }
            else
            {

                if (0 != scmbUn.extRefPtr)
                {
                    scmbUn.extRefPtr->getCIMObjectPath(theObjPath);
                    cimV.set(theObjPath);
                }
                else
                {
                    cimV.set(CIMObjectPath());
                }
            }
            break;
        }
    case CIMTYPE_OBJECT:
        {
            CIMInstance cimInstance;
            CIMClass cimClass;

            if(isArray)
            {
                Array<CIMObject> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    if (0 != pscmbArrayUn[i].extRefPtr)
                    {
                        // check if the Object is an emedded instance or class
                        if(pscmbArrayUn[i].extRefPtr->
                               inst.hdr->flags.isClassOnly)
                        {
                            pscmbArrayUn[i].extRefPtr->
                                inst.hdr->theClass.ptr->getCIMClass(cimClass);
                            x.append(CIMObject(cimClass));
                        }
                        else
                        {
                            pscmbArrayUn[i].extRefPtr->
                                getCIMInstance(cimInstance);
                            x.append(CIMObject(cimInstance));
                        }
                    }
                    else
                    {
                        // set an empty object
                        x.append(CIMObject());
                    }
                }
                cimV.set(x);
            }
            else
            {

                if (0 != scmbUn.extRefPtr)
                {
                    // check if the Object is an emedded instance or class
                    if(scmbUn.extRefPtr->inst.hdr->flags.isClassOnly)
                    {
                        scmbUn.extRefPtr->
                            inst.hdr->theClass.ptr->getCIMClass(cimClass);
                        cimV.set(CIMObject(cimClass));
                    }
                    else
                    {
                        scmbUn.extRefPtr->getCIMInstance(cimInstance);
                        cimV.set(CIMObject(cimInstance));
                    }
                }
                else
                {
                    cimV.set(CIMObject());
                }
            }
            break;
        }

    case CIMTYPE_INSTANCE:
        {
            CIMInstance theInstance;

            if(isArray)
            {
                Array<CIMInstance> x;
                for (Uint32 i = 0, k = arraySize; i < k ; i++)
                {
                    if (0 != pscmbArrayUn[i].extRefPtr)
                    {
                        pscmbArrayUn[i].extRefPtr->
                            getCIMInstance(theInstance);
                        x.append(theInstance);
                    }
                    else
                    {
                        // set an empty object
                        x.append(CIMInstance());
                    }
                }
                cimV.set(x);
            }
            else
            {

                if (0 != scmbUn.extRefPtr)
                {
                    scmbUn.extRefPtr->getCIMInstance(theInstance);
                    cimV.set(theInstance);
                }
                else
                {
                    cimV.set(CIMInstance());
                }
            }
            break;
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }
}

void SCMOInstance::_getCIMValueFromSCMBValue(
    CIMValue& cimV,
    const SCMBValue& scmbV,
    const char * base)
{
    SCMOInstance::_getCIMValueFromSCMBUnion(
        cimV,
        scmbV.valueType,
        scmbV.flags.isNull,
        scmbV.flags.isArray,
        scmbV.valueArraySize,
        scmbV.value,
        base);
}


void SCMOInstance::_setCIMObjectPath(const CIMObjectPath& cimObj)
{
    CString className = cimObj.getClassName().getString().getCString();

    // Is the instance from the same class ?
    if (!(_equalNoCaseUTF8Strings(
             inst.hdr->instClassName,
             inst.base,
             (const char*)className,
             strlen(className))))
    {
        throw PEGASUS_CIM_EXCEPTION(CIM_ERR_INVALID_CLASS,
           cimObj.getClassName().getString());
    }

    //set host name
    _setString(cimObj.getHost(),inst.hdr->hostName,&inst.mem );

    const Array<CIMKeyBinding> & keys=cimObj.getKeyBindings();
    for (Uint32 i = 0, k = keys.size(); i < k; i++)
    {
        String key = keys[i].getValue();
        _setKeyBindingFromString(
            (const char*) keys[i].getName().getString().getCString(),
            _CIMTypeFromKeyBindingType(
                (const char*)key.getCString(),
                keys[i].getType()),
            key);
    }
}
/*
    This function handles both class and user-defined properties
    because _getSCMBValueForNode handles both
*/
void SCMOInstance::_setCIMValueAtNodeIndex(
    Uint32 node,
    CIMValueRep* valRep,
    CIMType realType)
{
    SCMBValue& theInstProp = _getSCMBValueForNode(node);

    theInstProp.valueType=realType;
    theInstProp.flags.isNull=valRep->isNull;
    theInstProp.flags.isArray=valRep->isArray;
    theInstProp.flags.isSet=true;
    theInstProp.valueArraySize=0;

    if (valRep->isNull)
    {
        return;
    }

    Uint64 start = ((const char*)&(theInstProp.value))-inst.base;

    if (valRep->isArray)
    {
        _setUnionArrayValue(
            start,
            &inst.mem,
            realType,
            // Is set to the number of array members by the function.
            theInstProp.valueArraySize,
            inst.hdr->instNameSpace.start,
            inst.hdr->instNameSpace.size,
            valRep->u);
    }
    else
    {
        _setUnionValue(
            start,
            &inst.mem,
            realType,
            inst.hdr->instNameSpace.start,
            inst.hdr->instNameSpace.size,
            valRep->u);
    }
}


Boolean SCMOInstance::isSame(SCMOInstance& theInstance) const
{
    return inst.base == theInstance.inst.base;
}

void SCMOInstance::setHostName(const char* hostName)
{
    Uint32 len = 0;

    _copyOnWrite();

    if (hostName!=0)
    {

        len = strlen((const char*)hostName);
    }
    // copy including trailing '\0'
    _setBinary(hostName,len+1,inst.hdr->hostName,&inst.mem);
}

const char* SCMOInstance::getHostName() const
{
    return _getCharString(inst.hdr->hostName,inst.base);
}

const char* SCMOInstance::getHostName_l(Uint32& length) const
{
    length = inst.hdr->hostName.size;
    if (0 == length)
    {
        return 0;
    }
    else
    {
        length--;
    }
    return _getCharString(inst.hdr->hostName,inst.base);
}

void SCMOInstance::setClassName(const char* className)
{
    Uint32 len=0;

    _copyOnWrite();

    // flag the instance as compromized
    inst.hdr->flags.isCompromised=true;
    if (className!=0)
    {
        len = strlen((const char*)className);
    }
    // copy including trailing '\0'
    // _setBinary also sets the name to 0 if either className==0 or len+1==1
    _setBinary(className,len+1,inst.hdr->instClassName,&inst.mem);
}

void SCMOInstance::setClassName_l(const char* className, Uint32 len)
{
    _copyOnWrite();

    // flag the instance as compromised
    inst.hdr->flags.isCompromised=true;
    // copy including trailing '\0'
    // _setBinary also sets the name to 0 if either className==0 or len+1==1
    _setBinary(className,len+1,inst.hdr->instClassName,&inst.mem);
}

const char* SCMOInstance::getClassName() const
{
    return _getCharString(inst.hdr->instClassName,inst.base);
}

const char* SCMOInstance::getClassName_l(Uint32 & length) const
{
    length = inst.hdr->instClassName.size;
    if (0 == length)
    {
        return 0;
    }
    else
    {
        length--;
    }
    return _getCharString(inst.hdr->instClassName,inst.base);
}

void SCMOInstance::setNameSpace(const char* nameSpace)
{
    Uint32 len = 0;

    _copyOnWrite();

    // flag the instance as compromized
    inst.hdr->flags.isCompromised=true;

    if (nameSpace!=0)
    {

        len = strlen((const char*)nameSpace);
    }
    // copy including trailing '\0'
    _setBinary(nameSpace,len+1,inst.hdr->instNameSpace,&inst.mem);
}

void SCMOInstance::setNameSpace_l(const char* nameSpace, Uint32 len)
{
    // Copy on Write is only necessary if a realloc() becomes necessary
    if (inst.mem->freeBytes < ((len+8) & ~7))
    {
        _copyOnWrite();
    }
    // flag the instance as compromized
    inst.hdr->flags.isCompromised=true;
    // copy including trailing '\0'
    _setBinary(nameSpace,len+1,inst.hdr->instNameSpace,&inst.mem);
}

const char* SCMOInstance::getNameSpace() const
{
    return _getCharString(inst.hdr->instNameSpace,inst.base);
}

const char* SCMOInstance::getNameSpace_l(Uint32 & length) const
{
    length = inst.hdr->instNameSpace.size;
    if (0 == length)
    {
        return 0;
    }
    else
    {
        length--;
    }
    return _getCharString(inst.hdr->instNameSpace,inst.base);
}

void SCMOInstance::completeHostNameAndNamespace(
    const char* hn,
    Uint32 hnLen,
    const char* ns,
    Uint32 nsLen)
{
    // hostName is Null or empty String ?
    if (0 == inst.hdr->hostName.size ||
        0 == inst.base[inst.hdr->hostName.start])
    {
        // Copy on Write is only necessary if a realloc() becomes necessary
        if (inst.mem->freeBytes < ((hnLen+8) & ~7))
        {
            _copyOnWrite();
        }
        // copy including trailing '\0'
        _setBinary(hn,hnLen+1,inst.hdr->hostName,&inst.mem);
    }
    // namespace is Null or empty String ?
    if (0 == inst.hdr->instNameSpace.size ||
        0 == inst.base[inst.hdr->instNameSpace.start])
    {
        setNameSpace_l(ns,nsLen);
    }
}


void SCMOInstance::buildKeyBindingsFromProperties()
{
    Uint32 propNode;
    // The theClassKeyPropList pointer will always be valid,
    // even after a realloc() caused by copyOnWrite()
    // as this is an absolute pointer to the class which does not change
    Uint32* theClassKeyPropList =
        (Uint32*) &((inst.hdr->theClass.ptr->cls.base)
                    [(inst.hdr->theClass.ptr->cls.hdr->keyIndexList.start)]);

    SCMBKeyBindingValue* theKeyBindValueArray =
        (SCMBKeyBindingValue*)&(inst.base[inst.hdr->keyBindingArray.start]);
    SCMBValue* theInstPropNodeArray=
        (SCMBValue*)&inst.base[inst.hdr->propertyArray.start];

    inst.hdr->numberKeyBindings =
        inst.hdr->theClass.ptr->cls.hdr->keyBindingSet.number;

    for (Uint32 i = 0, k = inst.hdr->numberKeyBindings; i < k; i++)
    {
        // If the keybinding is not set.
        if (!theKeyBindValueArray[i].isSet)
        {
            // get the node index for this key binding form class
            propNode = theClassKeyPropList[i];

            // if property was set by the provider and it is not null.
            if ( theInstPropNodeArray[propNode].flags.isSet &&
                !theInstPropNodeArray[propNode].flags.isNull)
            {
                _copyOnWrite();
                // the instance pointers have to be recalculated as copyOnWrite
                // might change the absolute address of these pointers
                theInstPropNodeArray =
                    (SCMBValue*)&inst.base[inst.hdr->propertyArray.start];
                theKeyBindValueArray =
                    (SCMBKeyBindingValue*)
                        &(inst.base[inst.hdr->keyBindingArray.start]);

                _setKeyBindingFromSCMBUnion(
                    theInstPropNodeArray[propNode].valueType,
                    theInstPropNodeArray[propNode].value,
                    inst.base,
                    theKeyBindValueArray[i]);

                // the instance pointers have to be reinitialized each time,
                // because a reallocation can take place
                // in _setKeyBindingFromSCMBUnion()
                theKeyBindValueArray =
                    (SCMBKeyBindingValue*)
                        &(inst.base[inst.hdr->keyBindingArray.start]);
                theInstPropNodeArray =
                    (SCMBValue*)&inst.base[inst.hdr->propertyArray.start];

            }
        }
    }
}

void SCMOInstance::_setKeyBindingFromSCMBUnion(
    CIMType type,
    const SCMBUnion& u,
    const char * uBase,
    SCMBKeyBindingValue& keyData)
{
    switch (type)
    {
    case CIMTYPE_UINT8:
    case CIMTYPE_UINT16:
    case CIMTYPE_UINT32:
    case CIMTYPE_UINT64:
    case CIMTYPE_SINT8:
    case CIMTYPE_SINT16:
    case CIMTYPE_SINT32:
    case CIMTYPE_SINT64:
    case CIMTYPE_REAL32:
    case CIMTYPE_REAL64:
    case CIMTYPE_CHAR16:
    case CIMTYPE_BOOLEAN:
        {
            memcpy(&keyData.data,&u,sizeof(SCMBUnion));
            keyData.data.simple.hasValue=true;
            keyData.isSet=true;
            break;
        }
    case CIMTYPE_DATETIME:
        {
            memcpy(&keyData.data,&u,sizeof(SCMBUnion));
            keyData.isSet=true;
            break;
        }
    case CIMTYPE_STRING:
        {
            keyData.isSet=true;
            // Check if a key binding is set within the same instance.
            // If this is the case, a reallocation can take place and the
            // uBase pointer can be invalid and cause a read in freed memory!
            if (uBase == inst.base)
            {
                if (0 != u.stringValue.size )
                {
                    // We are doing a in instance copy of data.
                    // We can not use the _setBinary() function because
                    // all real pointer can be in valid after
                    // the _getFreeSprace() function!
                    // We have to save all relative pointer on the stack.
                    Uint64 start;
                    SCMBDataPtr tmp;
                    tmp.size = u.stringValue.size;
                    tmp.start = u.stringValue.start;

                    // In this function a reallocation may take place!
                    // The keyData.data.stringValue is set
                    // before the reallocation.
                    start = _getFreeSpace(
                        keyData.data.stringValue,
                        u.stringValue.size,
                        &inst.mem);
                    // Copy the string,
                    // but using the own base pointer and the saved relative
                    // string pointer.
                    memcpy(
                        &(inst.base[start]),
                        _getCharString(tmp,inst.base),
                        tmp.size);
                }
                else
                {
                   keyData.data.stringValue.size=0;
                   keyData.data.stringValue.start=0;
                }

            }
            else
            {
                _setBinary(
                    &uBase[u.stringValue.start],
                    u.stringValue.size,
                    keyData.data.stringValue,
                    &inst.mem);
            }

            break;
        }
    case CIMTYPE_REFERENCE:
        {
            if(0 != keyData.data.extRefPtr)
            {
                delete keyData.data.extRefPtr;
            }

            if(u.extRefPtr)
            {
                keyData.data.extRefPtr = new SCMOInstance(*u.extRefPtr);
                keyData.isSet=true;
                // This function can cause a reallocation !
                // Pointers can be invalid after the call.
                _setExtRefIndex(&(keyData.data),&inst.mem);
            }
            else
            {
                keyData.isSet=true;
                keyData.data.extRefPtr=0;
            }
            break;
        }
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        {
            // From PEP 194: EmbeddedObjects cannot be keys.
            throw TypeMismatchException();
            break;
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }
}

void SCMOInstance::_initSCMOInstance(SCMOClass* pClass)
{
    PEGASUS_ASSERT(SCMB_INITIAL_MEMORY_CHUNK_SIZE
        - sizeof(SCMBInstance_Main)>0);

    inst.base = (char*)malloc(SCMB_INITIAL_MEMORY_CHUNK_SIZE);
    if (inst.base == 0)
    {
        // Not enough memory!
        throw PEGASUS_STD(bad_alloc)();
    }

    memset(inst.base,0,sizeof(SCMBInstance_Main));

    // initalize eye catcher
    inst.hdr->header.magic=PEGASUS_SCMB_INSTANCE_MAGIC;
    inst.hdr->header.totalSize=SCMB_INITIAL_MEMORY_CHUNK_SIZE;
    // The # of bytes free
    inst.hdr->header.freeBytes=
        SCMB_INITIAL_MEMORY_CHUNK_SIZE-sizeof(SCMBInstance_Main);
    // Index to the start of the free space in this instance
    inst.hdr->header.startOfFreeSpace=sizeof(SCMBInstance_Main);

    inst.hdr->refCount=1;
    inst.hdr->numberUserProperties = 0;

    if (pClass == NULL)
    {
        // Set number of properties
        inst.hdr->numberKeyBindings = 0;
        inst.hdr->numberProperties = 0;
        inst.hdr->flags.noClassForInstance = true;
    }
    else
    {
        //Assign the SCMBClass structure to this instance based on.
        inst.hdr->theClass.ptr = pClass;

    // Copy name space name and class name of the class
    _setBinary(
        _getCharString(inst.hdr->theClass.ptr->cls.hdr->className,
                       inst.hdr->theClass.ptr->cls.base),
        inst.hdr->theClass.ptr->cls.hdr->className.size,
        inst.hdr->instClassName,
        &inst.mem);

    _setBinary(
        _getCharString(inst.hdr->theClass.ptr->cls.hdr->nameSpace,
                       inst.hdr->theClass.ptr->cls.base),
        inst.hdr->theClass.ptr->cls.hdr->nameSpace.size,
        inst.hdr->instNameSpace,
        &inst.mem);

    // Number of key bindings
    inst.hdr->numberKeyBindings =
        inst.hdr->theClass.ptr->cls.hdr->keyBindingSet.number;

    // Number of properties
    inst.hdr->numberProperties =
        inst.hdr->theClass.ptr->cls.hdr->propertySet.number;
    }

    // Allocate the SCMOInstanceKeyBindingArray
    _getFreeSpace(
          inst.hdr->keyBindingArray,
          sizeof(SCMBKeyBindingValue)*inst.hdr->numberKeyBindings,
          &inst.mem);

    // Allocate the SCMBInstancePropertyArray
    _getFreeSpace(
        inst.hdr->propertyArray,
        sizeof(SCMBValue)*inst.hdr->numberProperties,
        &inst.mem);
}


void SCMOInstance::_setCIMInstance(const CIMInstance& cimInstance)
{
    SCMO_RC rc;
    CIMType realType;

    CIMInstanceRep* instRep = cimInstance._rep;

    // Test if the instance has qualifiers.
    // The instance level qualifiers are stored on the associated SCMOClass.
    inst.hdr->flags.includeQualifiers=(instRep->_qualifiers.getCount()>0);

    // To ensure that when converting a CIMInstance to a SCMOInstance
    // and vice versa they have the same property set.
    // If there is no class for this SCMO instance, ingore this since
    // there is no class to work from.  Have to depend on the properties
    if (!noClassForInstance())
    {
        inst.hdr->flags.exportSetOnly = true;
    }

    _setCIMObjectPath(instRep->_reference);

    // Copy all properties
    for (Uint32 i = 0, k = instRep->_properties.size(); i < k; i++)
    {
        CIMPropertyRep* propRep = instRep->_properties[i]._rep;
        // if not already detected that qualifiers are specified and
        // there are qualifers at that property.

        if (!inst.hdr->flags.includeQualifiers &&
            propRep->getQualifierCount() > 0)
        {
            includeQualifiers();
        }
        // if not already detected that class origins are specified and
        // there is a class origin specified at that property.
        if (!inst.hdr->flags.includeClassOrigin &&
            !propRep->_classOrigin.isNull())
        {
            includeClassOrigins();
        }

        // get the property node index for the property

        Uint32 propNode;
        rc = getPropertyNodeIndex(
            (const char*)propRep->_name.getString().getCString(),
            propNode);

        // class-defined property so map
        if (rc == SCMO_OK && !noClassForInstance())
        {
            // The type stored in the class information is set on realType.
            // It must be used in further calls to guaranty consistence.
            rc = inst.hdr->theClass.ptr->_isNodeSameType(
                     propNode,
                     propRep->_value._rep->type,
                     propRep->_value._rep->isArray,
                     realType);

            if (rc == SCMO_OK)
            {
                _setCIMValueAtNodeIndex(
                    propNode,
                    propRep->_value._rep,
                    realType);
            }
            else
            {
                PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL2,
                    "CIMProperty '%s' with type '%s' "
                        "can not be set at SCMOInstance."
                        "It is has not same type '%s' as defined in "
                        "class '%s' of name space '%s'",
                     cimTypeToString(propRep->_value._rep->type),
                    (const char*)propRep->_name.getString().getCString(),
                     cimTypeToString(realType),
                    (const char*)instRep->_reference._rep->
                           _className.getString().getCString(),
                    (const char*)instRep->_reference._rep->
                           _nameSpace.getString().getCString()));
            }

        }
        // user-defined property and property already exists
        else if (rc == SCMO_OK && noClassForInstance())
        {
            // get an existing or new user defined property
            Uint32 node;
            rc = _getUserPropertyNodeIndex(node,
                (const char*)propRep->_name.getString().getCString());

            // KS_FUTURE This would be good one to extend to handle
            // both user and class defined properties.
            _setCIMValueAtNodeIndex(
                propNode,
                propRep->_value._rep,
                propRep->_value._rep->type);
        }
        // User-defined property that has not yet been created.
        else if (rc == SCMO_NOT_FOUND && noClassForInstance())
        {
            _createNewUserDefinedProperty(
                (const char*)propRep->_name.getString().getCString(),
                propRep->_name.getString().size(),
                realType);

            // get the node  for element we just created.

            rc = getPropertyNodeIndex(
                (const char*)propRep->_name.getString().getCString(),
                propNode);

            _setCIMValueAtNodeIndex(
                propNode,
                propRep->_value._rep,
                propRep->_value._rep->type);
        }
        else
        {

            PEG_TRACE((TRC_DISCARDED_DATA,Tracer::LEVEL2,
                "CIMProperty '%s' can not be set at SCMOInstance."
                    "It is not part of class '%s' of name space '%s'",
                (const char*)propRep->_name.getString().getCString(),
                (const char*)instRep->_reference._rep->
                       _className.getString().getCString(),
                (const char*)instRep->_reference._rep->
                       _nameSpace.getString().getCString()));
        }
    }
}
/*
    Gets a SCMO Property from the instance based on the property name
    in the instance. This gets both class based properties and
    user-defined properties
*/
SCMO_RC SCMOInstance::getProperty(
    const char* name,
    CIMType& type,
    const SCMBUnion** pOutVal,
    Boolean& isArray,
    Uint32& size ) const
{
    Uint32 node;
    const char* pname;
    SCMO_RC rc = SCMO_OK;

    *pOutVal = 0;
    isArray = false;
    size = 0;

    // Try to get node from class
    // KS_TODO. This would be simpler if we used the public
    // getPropertyNodeIndex
    rc = inst.hdr->theClass.ptr->_getPropertyNodeIndex(node,name);

//  Error, not in class. Try to get from instance user-defined properties.
    if (rc != SCMO_OK)
    {
        if ((rc == SCMO_NOT_FOUND) && noClassForInstance())
        {
            rc = _getUserPropertyNodeIndex(node, name);
        }
        if (rc != SCMO_OK)
        {
            return rc;
        }
    }
    return  _getPropertyAtNodeIndex(node,&pname,type,pOutVal,isArray,size);
}

/*
    getPropertyAt - This function supports both class-defined and user
    defined properties. It is one of the public interfaces. Note that
    there is no public way to distinguish class-defined and user-defined
    properties.
*/

SCMO_RC SCMOInstance::getPropertyAt(
        Uint32 idx,
        const char** pname,
        CIMType& type,
        const SCMBUnion** pOutVal,
        Boolean& isArray,
        Uint32& size ) const
{
    *pname = 0;
    *pOutVal = 0;
    isArray = false;
    size = 0;

    if (idx >= getPropertyCount())
    {
        return SCMO_INDEX_OUT_OF_BOUND;
    }

    return  _getPropertyAtNodeIndex(idx,pname,type,pOutVal,isArray,size);
}

/*
    Gets the Node index of a property by name. This supports both
    class and user-defined properties.
    Returns the 0-origin node of the property and SCMO_OK or
    SCMO_NOT_FOUND.
    If the PropertyName argument is empty, it returns SCMO_INVALID_PARAMTER.
*/
SCMO_RC SCMOInstance::getPropertyNodeIndex(const char* name, Uint32& node) const
{
    SCMO_RC rc;
    if(name==0)
    {
        return SCMO_INVALID_PARAMETER;
    }

    if (noClassForInstance())
    {
        rc = _getUserPropertyNodeIndex(node, name);
    }
    else
    {
        rc = inst.hdr->theClass.ptr->_getPropertyNodeIndex(node, name);
    }
    return rc;

}
/*
    set a property with name from input parameters.  This will set
    into the normal properties array if the property is in the class
    or into the user-defined array if the property is NOT in the array and
    the noClassForInstance is set.
*/
SCMO_RC SCMOInstance::setPropertyWithOrigin(
    const char* name,
    CIMType theType,
    const SCMBUnion* pInVal,
    Boolean isArray,
    Uint32 size,
    const char* origin)
{
    // In this function no  _copyOnWrite(), it does not change the instance.
    Uint32 node;
    SCMO_RC rc;
    CIMType realType;

    // find the property in the class.
    rc = inst.hdr->theClass.ptr->_getPropertyNodeIndex(node,name);

    // If the instance flags indicate that there was no support class for
    // this instance and noClassForInstance set, handle the property as
    // a user-defined property in the user-defined property section of the
    // instance
    if (noClassForInstance())
    {
        rc = _getUserPropertyNodeIndex(node, name);

        SCMBUserPropertyElement* pElement;
        if (rc == SCMO_OK)
        {
            pElement = _getUserDefinedPropertyElementAt(node);
            if (pElement->value.flags.isSet
                && (pElement->value.valueType != theType))
            {
                return SCMO_TYPE_MISSMATCH;
            }
        }

        else if (rc == SCMO_NOT_FOUND)
        {
            pElement =_createNewUserDefinedProperty(name, strlen(name),
                                              theType);
        }
        else
        {
            return rc;
        }
        // check class origin if set.
        if ((origin != 0) && (pElement->classOrigin.start != 0))
        {
            if (!_equalNoCaseUTF8Strings(pElement->classOrigin,
                                        inst.base,
                                        origin,
                                        strlen(origin)))
            {
                return SCMO_NOT_SAME_ORIGIN;
            }
            else
            {
                // Copy the origin name including the trailing '\0'
                _setBinary(origin,strlen(name)+1,
                    pElement->classOrigin,&inst.mem);
            }
        }
        // Test for same type and array type
        SCMBValue& instVal = pElement->value;
        if (instVal.flags.isSet)
        {
            if (instVal.valueType != theType)
            {
                // Accept a property of type instance also
                // for an CIMTYPE_OBJECT property.
                if (!(theType == CIMTYPE_INSTANCE &&
                      instVal.valueType == CIMTYPE_OBJECT))
                {
                    return SCMO_WRONG_TYPE;
                }
            }

            if (instVal.flags.isArray != isArray)
            {
                return (instVal.flags.isArray?
                        SCMO_NOT_AN_ARRAY : SCMO_IS_AN_ARRAY);
            }
        }

        // KS_FUTURE. This used only once so no need for separate function
        _setPropertyInUserDefinedElement(
            pElement,
            theType,
            pInVal,
            isArray,
            size);

        // test and set classOrigin, etc.
        return SCMO_OK;
    }

    //
    // This is a class-defined instance
    //
    if (rc != SCMO_OK)
    {
        return rc;
    }

    // Is the target type OK ?
    // The type stored in the class information is set to real Type.
    // It must be used in further calls to guaranty consistence.
    rc = inst.hdr->theClass.ptr->_isNodeSameType(node,theType,isArray,realType);
    if (rc != SCMO_OK)
    {
        return rc;
    }

    // check class origin if set.
    if (origin!= 0)
    {
        if(!inst.hdr->theClass.ptr->_isSamePropOrigin(node,origin))
        {
            return SCMO_NOT_SAME_ORIGIN;
        }
    }
    // KS_FUTURE should this be user-defined capable so could delete
    // the user set above.
    _setPropertyAtNodeIndex(node,realType,pInVal,isArray,size);

    return SCMO_OK;
}

// KS_TODO This method never called in OpenPegasus.
// Is there a reason for it or should we delete it.
SCMO_RC SCMOInstance::setPropertyWithNodeIndex(
    Uint32 node,
    CIMType type,
    const SCMBUnion* pInVal,
    Boolean isArray,
    Uint32 size)
{
    // In this function no  _copyOnWrite(), it does not change the instance.

    SCMO_RC rc;
    CIMType realType;

    if (node >= inst.hdr->numberProperties)
    {
        return SCMO_INDEX_OUT_OF_BOUND;
    }

    // Is the traget type OK ?
    // The type stored in the class information is set on realType.
    // It must be used in further calls to guaranty consistence.
    rc = inst.hdr->theClass.ptr->_isNodeSameType(node,type,isArray,realType);
    if (rc != SCMO_OK)
    {
        return rc;
    }

    _setPropertyAtNodeIndex(node,realType,pInVal,isArray,size);

    return SCMO_OK;
}

// KS_TODO this one NOT user-defined capable now
void SCMOInstance::_setPropertyAtNodeIndex(
    Uint32 node,
    CIMType type,
    const SCMBUnion* pInVal,
    Boolean isArray,
    Uint32 size)
{

    _copyOnWrite();

    SCMBValue* theInstPropNodeArray =
        (SCMBValue*)&(inst.base[inst.hdr->propertyArray.start]);


    theInstPropNodeArray[node].flags.isSet=true;
    theInstPropNodeArray[node].valueType=type;
    theInstPropNodeArray[node].flags.isArray=isArray;
    if (isArray)
    {
        theInstPropNodeArray[node].valueArraySize=size;
    }

    if (pInVal==0)
    {
        theInstPropNodeArray[node].flags.isNull=true;
    }
    else
    {
        theInstPropNodeArray[node].flags.isNull=false;
        _setSCMBUnion(
            pInVal,
            type,
            isArray,
            size,
            theInstPropNodeArray[node].value);
    }
}
/*
    This function can cause a reallocation.
*/
void SCMOInstance::_setSCMBUnion(
    const SCMBUnion* pInVal,
    CIMType type,
    Boolean isArray,
    Uint32 size,
    SCMBUnion & u)
{
    switch (type)
    {
    case CIMTYPE_BOOLEAN:
    case CIMTYPE_UINT8:
    case CIMTYPE_SINT8:
    case CIMTYPE_UINT16:
    case CIMTYPE_SINT16:
    case CIMTYPE_UINT32:
    case CIMTYPE_SINT32:
    case CIMTYPE_UINT64:
    case CIMTYPE_SINT64:
    case CIMTYPE_REAL32:
    case CIMTYPE_REAL64:
    case CIMTYPE_CHAR16:
        {
            if (isArray)
            {
                _setBinary(pInVal,size*sizeof(SCMBUnion),
                           u.arrayValue,
                           &inst.mem );
            }
            else
            {
                memcpy(&u,pInVal,sizeof(SCMBUnion));
                u.simple.hasValue=true;
            }
            break;
        }
    case CIMTYPE_DATETIME:
        {
            if (isArray)
            {
                _setBinary(pInVal,size*sizeof(SCMBUnion),
                           u.arrayValue,
                           &inst.mem );
            }
            else
            {
                memcpy(&u,pInVal,sizeof(SCMBUnion));

            }
            break;
        }
    case CIMTYPE_STRING:
        {
            if (isArray)
            {
                SCMBUnion* ptr;
                Uint64 startPtr;

                startPtr = _getFreeSpace(
                    u.arrayValue,
                    size*sizeof(SCMBUnion),
                    &inst.mem);

                for (Uint32 i = 0; i < size; i++)
                {
                    ptr = (SCMBUnion*)&(inst.base[startPtr]);
                    // Copy the sting including the trailing '\0'
                    _setBinary(
                        pInVal[i].extString.pchar,
                        pInVal[i].extString.length+1,
                        ptr[i].stringValue,
                        &inst.mem );
                }
            }
            else
            {
                // Copy the sting including the trailing '\0'
                _setBinary(
                    pInVal->extString.pchar,
                    pInVal->extString.length+1,
                    u.stringValue,
                    &inst.mem );
            }
            break;
        }

    case CIMTYPE_REFERENCE:
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        {
            if(isArray)
            {
                SCMBUnion* ptr;
                Uint64 startPtr;

                // if the array was previously set, delete the references !
                _deleteArrayExtReference(u.arrayValue,&inst.mem);

                // get new array
                startPtr = _getFreeSpace(
                    u.arrayValue,
                    size*sizeof(SCMBUnion),
                    &inst.mem);

                ptr = (SCMBUnion*)&(inst.base[startPtr]);

                for (Uint32 i = 0 ; i < size ; i++)
                {
                    if(pInVal[i].extRefPtr)
                    {
                        ptr[i].extRefPtr=
                            new SCMOInstance(*(pInVal[i].extRefPtr));

                        // This function can cause a reallocation !
                        // Pointers can be invalid after the call.
                        _setExtRefIndex(&(ptr[i]),&inst.mem);
                    }
                    else
                    {
                        ptr[i].extRefPtr = 0;
                    }
                }

            }
            else
            {
                if(0 != u.extRefPtr)
                {
                    delete u.extRefPtr;
                }

                if(pInVal->extRefPtr)
                {
                    u.extRefPtr = new SCMOInstance(*(pInVal->extRefPtr));
                    // This function can cause a reallocation !
                    // Pointers can be invalid after the call.
                    _setExtRefIndex(&u,&inst.mem);

                }
                else
                {
                    u.extRefPtr = 0;
                }
            }
            break;
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }
}

void SCMOInstance::_setUnionArrayValue(
    Uint64 start,
    SCMBMgmt_Header** pmem,
    CIMType type,
    Uint32& n,
    Uint64 startNS,
    Uint32 sizeNS,
    Union& u)
{
    SCMBUnion* scmoUnion = (SCMBUnion*)&(((char*)*pmem)[start]);
    SCMBUnion* ptargetUnion;
    Uint64 arrayStart;
    Uint32 loop;

    switch (type)
    {
    case CIMTYPE_BOOLEAN:
        {
            Array<Boolean> *x = reinterpret_cast<Array<Boolean>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Boolean> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.bin  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_UINT8:
        {
            Array<Uint8> *x = reinterpret_cast<Array<Uint8>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Uint8> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.u8  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_SINT8:
        {
            Array<Sint8> *x = reinterpret_cast<Array<Sint8>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Sint8> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.s8  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_UINT16:
        {
            Array<Uint16> *x = reinterpret_cast<Array<Uint16>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Uint16> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.u16  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_SINT16:
        {
            Array<Sint16> *x = reinterpret_cast<Array<Sint16>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Sint16> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.s16  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_UINT32:
        {
            Array<Uint32> *x = reinterpret_cast<Array<Uint32>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Uint32> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.u32  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_SINT32:
        {
            Array<Sint32> *x = reinterpret_cast<Array<Sint32>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Sint32> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.s32  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_UINT64:
        {
            Array<Uint64> *x = reinterpret_cast<Array<Uint64>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Uint64> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.u64  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_SINT64:
        {
            Array<Sint64> *x = reinterpret_cast<Array<Sint64>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Sint64> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.s64  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_REAL32:
        {
            Array<Real32> *x = reinterpret_cast<Array<Real32>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Real32> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.r32  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_REAL64:
        {
            Array<Real64> *x = reinterpret_cast<Array<Real64>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Real64> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.r64  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_CHAR16:
        {
            Array<Char16> *x = reinterpret_cast<Array<Char16>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();
            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<Char16> iterator(*x);

            ptargetUnion = (SCMBUnion*)&(((char*)*pmem)[arrayStart]);
            for (Uint32 i = 0; i < loop; i++)
            {
                ptargetUnion[i].simple.val.c16  = iterator[i];
                ptargetUnion[i].simple.hasValue = true;
            }
            break;
        }

    case CIMTYPE_STRING:
        {
            Array<String> *x = reinterpret_cast<Array<String>*>(&u);
            // n can be invalid after re-allocation in _getFreeSpace !
            loop = n = x->size();

            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<String> iterator(*x);

            for (Uint32 i = 0; i < loop ; i++)
            {
                // the pointer has to be set eache loop,
                // because a reallocation may take place.
                ptargetUnion = (SCMBUnion*)(&((char*)*pmem)[arrayStart]);
                _setString( iterator[i],ptargetUnion[i].stringValue, pmem );
            }

            break;
        }

    case CIMTYPE_DATETIME:
        {
            Array<CIMDateTime> *x = reinterpret_cast<Array<CIMDateTime>*>(&u);
            // n can be invalid after reallocation in _getFreeSpace !
            loop = n = x->size();

            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<CIMDateTime> iterator(*x);

            ptargetUnion = (SCMBUnion*)(&((char*)*pmem)[arrayStart]);

            for (Uint32 i = 0; i < loop ; i++)
            {
                memcpy(
                    &(ptargetUnion[i].dateTimeValue),
                    iterator[i]._rep,
                    sizeof(SCMBDateTime));
            }
            break;
        }

    case CIMTYPE_REFERENCE:
        {
            Array<CIMObjectPath> *x =
                reinterpret_cast<Array<CIMObjectPath>*>(&u);

            // if the array was previously set, delete the references !
            _deleteArrayExtReference(scmoUnion->arrayValue,pmem);

            // n can be invalid after reallocation in _getFreeSpace !
            loop = n = x->size();

            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<CIMObjectPath> iterator(*x);

            ptargetUnion = (SCMBUnion*)(&((char*)*pmem)[arrayStart]);

            for (Uint32 i = 0; i < loop ; i++)
            {
                ptargetUnion[i].extRefPtr =
                    new SCMOInstance(
                        iterator[i],
                        &(((const char*)*pmem)[startNS]),
                        sizeNS-1);
                // Was the conversion successful?
                if (ptargetUnion[i].extRefPtr->isEmpty())
                {
                    // N0, delete the SCMOInstance.
                    delete ptargetUnion[i].extRefPtr;
                    ptargetUnion[i].extRefPtr = 0;
                }
                else
                {
                    _setExtRefIndex(&(ptargetUnion[i]),pmem);
                }
            }

            break;
        }
    case CIMTYPE_OBJECT:
        {
            Array<CIMObject> *x =
                reinterpret_cast<Array<CIMObject>*>(&u);

            // if the array was previously set, delete the references !
            _deleteArrayExtReference(scmoUnion->arrayValue,pmem);

            // n can be invalid after reallocation in _getFreeSpace !
            loop = n = x->size();

            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<CIMObject> iterator(*x);

            ptargetUnion = (SCMBUnion*)(&((char*)*pmem)[arrayStart]);

            for (Uint32 i = 0; i < loop ; i++)
            {
                if (iterator[i].isUninitialized())
                {
                    // the Object was empty.
                    ptargetUnion[i].extRefPtr = 0;
                }
                else
                {
                    if (iterator[i].isClass())
                    {
                        CIMClass cimClass(iterator[i]);

                        ptargetUnion[i].extRefPtr =
                            new SCMOInstance(
                                cimClass,
                                (&((const char*)*pmem)[startNS]));
                        // marke as class only !
                        ptargetUnion[i].extRefPtr->
                            inst.hdr->flags.isClassOnly=true;

                        // This function can cause a reallocation !
                        // Pointers can be invalid after the call.
                        _setExtRefIndex(&(ptargetUnion[i]),pmem);
                    }
                    else
                    {
                        CIMInstance theInst(iterator[i]);

                        ptargetUnion[i].extRefPtr =
                            new SCMOInstance(
                                theInst,
                                &(((const char*)*pmem)[startNS]),
                                sizeNS-1);
                         // Was the conversion successful?
                         if (ptargetUnion[i].extRefPtr->isEmpty())
                         {
                             // N0, delete the SCMOInstance.
                             delete ptargetUnion[i].extRefPtr;
                             ptargetUnion[i].extRefPtr = 0;
                         }
                         else
                         {
                             // This function can cause a reallocation !
                             // Pointers can be invalid after the call.
                             _setExtRefIndex(&(ptargetUnion[i]),pmem);
                         }
                    }
                }
            }

            break;
        }
    case CIMTYPE_INSTANCE:
        {
            Array<CIMInstance> *x =
                reinterpret_cast<Array<CIMInstance>*>(&u);

            // if the array was previously set, delete the references !
            _deleteArrayExtReference(scmoUnion->arrayValue,pmem);

            // n can be invalid after reallocation in _getFreeSpace !
            loop = n = x->size();

            arrayStart = _getFreeSpace(
                scmoUnion->arrayValue,
                loop*sizeof(SCMBUnion),
                pmem);

            ConstArrayIterator<CIMInstance> iterator(*x);

            ptargetUnion = (SCMBUnion*)(&((char*)*pmem)[arrayStart]);

            for (Uint32 i = 0; i < loop ; i++)
            {
                if (iterator[i].isUninitialized())
                {
                    // the Instance was empty.
                    ptargetUnion[i].extRefPtr = 0;
                }
                else
                {
                    ptargetUnion[i].extRefPtr =
                        new SCMOInstance(
                            iterator[i],
                            &(((const char*)*pmem)[startNS]),
                            sizeNS-1);
                    // Was the conversion successful?
                    if (ptargetUnion[i].extRefPtr->isEmpty())
                    {
                        // N0, delete the SCMOInstance.
                        delete ptargetUnion[i].extRefPtr;
                        ptargetUnion[i].extRefPtr = 0;
                    }
                    else
                    {
                        // This function can cause a reallocation !
                        // Pointers can be invalid after the call.
                        _setExtRefIndex(&(ptargetUnion[i]),pmem);
                    }

                }
            }

            break;
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }
}


void SCMOInstance::_setUnionValue(
    Uint64 start,
    SCMBMgmt_Header** pmem,
    CIMType type,
    Uint64 startNS,
    Uint32 sizeNS,
    Union& u)
{
    SCMBUnion* scmoUnion = (SCMBUnion*)&(((char*)*pmem)[start]);

    switch (type)
    {
    case CIMTYPE_BOOLEAN:
        {
            scmoUnion->simple.val.bin = u._booleanValue;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_UINT8:
        {
            scmoUnion->simple.val.u8 = u._uint8Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_SINT8:
        {
            scmoUnion->simple.val.s8 = u._sint8Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_UINT16:
        {
            scmoUnion->simple.val.u16 = u._uint16Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_SINT16:
        {
            scmoUnion->simple.val.s16 = u._sint16Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_UINT32:
        {
            scmoUnion->simple.val.u32 = u._uint32Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_SINT32:
        {
            scmoUnion->simple.val.s32 = u._sint32Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_UINT64:
        {
            scmoUnion->simple.val.u64 = u._uint64Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_SINT64:
        {
            scmoUnion->simple.val.s64 = u._sint64Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_REAL32:
        {
            scmoUnion->simple.val.r32 = u._real32Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_REAL64:
        {
            scmoUnion->simple.val.r64 = u._real64Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_CHAR16:
        {
            scmoUnion->simple.val.c16 = u._char16Value;
            scmoUnion->simple.hasValue=true;
            break;
        }

    case CIMTYPE_STRING:
        {
            // KS_TODO why do this when there is a _setChar function
            // that directly uses Strings??? low priority question but
            // this would seem less efficient that _setString with the
            // strlen function in it.
            CString cstr = ((String*)((void*)&u))->getCString();
            const char *cptr = (const char*)cstr;
            _setBinary(
                cptr,
                strlen(cptr) + 1,
                scmoUnion->stringValue,
                pmem );
            break;
        }

    case CIMTYPE_DATETIME:
        {
            memcpy(
                &scmoUnion->dateTimeValue,
                (*((CIMDateTime*)((void*)&u)))._rep,
                sizeof(SCMBDateTime));
            break;
        }

    case CIMTYPE_REFERENCE:
        {
            if (0 != scmoUnion->extRefPtr)
            {
                delete scmoUnion->extRefPtr;
                scmoUnion->extRefPtr = 0;
            }

            if (0 == u._referenceValue)
            {
              scmoUnion->extRefPtr = 0;
              return;
            }

            CIMObjectPath* theCIMObj =
                (CIMObjectPath*)((void*)&u._referenceValue);

            scmoUnion->extRefPtr =
                new SCMOInstance(
                    *theCIMObj,
                    &(((const char*)*pmem)[startNS]),
                    sizeNS-1);

            // Was the conversion successful?
            if (scmoUnion->extRefPtr->isEmpty())
            {
                // N0, delete the SCMOInstance.
                delete scmoUnion->extRefPtr;
                scmoUnion->extRefPtr = 0;
            }
            else
            {
                // This function can cause a reallocation !
                // Pointers can be invalid after the call.
                _setExtRefIndex(scmoUnion,pmem);
            }

            break;
        }
    case CIMTYPE_OBJECT:
        {
            if (0 != scmoUnion->extRefPtr)
            {
                delete scmoUnion->extRefPtr;
                scmoUnion->extRefPtr = 0;
            }

            if (0 == u._referenceValue)
            {
              scmoUnion->extRefPtr=0;
              return;
            }

            CIMObject* theCIMObject =(CIMObject*)((void*)&u._objectValue);

            if (theCIMObject->isUninitialized())
            {
                // the Object was empty.
                scmoUnion->extRefPtr = 0;
            }
            else
            {
                if (theCIMObject->isClass())
                {
                    CIMClass cimClass(*theCIMObject);

                    scmoUnion->extRefPtr =
                        new SCMOInstance(
                            cimClass,
                            (&((const char*)*pmem)[startNS]));
                    // mark as class only !
                    scmoUnion->extRefPtr->inst.hdr->flags.isClassOnly=true;

                    // This function can cause a reallocation !
                    // Pointers can be invalid after the call.
                    _setExtRefIndex(scmoUnion,pmem);
                }
                else
                {
                    CIMInstance theCIMInst(*theCIMObject);

                    scmoUnion->extRefPtr =
                        new SCMOInstance(
                            theCIMInst,
                            &(((const char*)*pmem)[startNS]),
                            sizeNS);

                     // Was the conversion successful?
                    if (scmoUnion->extRefPtr->isEmpty())
                     {
                         // N0, delete the SCMOInstance.
                         delete scmoUnion->extRefPtr;
                         scmoUnion->extRefPtr = 0;
                     }
                     else
                     {
                         // This function can cause a reallocation !
                         // Pointers can be invalid after the call.
                         _setExtRefIndex(scmoUnion,pmem);
                     }
                }
            }
            break;
        }
    case CIMTYPE_INSTANCE:
        {
            if (0 != scmoUnion->extRefPtr)
            {
                delete scmoUnion->extRefPtr;
                scmoUnion->extRefPtr = 0;
            }

            if (0 == u._referenceValue)
            {
              scmoUnion->extRefPtr=0;
              return;
            }

            CIMInstance* theCIMInst =
                (CIMInstance*)((void*)&u._instanceValue);

            if (theCIMInst->isUninitialized())
            {
                // the Instance was empty.
                scmoUnion->extRefPtr = 0;
            }
            else
            {
                scmoUnion->extRefPtr =
                    new SCMOInstance(
                        *theCIMInst,
                        &(((const char*)*pmem)[startNS]),
                        sizeNS-1);

                 // Was the conversion successful?
                 if (scmoUnion->extRefPtr->isEmpty())
                 {
                     // N0, delete the SCMOInstance.
                     delete scmoUnion->extRefPtr;
                     scmoUnion->extRefPtr = 0;
                 }
                 else
                 {
                     // This function can cause a reallocation !
                     // Pointers can be invalid after the call.
                     _setExtRefIndex(scmoUnion,pmem);
                 }
            }
            break;
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }
}

/*
    Gets the property at the defined node index. This covers both
    normal and user defined properties.
*/
SCMO_RC SCMOInstance::_getPropertyAtNodeIndex(
        Uint32 node,
        const char** pname,
        CIMType& type,
        const SCMBUnion** pvalue,
        Boolean& isArray,
        Uint32& size ) const
{
    if (_isClassDefinedProperty(node))
    {
        SCMBValue* theInstPropNodeArray =
            (SCMBValue*)&inst.base[inst.hdr->propertyArray.start];

        // create a pointer to property node array of the class.
        Uint64 idx =
            inst.hdr->theClass.ptr->cls.hdr->propertySet.nodeArray.start;

        SCMBClassPropertyNode* theClassPropNodeArray =
            (SCMBClassPropertyNode*)&(inst.hdr->theClass.ptr->cls.base)[idx];

        // the property name is always from the class.
        // return the absolute pointer to the property name,
        // the caller has to copy the name!
        *pname=_getCharString(
            theClassPropNodeArray[node].theProperty.name,
            inst.hdr->theClass.ptr->cls.base);

        // the property was set by the provider.
        if (theInstPropNodeArray[node].flags.isSet)
        {
            type = theInstPropNodeArray[node].valueType;
            isArray = theInstPropNodeArray[node].flags.isArray;
            if (isArray)
            {
                size = theInstPropNodeArray[node].valueArraySize;
            }

            if (theInstPropNodeArray[node].flags.isNull)
            {
                return SCMO_NULL_VALUE;
            }

            // calculate the relative index for the value.
            Uint64 start =
                (const char*)&(theInstPropNodeArray[node].value) - inst.base;

            // the caller has to copy the value !
            *pvalue = _resolveSCMBUnion(type,isArray,size,start,inst.base);

            return SCMO_OK;
        }

        // the get the defaults out of the class.
        type = theClassPropNodeArray[node].theProperty.defaultValue.valueType;
        isArray =
            theClassPropNodeArray[node].theProperty.defaultValue.flags.isArray;
        if (isArray)
        {
            size = theClassPropNodeArray[node].
                       theProperty.defaultValue.valueArraySize;
        }

        if (theClassPropNodeArray[node].theProperty.defaultValue.flags.isNull)
        {
            return SCMO_NULL_VALUE;
        }

        // calculate the relative start address of the value in the class
        Uint64 start =
            (const char*)
                &(theClassPropNodeArray[node].theProperty.defaultValue.value) -
            (inst.hdr->theClass.ptr->cls.base);

        *pvalue = _resolveSCMBUnion(
            type,
            isArray,
            size,
            start,
            (inst.hdr->theClass.ptr->cls.base));
    }

    // else treated as user-defined property
    else
    {
        SCMBUserPropertyElement* pElement =
            _getUserDefinedPropertyElementAt(node);

        if (pElement == 0)
        {
            return SCMO_INDEX_OUT_OF_BOUND;
        }

        SCMBValue& instValue = pElement->value;

        *pname = _getCharString(pElement->name,inst.base);

        if (instValue.flags.isSet)
        {
            type = instValue.valueType;

            isArray = instValue.flags.isArray;
            if (isArray)
            {
                size = instValue.valueArraySize;
            }

            if (instValue.flags.isNull)
            {
                return SCMO_NULL_VALUE;
            }
        }
        else
        {
            // KS_TODO sort out what we do if user-defined element not
            // initialized.
            // Should never happen
            return SCMO_NULL_VALUE;
        }

        // resolve start for the Union.
        // KS_TODO we should be able to combine following and
        // the same for normal properties
        Uint64 start =
                (const char*)&(instValue.value) - inst.base;

        *pvalue = _resolveSCMBUnion(type,isArray,size,start,inst.base);
    }
    return SCMO_OK;

}

SCMOInstance SCMOInstance::clone(Boolean objectPathOnly) const
{
    if (objectPathOnly)
    {
        // Create a new, empty SCMOInstance
        SCMOInstance newInst(*(this->inst.hdr->theClass.ptr));

        // Copy the host name to the new instance-
        _setBinary(
            _resolveDataPtr(this->inst.hdr->hostName,this->inst.base),
            this->inst.hdr->hostName.size,
            newInst.inst.hdr->hostName,
            &newInst.inst.mem);

        newInst.inst.hdr->flags.isCompromised =
            this->inst.hdr->flags.isCompromised;

        // If the instance contains a user set class and/or name space name
        if (this->inst.hdr->flags.isCompromised)
        {
            // Copy the class name to the new instance-
            _setBinary(
                _resolveDataPtr(this->inst.hdr->instClassName,this->inst.base),
                this->inst.hdr->instClassName.size,
                newInst.inst.hdr->instClassName,
                &newInst.inst.mem);

            // Copy the name space name to tha new instance-
            _setBinary(
                _resolveDataPtr(this->inst.hdr->instNameSpace,this->inst.base),
                this->inst.hdr->instNameSpace.size,
                newInst.inst.hdr->instNameSpace,
                &newInst.inst.mem);
        }

        // Copy the key bindings to that new instance.
        this->_copyKeyBindings(newInst);

        return newInst;
    }

    SCMOInstance newInst;
    newInst.inst.base = inst.base;
    newInst._clone();

    return newInst;
}

void SCMOInstance::_clone()
{
    char* newBase;
    newBase = (char*)malloc((size_t)inst.mem->totalSize);
    if (0 == newBase )
    {
        throw PEGASUS_STD(bad_alloc)();
    }

    memcpy( newBase,inst.base,(size_t)inst.mem->totalSize);

    // make new new memory block to mine.
    inst.base = newBase;
    // reset the refcounter of this instance
    inst.hdr->refCount = 1;
    // keep the ref counter of the class correct !
    inst.hdr->theClass.ptr = new SCMOClass(*(inst.hdr->theClass.ptr));
    // keep the ref count for external references
    _copyExternalReferences();
}

void SCMOInstance::_copyKeyBindings(SCMOInstance& targetInst) const
{
    Uint32 noBindings = inst.hdr->numberKeyBindings;

    SCMBKeyBindingValue* sourceArray =
        (SCMBKeyBindingValue*)&(inst.base[inst.hdr->keyBindingArray.start]);

    // Address the class keybinding information
    const SCMBClass_Main* clshdr = inst.hdr->theClass.ptr->cls.hdr;
    const char * clsbase = inst.hdr->theClass.ptr->cls.base;
    SCMBKeyBindingNode* scmoClassArray =
        (SCMBKeyBindingNode*)&(clsbase[clshdr->keyBindingSet.nodeArray.start]);

    SCMBKeyBindingValue* targetArray;

    for (Uint32 i = 0; i < noBindings; i++)
    {
        // has to be set every time, because of reallocation.
        targetArray=(SCMBKeyBindingValue*)&(targetInst.inst.base)
                             [targetInst.inst.hdr->keyBindingArray.start];
        if(sourceArray[i].isSet)
        {
            // this has to be done on the target instance to keep constantness.
            targetInst._setKeyBindingFromSCMBUnion(
                scmoClassArray[i].type,
                sourceArray[i].data,
                inst.base,
                targetArray[i]);
        }
    }

    // Are there user defined key bindings ? If so, copy them
    if (0 != inst.hdr->numberUserKeyBindings)
    {
        SCMBUserKeyBindingElement* theUserDefKBElement =
            (SCMBUserKeyBindingElement*)
                 &(inst.base[inst.hdr->userKeyBindingElement.start]);

        for(Uint32 i = 0; i < inst.hdr->numberUserKeyBindings; i++)
        {
            if (theUserDefKBElement->value.isSet)
            {
                targetInst._setUserDefinedKeyBinding(*theUserDefKBElement,
                                                        inst.base);
            }

            theUserDefKBElement =
                (SCMBUserKeyBindingElement*)
                     &(inst.base[theUserDefKBElement->nextElement.start]);
        } // for all user def. key bindings.
    }
}

/*
    Set theInsertElement into new or existing user-defined key
    binding.
*/
void SCMOInstance::_setUserDefinedKeyBinding(
        SCMBUserKeyBindingElement& theInsertElement,
        char* elementBase)
{
    SCMBUserKeyBindingElement* ptrNewElement;

    // get an exsiting or new user defined key binding
    ptrNewElement = _getUserDefinedKeyBinding(
        _getCharString(theInsertElement.name,elementBase),
        // length is without the trailing '\0'
        theInsertElement.name.size-1,
        theInsertElement.type);

    // Copy the data
    _setKeyBindingFromSCMBUnion(
                theInsertElement.type,
                theInsertElement.value.data,
                elementBase,
                ptrNewElement->value);
}


SCMBUserKeyBindingElement* SCMOInstance::_getUserDefinedKeyBindingAt(
    Uint32 index ) const
{
    // Get the start element
    SCMBUserKeyBindingElement *ptrNewElement =
        (SCMBUserKeyBindingElement*)
             &(inst.base[inst.hdr->userKeyBindingElement.start]);

    // Calculate the index within the user defined key bindings

    index = index - inst.hdr->numberKeyBindings;

    // traverse through the user defined key binding nodes.
    for (Uint32 i = 0; i < index; i ++)
    {
        PEGASUS_ASSERT(ptrNewElement->nextElement.start != 0);
        ptrNewElement = (SCMBUserKeyBindingElement*)
              &(inst.base[ptrNewElement->nextElement.start]);
    }

    return ptrNewElement;
}

/*
    If the user-defined keybinding already exists, return a pointer to
    that element. Otherwise allocate a new element and return pointer
    to the new element
*/
SCMBUserKeyBindingElement* SCMOInstance::_getUserDefinedKeyBinding(
    const char* name,
    Uint32 nameLen,
    CIMType type)
{
    SCMBDataPtr newElement;
    SCMBUserKeyBindingElement* ptrNewElement;
    Uint32 node;

    // is the key binding already stored as user defind in the instance ?
    if (SCMO_OK == _getUserKeyBindingNodeIndex(node,name))
    {
       ptrNewElement = _getUserDefinedKeyBindingAt(node);
    }
    else // Not found, create a new user defined key binding.
    {
        _getFreeSpace(newElement,
                      sizeof(SCMBUserKeyBindingElement),
                      &inst.mem);

        ptrNewElement =
            (SCMBUserKeyBindingElement*)&(inst.base[newElement.start]);

        // link new first user defined key binding element into chain:
        // - Adding the start point of user key binding element chain
        //   to the next element of the new element.
        ptrNewElement->nextElement.start =
            inst.hdr->userKeyBindingElement.start;
        ptrNewElement->nextElement.size =
            inst.hdr->userKeyBindingElement.size;
        // - Adding the the new element
        //   to the  start point of user key binding element chain
        inst.hdr->userKeyBindingElement.start = newElement.start;
        inst.hdr->userKeyBindingElement.size = newElement.size;
        // Adjust the counter of user defined key bindings.
        inst.hdr->numberUserKeyBindings++;

        // Copy the type
        ptrNewElement->type = type;
        ptrNewElement->value.isSet=false;

        // Copy the key binding name including the trailing '\0'
        _setBinary(name,nameLen+1,ptrNewElement->name,&inst.mem);

        // reset the pointer. May the instance was reallocated.
        ptrNewElement =
            (SCMBUserKeyBindingElement*)&(inst.base[newElement.start]);
    }
    return ptrNewElement;
}

/*
    Create a new UserDefined Property Element, link it into the User-defined
    chain, and populate it with name, type, and isArray.  Set isSet to ftrue.
    Returns pointer to the new element
*/
SCMBUserPropertyElement* SCMOInstance::_createNewUserDefinedProperty(
    const char * name,
    Uint32 nameLen,
    CIMType theType)
{
    SCMBDataPtr newElement;
    _getFreeSpace(newElement,
                  sizeof(SCMBUserPropertyElement),
                  &inst.mem);

    SCMBUserPropertyElement* pElement =
        (SCMBUserPropertyElement*)&(inst.base[newElement.start]);

    // link new first user defined property element into chain:
    // - Add the start point of user property element chain
    //   to the next element of the new element.
    pElement->nextElement.start =
        inst.hdr->userPropertyElement.start;
    pElement->nextElement.size =
        inst.hdr->userPropertyElement.size;
    // Add the the new element to the  start point of user property
    // element chain
    inst.hdr->userPropertyElement.start = newElement.start;
    inst.hdr->userPropertyElement.size = newElement.size;
    // Adjust the counter of user defined key bindings.
    inst.hdr->numberUserProperties++;

    // Copy the type
    pElement->value.valueType = theType;
    pElement->value.flags.isSet=false;

    // Copy the property name including the trailing '\0'
    _setBinary(name,nameLen+1,pElement->name,&inst.mem);

    // reset the pointer. Maybe the instance was reallocated.
    pElement =
        (SCMBUserPropertyElement*)&(inst.base[newElement.start]);
    return pElement;
}
/*
    set either a new or existing UserDefinedProperty with the
    InsertElement provided
*/

void SCMOInstance::_setPropertyInUserDefinedElement(
        SCMBUserPropertyElement* pElement,
        CIMType theType,
        const SCMBUnion* pInVal,
        Boolean isArray,
        Uint32 size)
{
    _copyOnWrite();

    SCMBValue& instVal = pElement->value;

    // Set data into element
    instVal.flags.isSet=true;
    instVal.valueType=theType;
    instVal.flags.isArray= isArray;
    pElement->classOrigin.start = 0;

    if (isArray)
    {
        instVal.valueArraySize = size;
    }

    if (pInVal==0)
    {
        instVal.flags.isNull=true;
    }
    else
    {
        instVal.flags.isNull=false;
        _setSCMBUnion(
            pInVal,
            theType,
            isArray,
            size,
            instVal.value);
    }

}

/*
    Get the user-defined propertyElement defined by index arguement
    Returns the Element.
    KS_TODO what do we do if it runs through the end of the list.
    Right now it is an assert so somewhere a check is needed.
    Probablly the callers
*/
SCMBUserPropertyElement* SCMOInstance::_getUserDefinedPropertyElementAt(
    Uint32 index ) const
{
    // Get the start element
    SCMBUserPropertyElement* pElement =
        (SCMBUserPropertyElement*)
             &(inst.base[inst.hdr->userPropertyElement.start]);

    // calculate the index within the user defined properties
    // based on numberProperties in instance. This should
    // be verified against numberUserProperties

    index = index - inst.hdr->numberProperties;

    // traverse  the user defined property nodes.
    for (Uint32 i = 0; i < index; i ++)
    {
        PEGASUS_ASSERT(pElement->nextElement.start != 0);
        pElement = (SCMBUserPropertyElement*)
              &(inst.base[pElement->nextElement.start]);
        // KS_TODO should be able to compute this error from number
        // properties
        // return NULL if this index not found.
        if (pElement == 0)
        {
            // This error should never occur.
            PEGASUS_ASSERT(false);
            return 0;
        }
    }
    return pElement;
}

SCMBUnion * SCMOInstance::_resolveSCMBUnion(
    CIMType type,
    Boolean isArray,
    Uint32 size,
    Uint64 start,
    char* base) const
{
    SCMBUnion* u = (SCMBUnion*)&(base[start]);

    SCMBUnion* av = 0;

    if (isArray)
    {
        if (size == 0)
        {
            return 0;
        }
        av = (SCMBUnion*)&(base[u->arrayValue.start]);
    }

    switch (type)
    {
    case CIMTYPE_BOOLEAN:
    case CIMTYPE_UINT8:
    case CIMTYPE_SINT8:
    case CIMTYPE_UINT16:
    case CIMTYPE_SINT16:
    case CIMTYPE_UINT32:
    case CIMTYPE_SINT32:
    case CIMTYPE_UINT64:
    case CIMTYPE_SINT64:
    case CIMTYPE_REAL32:
    case CIMTYPE_REAL64:
    case CIMTYPE_CHAR16:
    case CIMTYPE_DATETIME:
    case CIMTYPE_REFERENCE:
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        {
            if(isArray)
            {
                return (av);
            }
            else
            {
                return(u);
            }
        }

    case CIMTYPE_STRING:
        {
            SCMBUnion *ptr;

            if (isArray)
            {

                ptr = (SCMBUnion*)malloc(size*sizeof(SCMBUnion));
                if (ptr == 0 )
                {
                    throw PEGASUS_STD(bad_alloc)();
                }

                for(Uint32 i = 0; i < size; i++)
                {
                    // resolv relative pointer to absolute pointer
                    ptr[i].extString.pchar =
                        (char*)_getCharString(av[i].stringValue,base);
                    // lenght with out the trailing /0 !
                    ptr[i].extString.length = av[i].stringValue.size-1;
                }
            }
            else
            {
                ptr = (SCMBUnion*)malloc(sizeof(SCMBUnion));
                ptr->extString.pchar =
                    (char*)_getCharString(u->stringValue,base);
                // lenght with out the trailing /0 !
                ptr->extString.length = u->stringValue.size-1;
            }

            return(ptr);
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }
    return 0;
}

void SCMOInstance::clearKeyBindings()
{
    _copyOnWrite();

    // First destroy all external references in the key bindings
    _destroyExternalKeyBindings();

    // reset user keybindings
    inst.hdr->numberUserKeyBindings = 0;
    inst.hdr->userKeyBindingElement.start = 0;
    inst.hdr->userKeyBindingElement.size = 0;

    // Allocate a clean the SCMOInstanceKeyBindingArray
    _getFreeSpace(
          inst.hdr->keyBindingArray,
          sizeof(SCMBKeyBindingValue)*inst.hdr->numberKeyBindings,
          &inst.mem);

    // Clear the keybindings after the allocation. Setting the keybindings
    // later causes this value to be reinitialized.
    inst.hdr->numberKeyBindings = 0;

    markAsCompromised();
}

Uint32 SCMOInstance::getKeyBindingCount() const
{
    // count of class keys + user definded keys
    return(inst.hdr->numberKeyBindings+
           inst.hdr->numberUserKeyBindings);
}


SCMO_RC SCMOInstance::getKeyBindingAt(
        Uint32 node,
        const char** pname,
        CIMType& type,
        const SCMBUnion** pvalue) const
{
    SCMO_RC rc;
    const SCMBUnion* pdata=0;
    Uint32 pnameLen=0;

    *pname = 0;
    *pvalue = 0;

    // count of class keys + user definded keys
    if (node >= (inst.hdr->numberKeyBindings+
                 inst.hdr->numberUserKeyBindings))
    {
        return SCMO_INDEX_OUT_OF_BOUND;
    }

    rc = _getKeyBindingDataAtNodeIndex(node,pname,pnameLen,type,&pdata);
    if (rc != SCMO_OK)
    {
        return rc;
    }

    *pvalue = _resolveSCMBUnion(
        type,
        false,  // A key binding can never be an array.
        0,
        (char*)pdata-inst.base,
        inst.base);

    return SCMO_OK;

}

SCMO_RC SCMOInstance::getKeyBinding(
    const char* name,
    CIMType& type,
    const SCMBUnion** pvalue) const
{
    SCMO_RC rc;
    Uint32 node;
    const char* pname=0;
    const SCMBUnion* pdata=0;
    Uint32 pnameLen=0;

    *pvalue = 0;

    rc = inst.hdr->theClass.ptr->_getKeyBindingNodeIndex(node,name);
    if (rc != SCMO_OK)
    {
        // look at the user defined key bindings.
        rc = _getUserKeyBindingNodeIndex(node,name);
        if (rc != SCMO_OK)
        {
            return rc;
        }
    }

    rc = _getKeyBindingDataAtNodeIndex(node,&pname,pnameLen,type,&pdata);

    if (rc != SCMO_OK)
    {
        return rc;
    }

    *pvalue = _resolveSCMBUnion(
        type,
        false,  // A key binding can never be an array.
        0,
        (char*)pdata-inst.base,
        inst.base);

    return SCMO_OK;
}


SCMO_RC SCMOInstance::_getKeyBindingDataAtNodeIndex(
    Uint32 node,
    const char** pname,
    Uint32 & pnameLen,
    CIMType & type,
    const SCMBUnion** pdata) const
{
    if (node < inst.hdr->numberKeyBindings)
    {
        SCMBKeyBindingValue* theInstKeyBindValueArray =
            (SCMBKeyBindingValue*)&(inst.base[inst.hdr->keyBindingArray.start]);

        // create a pointer to key binding node array of the class.
        Uint64 idx = inst.hdr->theClass.ptr->cls.hdr->
            keyBindingSet.nodeArray.start;
        SCMBKeyBindingNode* theClassKeyBindNodeArray =
            (SCMBKeyBindingNode*)&((inst.hdr->theClass.ptr->cls.base)[idx]);

        type = theClassKeyBindNodeArray[node].type;

        // First resolve pointer to the key binding name
        pnameLen = theClassKeyBindNodeArray[node].name.size;
        *pname = _getCharString(
            theClassKeyBindNodeArray[node].name,
            inst.hdr->theClass.ptr->cls.base);

        // There is no value set in the instance
        if (!theInstKeyBindValueArray[node].isSet)
        {
            return SCMO_NULL_VALUE;
        }

        *pdata = &(theInstKeyBindValueArray[node].data);
    }
    else // look at the user defined key bindings
    {
        SCMBUserKeyBindingElement* theElem = _getUserDefinedKeyBindingAt(node);

        type = theElem->type;

        pnameLen = theElem->name.size;
        *pname = _getCharString(theElem->name,inst.base);

        // There is no value set in the instance
        if (!theElem->value.isSet)
        {
            return SCMO_NULL_VALUE;
        }

        *pdata = &(theElem->value.data);

    }

    return SCMO_OK;
}

SCMO_RC SCMOInstance::_getUserKeyBindingNodeIndex(
    Uint32& node,
    const char* name) const
{

    Uint32 len = strlen(name);
    node = 0;

    Uint64 elementStart = inst.hdr->userKeyBindingElement.start;

    while (elementStart != 0)
    {
        SCMBUserKeyBindingElement* theUserDefKBElement =
            (SCMBUserKeyBindingElement*)&(inst.base[elementStart]);

        if (_equalNoCaseUTF8Strings(
            theUserDefKBElement->name,inst.base,name,len))
        {
            // the node index of a user defined key binding has an offset
            // by the number of key bindings defined in the class
            node = node + inst.hdr->numberKeyBindings;
            return SCMO_OK;
        }
        node = node + 1;
        elementStart = theUserDefKBElement->nextElement.start;
    }

    return SCMO_NOT_FOUND;

}
/*
    Get the node index for a user-defined property
    // KS_TODO confirm not duplicated elsewhere
*/
SCMO_RC SCMOInstance::_getUserPropertyNodeIndex(
    Uint32& node,
    const char* name) const
{
    Uint32 len = strlen(name);
    node = 0;

    Uint64 elementStart = inst.hdr->userPropertyElement.start;

    while (elementStart != 0)
    {
        SCMBUserPropertyElement* pElement =
            (SCMBUserPropertyElement*)&(inst.base[elementStart]);

        if (_equalNoCaseUTF8Strings(
            pElement->name,inst.base,name,len))
        {
            // the node index of a user defined key binding has an offset
            // by the number of user-defined properties defined in the class
            node = node + inst.hdr->numberKeyBindings;
            return SCMO_OK;
        }
        node++;
        elementStart = pElement->nextElement.start;
    }
    return SCMO_NOT_FOUND;

}

CIMType SCMOInstance::_CIMTypeFromKeyBindingType(
    const char* key,
    CIMKeyBinding::Type t)
{
    switch( t )
    {
        case CIMKeyBinding::NUMERIC:
            {
                if( *(key)=='-' )
                {
                   Sint64 x;
                   // check if it is realy an integer
                   if (StringConversion::stringToSignedInteger(key, x))
                   {
                       return CIMTYPE_SINT64;
                   }
                   else
                   {
                       return CIMTYPE_REAL64;
                   }
                }
                else
                {
                    Uint64 x;
                    // check if it is realy an integer
                    if (StringConversion::stringToUnsignedInteger(key, x))
                    {
                        return CIMTYPE_UINT64;
                    }
                    else
                    {
                        return CIMTYPE_REAL64;
                    }
                }
            }


        case CIMKeyBinding::STRING:
        {
            return CIMTYPE_STRING;
        }

        case CIMKeyBinding::BOOLEAN:
        {
            return CIMTYPE_BOOLEAN;
        }

        case CIMKeyBinding::REFERENCE:
        {
            return CIMTYPE_REFERENCE;
        }

        default:
            return CIMTYPE_UINT64;
    }
    return CIMTYPE_UINT64;
}

Boolean SCMOInstance::_setCimKeyBindingStringToSCMOKeyBindingValue(
    const String& kbs,
    CIMType type,
    SCMBKeyBindingValue& scmoKBV
    )
{
    scmoKBV.isSet=false;
    // If it not a simple value, it will be over written.
    scmoKBV.data.simple.hasValue=false;

    if ( kbs.size() == 0 && type != CIMTYPE_STRING)
    {
        // The string is empty ! Do nothing.
        return false;
    }

    CString a = kbs.getCString();
    const char* v = a;

    switch (type)
    {
    case CIMTYPE_UINT8:
        {
            Uint64 x;
            if (StringConversion::stringToUnsignedInteger(v, x) &&
                StringConversion::checkUintBounds(x, type))
            {
              scmoKBV.data.simple.val.u8 = Uint8(x);
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }
    case CIMTYPE_UINT16:
        {
            Uint64 x;
            if (StringConversion::stringToUnsignedInteger(v, x) &&
                StringConversion::checkUintBounds(x, type))
            {
              scmoKBV.data.simple.val.u16 = Uint16(x);
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_UINT32:
        {
            Uint64 x;
            if (StringConversion::stringToUnsignedInteger(v, x) &&
                StringConversion::checkUintBounds(x, type))
            {
              scmoKBV.data.simple.val.u32 = Uint32(x);
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_UINT64:
        {
            Uint64 x;
            if (StringConversion::stringToUnsignedInteger(v, x))
            {
              scmoKBV.data.simple.val.u64 = x;
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_SINT8:
        {
            Sint64 x;
            if (StringConversion::stringToSignedInteger(v, x) &&
                StringConversion::checkSintBounds(x, type))
            {
              scmoKBV.data.simple.val.s8 = Sint8(x);
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_SINT16:
        {
            Sint64 x;
            if (StringConversion::stringToSignedInteger(v, x) &&
                StringConversion::checkSintBounds(x, type))
            {
              scmoKBV.data.simple.val.s16 = Sint16(x);
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_SINT32:
        {
            Sint64 x;
            if (StringConversion::stringToSignedInteger(v, x) &&
                StringConversion::checkSintBounds(x, type))
            {
              scmoKBV.data.simple.val.s32 = Sint32(x);
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_SINT64:
        {
            Sint64 x;
            if (StringConversion::stringToSignedInteger(v, x))
            {
              scmoKBV.data.simple.val.s64 = x;
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_DATETIME:
        {
            CIMDateTime tmp;

            try
            {
                tmp.set(v);
            }
            catch (InvalidDateTimeFormatException&)
            {
                return false;
            }

            memcpy(
                &(scmoKBV.data.dateTimeValue),
                tmp._rep,
                sizeof(SCMBDateTime));
            scmoKBV.isSet=true;
            break;
        }

    case CIMTYPE_REAL32:
        {
            Real64 x;

            if (StringConversion::stringToReal64(v, x))
            {
              scmoKBV.data.simple.val.r32 = Real32(x);
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_REAL64:
        {
            Real64 x;

            if (StringConversion::stringToReal64(v, x))
            {
              scmoKBV.data.simple.val.r64 = x;
              scmoKBV.data.simple.hasValue=true;
              scmoKBV.isSet=true;
            }
            break;
        }

    case CIMTYPE_CHAR16:
        {
            if (kbs.size() == 1)
            {
                scmoKBV.data.simple.val.c16 = kbs[0];
                scmoKBV.data.simple.hasValue=true;
                scmoKBV.isSet=true;
            }
            break;
        }
    case CIMTYPE_BOOLEAN:
        {
            if (String::equalNoCase(kbs,"TRUE"))
            {
                scmoKBV.data.simple.val.bin = true;
                scmoKBV.data.simple.hasValue=true;
                scmoKBV.isSet=true;
            }
            else if (String::equalNoCase(kbs,"FALSE"))
                 {
                     scmoKBV.data.simple.val.bin = false;
                     scmoKBV.data.simple.hasValue=true;
                     scmoKBV.isSet=true;
                 }
            break;
        }

    case CIMTYPE_STRING:
        {
            scmoKBV.isSet=true;
            // Can cause reallocation !
            _setString(kbs,scmoKBV.data.stringValue,&inst.mem);
            return true;
        }
    case CIMTYPE_REFERENCE:
        {
            if (0 != scmoKBV.data.extRefPtr)
            {
                delete scmoKBV.data.extRefPtr;
                scmoKBV.data.extRefPtr = 0;
                scmoKBV.isSet=false;
            }
            // TBD: Optimize parsing and SCMOInstance creation.
            CIMObjectPath theCIMObj(kbs);

            scmoKBV.data.extRefPtr = new SCMOInstance(theCIMObj);
            scmoKBV.isSet=true;

            // Was the conversion successful?
            if (scmoKBV.data.extRefPtr->isEmpty())
            {
                // N0, delete the SCMOInstance.
                delete scmoKBV.data.extRefPtr;
                scmoKBV.data.extRefPtr = 0;
                scmoKBV.isSet=false;
            }
            else
            {
                // This function can cause a reallocation !
                // Pointers can be invalid after the call.
                _setExtRefIndex(&(scmoKBV.data),&inst.mem);
            }
            break;
        }
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        {
            // From PEP 194: EmbeddedObjects cannot be keys.
            throw TypeMismatchException();
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }

    return scmoKBV.isSet;
}

SCMO_RC SCMOInstance::_setKeyBindingFromString(
    const char* name,
    CIMType type,
    String cimKeyBinding)
{
    Uint32 node;

    if (0 == name)
    {
        return SCMO_INVALID_PARAMETER;
    }

    if (SCMO_OK == inst.hdr->theClass.ptr->_getKeyBindingNodeIndex(node,name))
    {
        // create a pointer to keybinding node array of the class.
        Uint64 idx = inst.hdr->theClass.ptr->cls.hdr->
            keyBindingSet.nodeArray.start;
        SCMBKeyBindingNode* theClassKeyBindNodeArray =
            (SCMBKeyBindingNode*)&((inst.hdr->theClass.ptr->cls.base)[idx]);

        // create a pointer to instance keybinding values
        SCMBKeyBindingValue* theInstKeyBindValueArray =
            (SCMBKeyBindingValue*)&(inst.base[inst.hdr->keyBindingArray.start]);

        // If the set was not successful, the conversion was not successful
        if ( !_setCimKeyBindingStringToSCMOKeyBindingValue(
                cimKeyBinding,
                theClassKeyBindNodeArray[node].type,
                theInstKeyBindValueArray[node]))
        {
            return SCMO_TYPE_MISSMATCH;
        }

        return SCMO_OK;
    }

    // the key binding does not belong to the associated class
    // add/set it as user defined key binding.
    SCMBUserKeyBindingElement* ptrNewElement;

    ptrNewElement = _getUserDefinedKeyBinding( name,strlen(name),type);

    // Copy the data.
    // If the set was not successful, the conversion was not successful
    if ( !_setCimKeyBindingStringToSCMOKeyBindingValue(
            cimKeyBinding,
            type,
            ptrNewElement->value))
    {
        return SCMO_TYPE_MISSMATCH;
    }

    return SCMO_OK;
}

SCMO_RC SCMOInstance::setKeyBinding(
    const char* name,
    CIMType type,
    const SCMBUnion* keyvalue)
{
    SCMO_RC rc;
    Uint32 node;

    if (0 == name)
    {
        return SCMO_INVALID_PARAMETER;
    }

    if (0 == keyvalue)
    {
        return SCMO_INVALID_PARAMETER;
    }

    _copyOnWrite();

    // If keybindings exists and cleared using the clearKeyBindings()
    // method, reset the value to the actual keybindings count exists
    // in the class.
    if (!inst.hdr->numberKeyBindings)
    {
        inst.hdr->numberKeyBindings =
            inst.hdr->theClass.ptr->cls.hdr->keyBindingSet.number;
    }

    rc = inst.hdr->theClass.ptr->_getKeyBindingNodeIndex(node,name);

    if (rc != SCMO_OK)
    {
        // the key binding does not belong to the associated class
        // add/set it as user defined key binding.
        SCMBUserKeyBindingElement *pElement;

        pElement = _getUserDefinedKeyBinding( name,strlen(name),type);

        // Is this a new node or an existing user key binding?
        if (pElement->value.isSet && (pElement->type != type))
        {
            return SCMO_TYPE_MISSMATCH;
        }

        pElement->value.isSet=true;

        _setSCMBUnion(
            keyvalue,
            type,
            false, // a key binding can never be an array.
            0,
            pElement->value.data);

         return SCMO_OK;
    }

    return setKeyBindingAt(node, type, keyvalue);
}

SCMO_RC SCMOInstance::setKeyBindingAt(
        Uint32 node,
        CIMType type,
        const SCMBUnion* keyvalue)
{
    if (0 == keyvalue)
    {
        return SCMO_INVALID_PARAMETER;
    }

    // count of class keys + user definded keys
    if (node >= (inst.hdr->numberKeyBindings+
                 inst.hdr->numberUserKeyBindings))
    {
        return SCMO_INDEX_OUT_OF_BOUND;
    }

    _copyOnWrite();

    // If keybindings exists and cleared using the clearKeyBindings()
    // method, reset the value to the actual keybindings count exists
    // in the class.
    if (!inst.hdr->numberKeyBindings)
    {
        inst.hdr->numberKeyBindings =
            inst.hdr->theClass.ptr->cls.hdr->keyBindingSet.number;
    }


   // create a pointer to keybinding node array of the class.
    Uint64 idx = inst.hdr->theClass.ptr->cls.hdr->
        keyBindingSet.nodeArray.start;
    SCMBKeyBindingNode* theClassKeyBindNodeArray =
        (SCMBKeyBindingNode*)&((inst.hdr->theClass.ptr->cls.base)[idx]);

    // is the node a user defined key binding ? Tests if node ge number
    // of key bindings.
    if (node >= inst.hdr->numberKeyBindings)
    {
        SCMBUserKeyBindingElement* theNode = _getUserDefinedKeyBindingAt(node);

        // Does the new value for the user defined keybinding match?
        if (theNode->type != type)
        {
            return SCMO_TYPE_MISSMATCH;
        }

        _setSCMBUnion(
            keyvalue,
            type,
            false, // a key binding can never be an array.
            0,
            theNode->value.data);

         return SCMO_OK;

    }

    SCMBKeyBindingValue* theInstKeyBindValueArray =
        (SCMBKeyBindingValue*)&(inst.base[inst.hdr->keyBindingArray.start]);


    if (theClassKeyBindNodeArray[node].type == type)
    {

        // Has to be set first,
        // because reallocaton can take place in _setSCMBUnion()
        theInstKeyBindValueArray[node].isSet=true;

        _setSCMBUnion(
            keyvalue,
            type,
            false, // a key binding can never be an array.
            0,
            theInstKeyBindValueArray[node].data);

        return SCMO_OK;

    }

    // The type does not match.
    return _setKeyBindingTypeTolerate(
        theClassKeyBindNodeArray[node].type,
        type,
        keyvalue,
        theInstKeyBindValueArray[node]);

}

/**
 * Set a SCMO user defined key binding using the class CIM type tolerating
 * CIM key binding types converted to CIM types by fuction
 *  _CIMTypeFromKeyBindingType().
 *
 * @parm classType The type of the key binding in the class definition
 * @parm setType The type of the key binding to be set.
 * @param keyValue A pointer to the key binding to be set.
 * @param kbValue Out parameter, the SCMO keybinding to be set.
 *
 **/
SCMO_RC SCMOInstance::_setKeyBindingTypeTolerate(
    CIMType classType,
    CIMType setType,
    const SCMBUnion* keyValue,
    SCMBKeyBindingValue& kbValue)
{
    if (setType == CIMTYPE_UINT64 )
    {
        switch (classType)
        {

        case CIMTYPE_UINT8:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.u8=Uint8(keyValue->simple.val.u64);
                break;
            }
        case CIMTYPE_UINT16:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.u16=Uint16(keyValue->simple.val.u64);
                break;
            }
        case CIMTYPE_UINT32:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.u32=Uint32(keyValue->simple.val.u64);
                break;
            }
        case CIMTYPE_UINT64:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.u64=keyValue->simple.val.u64;
                break;
            }
        default:
            {
                return SCMO_TYPE_MISSMATCH;
            }
        }
        return SCMO_OK;

    }

    if (setType == CIMTYPE_SINT64)
    {
        switch (classType)
        {

        case CIMTYPE_SINT8:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.s8=Sint8(keyValue->simple.val.s64);
                break;
            }
        case CIMTYPE_SINT16:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.s16=Sint16(keyValue->simple.val.s64);
                break;
            }
        case CIMTYPE_SINT32:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.s32=Sint32(keyValue->simple.val.s64);
                break;
            }
        case CIMTYPE_SINT64:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.s64=keyValue->simple.val.s64;
                break;
            }
        default:
            {
                return SCMO_TYPE_MISSMATCH;
            }
        }
        return SCMO_OK;
    }

    if (setType == CIMTYPE_REAL64)
    {
        switch (classType)
        {
        case CIMTYPE_REAL32:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.r32=Real32(keyValue->simple.val.r64);
                break;
            }
        case CIMTYPE_REAL64:
            {
                kbValue.isSet=true;
                kbValue.data.simple.hasValue=true;
                kbValue.data.simple.val.r64=keyValue->simple.val.r64;
                break;
            }
        default:
            {
                return SCMO_TYPE_MISSMATCH;
            }
        }
        return SCMO_OK;
    }
    else
    {
        // If type defined in the class and the provided type does not match
        // at this point, no convertaion can be done and the provided type
        // is handlend as type missmatch.
        if (classType != setType)
        {
            return SCMO_TYPE_MISSMATCH;
        }

        switch (classType)
        {
        case CIMTYPE_DATETIME:
        case CIMTYPE_BOOLEAN:
        case CIMTYPE_UINT64:
        case CIMTYPE_SINT64:
        case CIMTYPE_REAL64:
        case CIMTYPE_STRING:
        case CIMTYPE_REFERENCE:
            {
                kbValue.isSet=true;
                _setSCMBUnion(keyValue,classType,false, 0,kbValue.data);
                return SCMO_OK;
            }
        default:
            {
                return SCMO_TYPE_MISSMATCH;
            }
        }
    }

    return SCMO_TYPE_MISSMATCH;

}

// class SCMODump only in debug builds available
#ifdef PEGASUS_DEBUG
/******************************************************************************
 * SCMODump Print and Dump functions
 *****************************************************************************/
SCMODump::SCMODump()
{
    _out = stderr;
    _fileOpen = false;

# ifdef PEGASUS_OS_ZOS
    setEBCDICEncoding(fileno(_out));
# endif

}

SCMODump::SCMODump(const char* filename)
{
    openFile(filename);
}

void SCMODump::openFile(const char* filename)
{
    const char* pegasusHomeDir = getenv("PEGASUS_HOME");

    if (pegasusHomeDir == 0)
    {
        pegasusHomeDir = ".";
    }

    _filename = pegasusHomeDir;
    _filename.append("/");
    _filename.append(filename);

    _out = fopen((const char*)_filename.getCString(),"w+");

    _fileOpen = true;

# ifdef PEGASUS_OS_ZOS
    setEBCDICEncoding(fileno(_out));
# endif

}

void SCMODump::deleteFile()
{
    if(_fileOpen)
    {
        closeFile();
    }

    System::removeFile((const char*)_filename.getCString());

}
void SCMODump::closeFile()
{
    if (_fileOpen)
    {
        fclose(_out);
        _fileOpen=false;
        _out = stdout;
    }
}

SCMODump::~SCMODump()
{
    if (_fileOpen)
    {
        fclose(_out);
        _fileOpen=false;
    }
}

Boolean SCMODump::compareFile(String master)
{
    if (!_fileOpen)
    {
        return false;
    }
    closeFile();

    ifstream isMaster;
    ifstream isDumpFile;

    Open(isDumpFile, _filename);
    Open(isMaster, master);

    String aLine;
    String bLine;

    while (GetLine(isDumpFile, aLine) && GetLine(isMaster, bLine))
    {
        if (aLine != bLine)
        {
            cout << "|" << aLine << "|" << endl;
            cout << "|" << bLine << "|" << endl;
            isDumpFile.close();
            isMaster.close();
            return false;
        }
    };
    isDumpFile.close();
    isMaster.close();
    return true;
}

void SCMODump::dumpSCMOInstance(SCMOInstance& testInst,
    Boolean inclMemHdr,
    Boolean verbose) const
{
    SCMBInstance_Main* insthdr = testInst.inst.hdr;
    char* instbase = testInst.inst.base;

    fprintf(_out,"\n\nDump of SCMOInstance\n");

    if (inclMemHdr)
    {
        _dumpSCMBMgmt_Header(insthdr->header,instbase);
    }

    // The reference counter for this c++ class
    // KS_FUTURE Modify the bool displays to use boolToString
    fprintf(_out,"\nrefCount=%i",insthdr->refCount.get());
    fprintf(_out,"\ntheClass: %p",insthdr->theClass.ptr);
    fprintf(_out,"\n\nThe Flags:");
    fprintf(_out,"\n   includeQualifiers: %s",
           (insthdr->flags.includeQualifiers ? "True" : "False"));
    fprintf(_out,"\n   includeClassOrigin: %s",
           (insthdr->flags.includeClassOrigin ? "True" : "False"));
    fprintf(_out,"\n   isClassOnly: %s",
           (insthdr->flags.isClassOnly ? "True" : "False"));
    fprintf(_out,"\n   isCompromised: %s",
           (insthdr->flags.isCompromised ? "True" : "False"));
    fprintf(_out,"\n   exportSetOnly: %s",
           (insthdr->flags.exportSetOnly ? "True" : "False"));
    fprintf(_out,"\n   noClassForThisInstance: %s",
           (insthdr->flags.noClassForInstance ? "True" : "False"));
    fprintf(_out,"\n\ninstNameSpace: \'%s\'",
           NULLSTR(_getCharString(insthdr->instNameSpace,instbase)));
    fprintf(_out,"\n\ninstClassName: \'%s\'",
           NULLSTR(_getCharString(insthdr->instClassName,instbase)));
    fprintf(_out,"\n\nhostName: \'%s\'",
           NULLSTR(_getCharString(insthdr->hostName,instbase)));

    dumpSCMOInstanceKeyBindings(testInst, verbose);

    dumpInstanceProperties(testInst, verbose);
    fprintf(_out,"\n\n");

}

void SCMODump::dumpInstanceProperties(
    SCMOInstance& testInst,
    Boolean verbose) const
{
    SCMBInstance_Main* insthdr = testInst.inst.hdr;
    char* instbase = testInst.inst.base;

    SCMBValue* val =
        (SCMBValue*)_resolveDataPtr(insthdr->propertyArray,instbase);

    fprintf(_out,"\n\nInstance Properties :");
    fprintf(_out,"\n=====================");
    fprintf(_out,"\n\nNumber of properties in instance : %u",
           insthdr->numberProperties);

    for (Uint32 i = 0, k = insthdr->numberProperties; i < k; i++)
    {
        fprintf(_out,"\n\nInstance property (#%3u) %s\n",i,
                NULLSTR(insthdr->theClass.ptr->_getPropertyNameAtNode(i)));

        printSCMOValue(val[i],instbase,verbose);
    }
    fprintf(_out,"\n\nInstance User-DefinedProperties :");
    fprintf(_out,"\n=====================");
    fprintf(_out,"\n\nNumber of user-defined properties in instance : %u\n",
           insthdr->numberUserProperties);

    SCMBUserPropertyElement* theElement;

    Uint64 start = insthdr->userPropertyElement.start;

    Uint32 count = 0;
    while (start != 0)
    {
        theElement = (SCMBUserPropertyElement*)&(instbase[start]);

        // KS_TODO eliminate this display. Once we are actually running
        // it is of little value.
        fprintf(_out, "\nUser-defined PropertyElement # %u %s", count++,
                ((theElement->nextElement.start != 0)?
                    "" : "last"));

        _dumpUserDefinedPropertyElement(
            instbase,theElement, verbose);

        start = theElement->nextElement.start;
    } // for all user def. key bindings.
}

void SCMODump::_dumpUserDefinedPropertyElement(char* instbase,
    SCMBUserPropertyElement* theElement, Boolean verbose) const
{
        if (theElement->value.flags.isSet)
        {
            fprintf(_out,"\n\nName: '%s'\nType: '%s'",
                NULLSTR(_getCharString(theElement->name,instbase)),
                cimTypeToString(theElement->value.valueType));

            printSCMOValue(
                theElement->value,
                instbase,
                verbose);
        }
        else
        {
            fprintf(_out,"\n\n    %s : Not Set",
                NULLSTR(_getCharString(theElement->name,instbase)));
        }
}

void SCMODump::dumpSCMOInstanceKeyBindings(
    SCMOInstance& testInst,
    Boolean verbose) const
{
    SCMBInstance_Main* insthdr = testInst.inst.hdr;
    char* instbase = testInst.inst.base;

    // create a pointer to keybinding node array of the class.
    Uint64 idx = insthdr->theClass.ptr->cls.hdr->keyBindingSet.nodeArray.start;
    SCMBKeyBindingNode* theClassKeyBindNodeArray =
        (SCMBKeyBindingNode*)&((insthdr->theClass.ptr->cls.base)[idx]);

    SCMBKeyBindingValue* ptr =
        (SCMBKeyBindingValue*)
             _resolveDataPtr(insthdr->keyBindingArray,instbase);

    fprintf(_out,"\n\nInstance Key Bindings :");
    fprintf(_out,"\n=======================");
    fprintf(_out,"\n\nNumber of Key Bindings defined in the Class: %u",
            insthdr->numberKeyBindings);

    for (Uint32 i = 0, k = insthdr->numberKeyBindings; i < k; i++)
    {
        if (ptr[i].isSet)
        {
            fprintf(_out,"\n\nName: '%s'\nType: '%s'",
                NULLSTR(_getCharString(
                    theClassKeyBindNodeArray[i].name,
                    insthdr->theClass.ptr->cls.base)),
                cimTypeToString(theClassKeyBindNodeArray[i].type));
            printUnionValue(
                theClassKeyBindNodeArray[i].type,
                ptr[i].data,
                instbase,
                verbose);
        }
        else
        {
            fprintf(_out,"\n\nName: '%s': Not Set",
                NULLSTR(_getCharString(
                    theClassKeyBindNodeArray[i].name,
                    insthdr->theClass.ptr->cls.base)));
        }
    }

    fprintf(_out,"\n\nNumber of User Defined Key Bindings: %u",
            insthdr->numberUserKeyBindings);


    SCMBUserKeyBindingElement* theUserDefKBElement;

    Uint64 start = insthdr->userKeyBindingElement.start;
    while (start != 0)
    {
        theUserDefKBElement = (SCMBUserKeyBindingElement*)&(instbase[start]);

        if (theUserDefKBElement->value.isSet)
        {
            fprintf(_out,"\n\nName: '%s'\nType: '%s'",
                NULLSTR(_getCharString(theUserDefKBElement->name,instbase)),
                cimTypeToString(theUserDefKBElement->type));
            printUnionValue(
                theUserDefKBElement->type,
                theUserDefKBElement->value.data,
                instbase,
                verbose);
        }
        else
        {
            fprintf(_out,"\n\n    %s : Not Set",
                NULLSTR(_getCharString(theUserDefKBElement->name,instbase)));

        }
        start = theUserDefKBElement->nextElement.start;
    } // for all user def. key bindings.

    fprintf(_out,"\n\n");

}

void SCMODump::_dumpSCMBMgmt_Header(SCMBMgmt_Header& header,char* base) const
{
    fprintf(_out,"\nThe Management Header:");
    // The magic number
    fprintf(_out,"\n   magic=%08X",header.magic);
    // Total size of the memory block( # bytes )
    fprintf(_out,"\n   totalSize=%llu",header.totalSize);
    // Free bytes in the block
    fprintf(_out,"\n   freeBytes=%llu",header.freeBytes);
    // Index to the start of the free space in this SCMB memory block.
    fprintf(_out,"\n   startOfFreeSpace=%llu",header.startOfFreeSpace);
    // Number of external references in this instance.
    fprintf(_out,"\n   numberExtRef=%u",header.numberExtRef);
    // Size of external reference index array;
    fprintf(_out,"\n   sizeExtRefIndexArray=%u",header.sizeExtRefIndexArray);

    if (header.numberExtRef > 0)
    {
        fprintf(_out,"\n   extRefIndexArray=[");
        Uint64* extRefIndexArray =
            (Uint64*)&(base[header.extRefIndexArray.start]);

        for (Uint32 i = 0; i < header.numberExtRef;)
        {
            fprintf(_out,"%llu",extRefIndexArray[i]);
            i++;
            if (i != header.numberExtRef)
            {
                fprintf(_out,", ");
            }
        }
        fprintf(_out,"\n");
    }
    else
    {
           fprintf(_out,"\n   extRefIndexArray=[NO INDEX]\n");
    }
}

void SCMODump::dumpSCMOClass(SCMOClass& testCls, Boolean inclMemHdr) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;
    char* clsbase = testCls.cls.base;

    fprintf(_out,"\n\nDump of SCMOClass\n");

    if (inclMemHdr)
    {
        _dumpSCMBMgmt_Header(clshdr->header,clsbase);
    }

    // The reference counter for this c++ class
    fprintf(_out,"\nrefCount=%i",clshdr->refCount.get());
    fprintf(_out,"\n\nThe Flags:");
    fprintf(_out,"\n   isEmpty: %s",
           (clshdr->flags.isEmpty ? "True" : "False"));
    fprintf(_out,"\n\nsuperClassName: \'%s\'",
           NULLSTR(_getCharString(clshdr->superClassName,clsbase)));
    fprintf(_out,"\nnameSpace: \'%s\'",
            NULLSTR(_getCharString(clshdr->nameSpace,clsbase)));
    fprintf(_out,"\nclassName: \'%s\'",
            NULLSTR(_getCharString(clshdr->className,clsbase)));
    fprintf(_out,"\n\nTheClass qualfiers:");
    _dumpQualifierArray(
        clshdr->qualifierArray.start,
        clshdr->numberOfQualifiers,
        clsbase);
    fprintf(_out,"\n");
    dumpKeyPropertyMask(testCls);
    fprintf(_out,"\n");
    dumpKeyIndexList(testCls);
    fprintf(_out,"\n");
    dumpClassProperties(testCls);
    fprintf(_out,"\n");
    dumpKeyBindingSet(testCls);
    fprintf(_out,"\n");
    /*
    */
    fprintf(_out,"\n");

}

void SCMODump::dumpSCMOClassQualifiers(SCMOClass& testCls) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;
    char* clsbase = testCls.cls.base;

    fprintf(_out,"\n\nTheClass qualfiers:");
    _dumpQualifierArray(
        clshdr->qualifierArray.start,
        clshdr->numberOfQualifiers,
        clsbase);
    fprintf(_out,"\n\n\n");

}

void SCMODump::hexDumpSCMOClass(SCMOClass& testCls) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;
    char* clsbase = testCls.cls.base;

    fprintf(_out,"\n\nHex dump of a SCMBClass:");
    fprintf(_out,"\n========================");
    fprintf(_out,"\n\n Size of SCMBClass: %llu\n\n",clshdr->header.totalSize);

    _hexDump(clsbase,clshdr->header.totalSize);

}
void SCMODump::dumpKeyIndexList(SCMOClass& testCls) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;
    char* clsbase = testCls.cls.base;

    fprintf(_out,"\n\nKey Index List:");
    fprintf(_out,"\n===============\n");

    // Get absolut pointer to key index list of the class
    Uint32* keyIndex = (Uint32*)&(clsbase)[clshdr->keyIndexList.start];
    Uint32 line,j,i,k = clshdr->propertySet.number;
    for (j = 0; j < k; j = j + line)
    {
        if ((clshdr->propertySet.number-j)/16)
        {
            line = 16 ;
        }
        else
        {
            line = clshdr->propertySet.number & 15;
        }


        fprintf(_out,"Index :");
        for (i = 0; i < line; i++)
        {
            fprintf(_out," %3u",j+i);
        }

        fprintf(_out,"\nNode  :");
        for (i = 0; i < line; i++)
        {
            fprintf(_out," %3u",keyIndex[j+i]);
        }

        fprintf(_out,"\n\n");

    }

}

void SCMODump::dumpKeyBindingSet(SCMOClass& testCls) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;

    fprintf(_out,"\n\nKey Binding Set:");
    fprintf(_out,"\n=================\n");
    fprintf(_out,"\nNumber of key bindings: %3u",clshdr->keyBindingSet.number);
    dumpHashTable(
        clshdr->keyBindingSet.hashTable,
        PEGASUS_KEYBINDIG_SCMB_HASHSIZE);

    dumpClassKeyBindingNodeArray(testCls);

}

void SCMODump::dumpClassKeyBindingNodeArray(SCMOClass& testCls) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;
    char* clsbase = testCls.cls.base;

    SCMBKeyBindingNode* nodeArray =
        (SCMBKeyBindingNode*)
             &(clsbase[clshdr->keyBindingSet.nodeArray.start]);

    for (Uint32 i = 0, k = clshdr->keyBindingSet.number; i < k; i++)
    {
        fprintf(_out,"\n\n===================");
        fprintf(_out,"\nKey Binding #%3u",i);
        fprintf(_out,"\n===================");

        fprintf(_out,"\nHas next: %s",(nodeArray[i].hasNext?"TRUE":"FALSE"));
        if (nodeArray[i].hasNext)
        {
            fprintf(_out,"\nNext Node: %3u",nodeArray[i].nextNode);
        }
        else
        {
            fprintf(_out,"\nNext Node: N/A");
        }

        fprintf(_out,"\nKey Property name: %s",
               NULLSTR(_getCharString(nodeArray[i].name,clsbase)));

        fprintf(_out,"\nHash Tag %3u Hash Index %3u",
               nodeArray[i].nameHashTag,
               nodeArray[i].nameHashTag & (PEGASUS_KEYBINDIG_SCMB_HASHSIZE -1));

        fprintf(_out,"\nType: %s",cimTypeToString(nodeArray[i].type));

    }

}

void SCMODump::dumpClassProperties(SCMOClass& testCls) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;

    fprintf(_out,"\n\nClass Properties:");
    fprintf(_out,"\n=================\n");
    fprintf(_out,"\nNumber of properties: %3u",clshdr->propertySet.number);
    dumpHashTable(
        clshdr->propertySet.hashTable,
        PEGASUS_PROPERTY_SCMB_HASHSIZE);
    dumpClassPropertyNodeArray(testCls);

}

void SCMODump::dumpClassPropertyNodeArray(SCMOClass& testCls) const
{
    SCMBClass_Main* clshdr = testCls.cls.hdr;
    char* clsbase = testCls.cls.base;

    SCMBClassPropertyNode* nodeArray =
        (SCMBClassPropertyNode*)
            &(clsbase[clshdr->propertySet.nodeArray.start]);

    for (Uint32 i = 0, k =  clshdr->propertySet.number; i < k; i++)
    {

        fprintf(_out,"\nClass property #%3u",i);
        fprintf(_out,"\n===================");

        fprintf(_out,"\nHas next: %s",(nodeArray[i].hasNext?"TRUE":"FALSE"));
        if (nodeArray[i].hasNext)
        {
            fprintf(_out,"\nNext Node: %3u",nodeArray[i].nextNode);
        }
        else
        {
            fprintf(_out,"\nNext Node: N/A");
        }

        _dumpClassProperty(nodeArray[i].theProperty,clsbase);
    }
}

void SCMODump::_dumpClassProperty(
    const SCMBClassProperty& prop,
    char* clsbase) const
{
    fprintf(_out,"\nProperty name: %s",
            NULLSTR(_getCharString(prop.name,clsbase)));

    fprintf(_out,"\nHash Tag %3u Hash Index %3u",
           prop.nameHashTag,
           prop.nameHashTag & (PEGASUS_PROPERTY_SCMB_HASHSIZE -1));
    fprintf(_out,"\nPropagated: %s isKey: %s",
           (prop.flags.propagated?"TRUE":"FALSE"),
           (prop.flags.isKey?"TRUE":"FALSE")
           );

    fprintf(_out,"\nOrigin class name: %s",
           NULLSTR(_getCharString(prop.originClassName,clsbase)));
    fprintf(_out,"\nReference class name: %s",
           NULLSTR(_getCharString(prop.refClassName,clsbase)));

    printSCMOValue(prop.defaultValue,clsbase);

    _dumpQualifierArray(
        prop.qualifierArray.start,
        prop.numberOfQualifiers,
        clsbase);

}

void SCMODump::dumpHashTable(Uint32* hashTable,Uint32 size) const
{
    Uint32 i,j,line;
    fprintf(_out,"\n\nHash table:\n");


    for (j = 0; j < size; j = j + line)
    {
        if ((size-j)/16)
        {
            line = 16 ;
        }
        else
        {
            line = size & 15;
        }


        fprintf(_out,"Index    :");
        for (i = 0; i < line; i++)
        {
            fprintf(_out," %3u",j+i);
        }

        fprintf(_out,"\nHashTable:");
        for (i = 0; i < line; i++)
        {
            fprintf(_out," %3u",hashTable[j+i]);
        }

        fprintf(_out,"\n\n");

    }


}

void SCMODump::_dumpQualifierArray(
    Uint64 start,
    Uint32 size,
    char* clsbase
    ) const
{

    SCMBQualifier *theArray = (SCMBQualifier*)&(clsbase[start]);

    for(Uint32 i = 0; i < size; i++)
    {
        _dumpQualifier(theArray[i],clsbase);
    }
}

void SCMODump::_dumpQualifier(
    const SCMBQualifier& theQualifier,
    char* clsbase
    ) const
{
     if(theQualifier.name == QUALNAME_USERDEFINED)
     {
         fprintf(_out,"\n\nQualifier user defined name: \'%s\'",
                NULLSTR(_getCharString(theQualifier.userDefName,clsbase)));
     }
     else
     {
         fprintf(_out,"\n\nQualifier DMTF defined name: \'%s\'",
                SCMOClass::qualifierNameStrLit(theQualifier.name).str);
     }

     fprintf(_out,"\nPropagated : %s",
            (theQualifier.propagated ? "True" : "False"));
     fprintf(_out,"\nFlavor : %s",
         (const char*)(CIMFlavor(theQualifier.flavor).toString().getCString()));

     printSCMOValue(theQualifier.value,clsbase);

}
/*
    printSCMOValue is a superset of printUnionValue.  It prints type
    information, flags, and then the UnionValue for the element
    or all elements in the array
*/
void SCMODump::printSCMOValue(
    const SCMBValue& theValue,
    char* base,
    Boolean verbose) const
{
   fprintf(_out,"\nValueType : %s",cimTypeToString(theValue.valueType));
   fprintf(_out,"\nValue was set: %s",
       (theValue.flags.isSet ? "True" : "False"));
   if (theValue.flags.isNull)
   {
       fprintf(_out,"\nIt's a NULL value.");
       return;
   }
   if (theValue.flags.isArray)
   {
       fprintf(
           _out,
           "\nThe value is an Array of size: %u",
           theValue.valueArraySize);
       printArrayValue(
           theValue.valueType,
           theValue.valueArraySize,
           theValue.value,
           base,
           verbose);
   }
   else
   {
       printUnionValue(theValue.valueType,theValue.value,base,verbose);
   }

   return;

}

void SCMODump::dumpKeyPropertyMask(SCMOClass& testCls ) const
{

    SCMBClass_Main* clshdr = testCls.cls.hdr;
    char* clsbase = testCls.cls.base;

     Uint64 *theKeyMask = (Uint64*)&(clsbase[clshdr->keyPropertyMask.start]);
     Uint32 end, noProperties = clshdr->propertySet.number;
     Uint32 noMasks = (noProperties-1)/64;
     Uint64 printMask = 1;

     for (Uint32 i = 0; i <= noMasks; i++ )
     {
         printMask = 1;
         if (i < noMasks)
         {
             end = 64;
         }
         else
         {
             end = noProperties & 63;
         }

         fprintf(_out,"\nkeyPropertyMask[%02u]= ",i);

         for (Uint32 j = 0; j < end; j++)
         {
             if (j > 0 && !(j & 7))
             {
                 fprintf(_out," ");
             }

             if (theKeyMask[i] & printMask)
             {
                 fprintf(_out,"1");
             }
             else
             {
                 fprintf(_out,"0");
             }

             printMask = printMask << 1;
         }
         fprintf(_out,"\n");
     }
}
void SCMODump::_dumpEmbeddedInstance(SCMBUnion u, Boolean verbose) const
{
    if (verbose)
    {
        fprintf(_out,"\n-----------> "
                      "Start of embedded external reference"
                      " <-----------\n");
        dumpSCMOInstance(*u.extRefPtr, true, verbose);
        fprintf(_out,"\n-----------> "
                     "End of embedded external reference"
                     " <-----------\n");
    }
    else
    {
        fprintf(_out,
            "\nPointer to external Reference : \'%p\'",
            u.extRefPtr);
    }
}
void SCMODump::_hexDump(char* buffer,Uint64 length) const
{

    unsigned char printLine[3][80];
    int p;
    int len;
    unsigned char item;

    for (Uint64 i = 0; i < length;i=i+1)
    {
        p = (int)i%80;

        if ((p == 0 && i > 0) || i == length-1 )
        {
            for (int y = 0; y < 3; y=y+1)
            {
                if (p == 0)
                {
                    len = 80;
                } else
                {
                    len = p;
                }

                for (int x = 0; x < len; x=x+1)
                {
                    if (y == 0)
                    {
                        fprintf(_out,"%c",printLine[y][x]);
                    }
                    else
                    {
                        fprintf(_out,"%1X",printLine[y][x]);
                    }
                }
                fprintf(_out,"\n");
            }
            fprintf(_out,"\n");
        }

        item = (unsigned char)buffer[i];

        if (item < 32 || item > 125 )
        {
            printLine[0][p] = '.';
        } else
        {
            printLine[0][p] = item;
        }

        printLine[1][p] = item/16;
        printLine[2][p] = item & 15;

    }
}

void SCMODump::printArrayValue(
    CIMType type,
    Uint32 size,
    SCMBUnion u,
    char* base,
    Boolean verbose) const
{
    Buffer out;

    SCMBUnion* p;
    p = (SCMBUnion*)&(base[u.arrayValue.start]);

    switch (type)
    {
    case CIMTYPE_BOOLEAN:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.bin);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_UINT8:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.u8);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_SINT8:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.s8);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_UINT16:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.u16);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_SINT16:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.s16);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_UINT32:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.u32);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_SINT32:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.s32);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_UINT64:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.u64);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_SINT64:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.s64);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_REAL32:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.r32);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_REAL64:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.r64);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_CHAR16:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                out.append('\'');
                _toString(out,p[i].simple.val.c16);
                out << STRLIT("\'(hasValue=");
                out << (p[i].simple.hasValue ?
                          STRLIT("TRUE)"):
                          STRLIT("FALSE)"));
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_STRING:
        {
            for (Uint32 i = 0; i < size; i++)
            {
                if ( 0 != p[i].stringValue.size)
                {
                    out.append('\'');
                    out.append(
                        (const char*)_getCharString(p[i].stringValue,base),
                        p[i].stringValue.size-1);
                    out.append('\'');
                }
                else
                {
                  out << STRLIT("NULL;");
                }
                out.append(';');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_DATETIME:
        {
            CIMDateTime x;
            for (Uint32 i = 0; i < size; i++)
            {
                memcpy(x._rep,&(p[i].dateTimeValue),sizeof(SCMBDateTime));
                _toString(out,x);
                out.append(' ');
            }
            fprintf(_out,"\nThe values are: %s",out.getData());
            break;
        }

    case CIMTYPE_REFERENCE:
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        {
            if (verbose)
            {
                for (Uint32 i = 0; i < size; i++)
                {
                    fprintf(_out,"\n-----------> "
                                  "Start of embedded external reference [%d]"
                                  " <-----------\n\n",i);
                    dumpSCMOInstance(*(p[i].extRefPtr), false, verbose);
                    fprintf(_out,"\n-----------> "
                                  "End of embedded external reference [%d]"
                                  " <-----------\n\n",i);
                }

            } else
            {
                fprintf(_out,"\nThe values are: ");

                for (Uint32 i = 0; i < size; i++)
                {
                    fprintf(
                        _out,
                        "\nPointer to external Reference[%d] : \'%p\';",
                        i,p[i].extRefPtr);
                }
            }

            break;

        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }

    return;
}

/*
    printUnionValue outputs a single Union Value.
    Note: this requires the base to output string information
*/

// KS_FUTURE this display does not represent or use the hasValue flag
void SCMODump::printUnionValue(
    CIMType type,
    SCMBUnion u,
    char* base,
    Boolean verbose) const
{
    Buffer out;

    switch (type)
    {
    case CIMTYPE_BOOLEAN:
        {
            _toString(out,u.simple.val.bin);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_UINT8:
        {
            _toString(out,u.simple.val.u8);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_SINT8:
        {
            _toString(out,u.simple.val.s8);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_UINT16:
        {
            _toString(out,(Uint32)u.simple.val.u16);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_SINT16:
        {
            _toString(out,u.simple.val.s16);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_UINT32:
        {
            _toString(out,u.simple.val.u32);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_SINT32:
        {
            _toString(out,u.simple.val.s32);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_UINT64:
        {
            _toString(out,u.simple.val.u64);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_SINT64:
        {
            _toString(out,u.simple.val.s64);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_REAL32:
        {
            _toString(out,u.simple.val.r32);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_REAL64:
        {
            _toString(out,u.simple.val.r32);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_CHAR16:
        {
            _toString(out,u.simple.val.c16);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_STRING:
        {
            if ( 0 != u.stringValue.size)
            {
                out.append((const char*)_getCharString(u.stringValue,base),
                           u.stringValue.size-1);
            }
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_DATETIME:
        {
            CIMDateTime x;
            memcpy(x._rep,&(u.dateTimeValue),sizeof(SCMBDateTime));
            _toString(out,x);
            fprintf(_out,"\nThe Value is: '%s'",out.getData());
            break;
        }

    case CIMTYPE_REFERENCE:
    case CIMTYPE_OBJECT:
    case CIMTYPE_INSTANCE:
        {
            _dumpEmbeddedInstance(u, verbose);
            break;
        }
    default:
        {
            PEGASUS_UNREACHABLE(PEGASUS_ASSERT(false);)
            break;
        }
    }

  return;
}
#endif // PEGASUS_DEBUG (class SCMODump only in debug builds available)


/*****************************************************************************
 * The constant functions
 *****************************************************************************/

#ifdef PEGASUS_HAS_ICU
Uint32 _utf8ICUncasecmp(
    const char* a,
    const char* b,
    Uint32 len)
{
    UErrorCode errorCode=U_ZERO_ERROR;

    Uint32 rc, a16len,b16len,utf16BufLen;
    utf16BufLen = (len*sizeof(UChar))+2;

    UChar* a_UTF16 = (UChar*)malloc(utf16BufLen);
    UChar* b_UTF16 = (UChar*)malloc(utf16BufLen);

    UConverter *conv = ucnv_open(0, &errorCode);
    if(U_FAILURE(errorCode))
    {
        free(a_UTF16);
        free(b_UTF16);
        String message("SCMO::_utf8ICUncasecmp() ICUError: ");
        message.append(u_errorName(errorCode));
        message.append(" Can not open ICU default converter!");
        throw CIMException(CIM_ERR_FAILED,message );
    }

    a16len = ucnv_toUChars(conv,a_UTF16,utf16BufLen,a,len,&errorCode);

    if(U_FAILURE(errorCode))
    {
        free(a_UTF16);
        free(b_UTF16);
        ucnv_close(conv);
        String message("SCMO::_utf8ICUncasecmp() ICUError: ");
        message.append(u_errorName(errorCode));
        message.append(" Can not convert string a:'");
        message.append(String(a,len));
        message.append('\'');
        throw CIMException(CIM_ERR_FAILED,message );
    }

    b16len = ucnv_toUChars(conv,b_UTF16,utf16BufLen,b,len,&errorCode);

    if(U_FAILURE(errorCode))
    {
        free(a_UTF16);
        free(b_UTF16);
        ucnv_close(conv);
        String message("SCMO::_utf8ICUncasecmp() ICUError: ");
        message.append(u_errorName(errorCode));
        message.append(" Can not convert string b:'");
        message.append(String(b,len));
        message.append('\'');
        throw CIMException(CIM_ERR_FAILED,message );
    }

    rc = u_strCaseCompare(
        a_UTF16,a16len,
        b_UTF16,b16len,
        U_FOLD_CASE_DEFAULT,
        &errorCode);

    if(U_FAILURE(errorCode))
    {
        free(a_UTF16);
        free(b_UTF16);
        ucnv_close(conv);
        String message("SCMO::_utf8ICUncasecmp() ICUError: ");
        message.append(u_errorName(errorCode));
        message.append(" Can not compare string a:'");
        message.append(String(a,len));
        message.append("' with b: '");
        message.append(String(b,len));
        message.append('\'');
        throw CIMException(CIM_ERR_FAILED,message );
    }

    free(a_UTF16);
    free(b_UTF16);
    ucnv_close(conv);

    return(rc);
}
#endif

/**
 * This function calculates a free memory slot in the single
 * chunk memory block. Warning: In this routine a reallocation
 * may take place.
 * @param ptr A reference to a data SCMB data pointer. The values to the free
 *            block is written into this pointer. If the provided ptr is
 *            located in the single chunk memory block, this pointer may be
 *            invalid after this call. You have to recalcuate the pointer
 *            after calling this function.
 * @parm size The requested free memory slot.
 * @parm pmem A reference to the pointer of the single chunk memory block.
 *            e.g. &cls.mem
 * @return The relative index of the free memory slot.
 */
Uint64 _getFreeSpace(
    SCMBDataPtr& ptr,
    Uint32 size,
    SCMBMgmt_Header** pmem)
{
    Uint64 oldSize, start;
    Uint64 alignedStart, reqAlignSize;

    if (size == 0)
    {
        ptr.start = 0;
        ptr.size = 0;
        return 0;
    }

    // The SCMBDataPtr has to be set before any reallocation.
    start = (*pmem)->startOfFreeSpace;

    // Need to align the start of freespace to 8 byte
    // boundaries to avoid alignment issues on some architectures
    // Round up to nearest multiple of 8
    alignedStart = (start + 7) & ~7;
    // due to the alignment, a little more room is needed in the SCMB
    reqAlignSize = (size + alignedStart - start);

    ptr.start = alignedStart;
    ptr.size = size;
    // add 8 bytes of size for later alignment on the next pointer
    while ((*pmem)->freeBytes < reqAlignSize)
    {
        // save old size of buffer
        oldSize = (*pmem)->totalSize;
        // reallocate the buffer, double the space !
        // This is a working approach until a better algorithm is found.
        void* newBlockPtr = realloc((*pmem),(size_t)oldSize*2);
        if ((newBlockPtr) == 0)
        {
            // Not enough memory!
            throw PEGASUS_STD(bad_alloc)();
        }
        (*pmem) = (SCMBMgmt_Header*)newBlockPtr;
        // increase the total size and free space
        (*pmem)->freeBytes+=oldSize;
        (*pmem)->totalSize+=oldSize;
    }

    (*pmem)->freeBytes -= reqAlignSize;
    (*pmem)->startOfFreeSpace = alignedStart + size;

    // Init memory from unaligned start up to the size required with alignment
    // to zero.
    memset(&((char*)(*pmem))[start],0,(size_t)reqAlignSize);
    PEGASUS_DEBUG_ASSERT(
        ((*pmem)->freeBytes+(*pmem)->startOfFreeSpace) == (*pmem)->totalSize);

    return alignedStart;
}

void _setString(
    const String& theString,
    SCMBDataPtr& ptr,
    SCMBMgmt_Header** pmem)
{

    // Get the UTF8 CString
    CString theCString=theString.getCString();
    // Get the real size of the UTF8 sting + \0.
    // It maybe greater then the length in the String due to
    // 4 byte encoding of non ASCII chars.
    Uint64 start;
    Uint32 length = strlen((const char*)theCString)+1;

    // If the string is not empty.
    if (length != 1)
    {

       // Attention ! In this function a reallocation may take place.
       // The reference ptr may be unusable after the call to _getFreeSpace
       // --> use the returned start index.
       start = _getFreeSpace(ptr , length, pmem);
       // Copy string including trailing \0
       memcpy(&((char*)(*pmem))[start],(const char*)theCString,length);
    }
    else
    {
        ptr.start = 0;
        ptr.size = 0;
    }
}

void _setBinary(
    const void* theBuffer,
    Uint32 bufferSize,
    SCMBDataPtr& ptr,
    SCMBMgmt_Header** pmem)
{

    // If buffer is not empty.
    if (bufferSize != 0 && theBuffer != 0)
    {

        Uint64 start;
        // Attention ! In this function a reallocation may take place.
        // The reference ptr may be unusable after the call to _getFreeSpace
        // --> use the returned start index.
        start = _getFreeSpace(ptr , bufferSize, pmem);
        // Copy buffer into SCMB
        memcpy(
            &((char*)(*pmem))[start],
            (const char*)theBuffer,
            bufferSize);
    }
    else
    {
        ptr.start = 0;
        ptr.size = 0;
    }
}

void _destroyExternalReferencesInternal(SCMBMgmt_Header* memHdr)
{

    Uint32 number = memHdr->numberExtRef;

    if (0 != number)
    {
        char * base = ((char*)memHdr);
        Uint64* array =
            (Uint64*)&(base[memHdr->extRefIndexArray.start]);
        for (Uint32 i = 0; i < number; i++)
        {
             delete ((SCMBUnion*)(&(base[array[i]])))->extRefPtr;
        }

    }
}

PEGASUS_NAMESPACE_END
