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

#ifndef _SCMOCLASS_H_
#define _SCMOCLASS_H_


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SCMO.h>
#include <Pegasus/Common/CIMClass.h>
#include <Pegasus/Common/CIMClassRep.h>
#include <Pegasus/Common/CIMObjectRep.h>

PEGASUS_NAMESPACE_BEGIN


#define PEGASUS_SCMB_CLASS_MAGIC 0xF00FABCD

class PEGASUS_COMMON_LINKAGE SCMOClass
{
public:

    /**
     * Constructs a SCMOClass out of a CIMClass.
     * @param theCIMClass The source the SCMOClass is constructed off.
     * @param nameSpaceName The namespace for the class, optional.
     * @return
     */
    SCMOClass(const CIMClass& theCIMClass, const char* altNameSpace=0 );

    /**
     * Copy constructor for the SCMO class, used to implement refcounting.
     * @param theSCMOClass The class for which to create a copy
     * @return
     */
    SCMOClass(const SCMOClass& theSCMOClass )
    {
        cls.hdr = theSCMOClass.cls.hdr;
        Ref();
    }

    /**
     * Constructs a SCMOClass from a memory object of type SCMBClass_Main.
     * It incremets the referece counter of the memory object.
     * @param hdr A memory object of type SCMBClass_Main.
     **/
    SCMOClass(SCMBClass_Main* hdr)
    {
        cls.hdr = hdr;
        Ref();
    }

    /**
     * Assignment operator for the SCMO class,
     * @param theSCMOClass The right hand value
     **/
    SCMOClass& operator=(const SCMOClass& theSCMOClass)
    {
        if (cls.hdr != theSCMOClass.cls.hdr)
        {
            Unref();
            cls.hdr = theSCMOClass.cls.hdr;
            Ref();
        }
        return *this;
    }

    /**
     * Constructs an empty SCMOClass only with name space and class name.
     *
     * If you construct a SCMOInstance using this class, you must mark it as
     * compromized using SCMOInstance.markAsCompromised().
     *
     * @param className The name for the class.
     * @param nameSpaceName The namespace for the class.
     */
    SCMOClass(const char* className, const char* nameSpaceName );


    /**
     * Destructor is decrementing the refcount. If refcount is zero, the
     * singele chunk memory object is deallocated.
     */
    ~SCMOClass()
    {
        Unref();
    }

    /**
     * Converts the SCMOClass into a CIMClass.
     * It is a deep copy of the SCMOClass into the CIMClass.
     * @param cimClass An empty CIMClass.
     */
    void getCIMClass(CIMClass& cimClass) const;

    /**
     * Gets the key property names as a string array
     * @return An Array of String objects containing the names of the key
     * properties.
     */
    void getKeyNamesAsString(Array<String>& keyNames) const;

    /**
     * Gets the super class name of the class.
     * @retuns The super class name.
     *         If not available a NULL pointer is returned
     */
    const char* getSuperClassName() const;

    /**
     * Gets the super class name of the class.
     * @param Return strlen of result string.
     * @retuns The super class name.
     *         If not available a NULL pointer is returned
     */
    const char* getSuperClassName_l(Uint32 & length) const;

    static StrLit qualifierNameStrLit(Uint32 num)
    {
        return _qualifierNameStrLit[num];
    }

    /**
     * Determines if the SCMOClass is an empty class.
     * A empty class is a class with no information about properties.
     * Maybe only the class name and/or name space are available.
     * @return True if it an empty class.
     **/
    Boolean isEmpty( )const
    {
        // The size of one indicates that only an empty string was stored.
        return (cls.hdr->flags.isEmpty);
    }

    /**
     * Constructs an empty SCMOClass object.
     * This should be private but it is used in a test in tests/SCMO
     * so made it public.
     */
    SCMOClass();

private:

    void Ref()
    {
        cls.hdr->refCount++;
    };

    void Unref()
    {
        if (cls.hdr->refCount.decAndTestIfZero())
        {
            _destroyExternalReferences();
            free(cls.base);
            cls.base=0;
        }

    };


    inline void _initSCMOClass();

    void _destroyExternalReferences();

    SCMO_RC _getPropertyNodeIndex(Uint32& node, const char* name) const;
    SCMO_RC _getKeyBindingNodeIndex(Uint32& node, const char* name) const;

    void _setClassQualifers(const CIMQualifierList& theQualifierList);

    QualifierNameEnum  _setQualifier(
        Uint64 start,
        const CIMQualifier& theCIMQualifier);

    void _setClassProperties(PropertySet& theCIMProperties);

    void _setProperty(
        Uint64 start,
        Boolean* isKey,
        const CIMProperty& theCIMProperty);

    Boolean _setPropertyQualifiers(
        Uint64 start,
        const CIMQualifierList& theQualifierList);

    void _setClassKeyBinding(Uint64 start, const CIMProperty& theCIMProperty);
    void _insertPropertyIntoOrderedSet(Uint64 start, Uint32 newIndex);
    void _insertKeyBindingIntoOrderedSet(Uint64 start, Uint32 newIndex);

    void _setPropertyAsKeyInMask(Uint32 i);
    Boolean _isPropertyKey(Uint32 i);

    void _setValue(Uint64 start, const CIMValue& theCIMValue);

    QualifierNameEnum _getSCMOQualifierNameEnum(const CIMName& theCIMname);
    Boolean _isSamePropOrigin(Uint32 node, const char* origin) const;

    const char* _getPropertyNameAtNode(Uint32 propNode) const;

    SCMO_RC _isNodeSameType(
        Uint32 node,
        CIMType type,
        Boolean isArray,
        CIMType& realType) const;

    CIMProperty _getCIMPropertyAtNodeIndex(Uint32 nodeIdx) const;
    static void _getCIMQualifierFromSCMBQualifier(
        CIMQualifier& theCIMQualifier,
        const SCMBQualifier& scmbQualifier,
        const char* base);

    union{
        // To access the class main structure
        SCMBClass_Main *hdr;
        // To access the memory management header
        SCMBMgmt_Header  *mem;
        // Generic access pointer
        char *base;
    }cls;

    static const StrLit _qualifierNameStrLit[72];

    friend class SCMOInstance;
    friend class SCMODump;
    friend class SCMOXmlWriter;
    friend class SCMOClassCache;
    friend class SCMOStreamer;
};


PEGASUS_NAMESPACE_END

#endif
