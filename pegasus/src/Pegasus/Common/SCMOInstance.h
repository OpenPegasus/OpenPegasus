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
// This file defines the SCMOInstance interfaces. The implementation of
// these methods is in SCMO.cpp
//%/////////////////////////////////////////////////////////////////////////////

#ifndef _SCMOINSTANCE_H_
#define _SCMOINSTANCE_H_


#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/SCMO.h>
#include <Pegasus/Common/SCMOClass.h>
#include <Pegasus/Common/Union.h>

PEGASUS_NAMESPACE_BEGIN

PEGASUS_USING_STD;

#define PEGASUS_SCMB_INSTANCE_MAGIC 0xD00D1234

class PEGASUS_COMMON_LINKAGE SCMOInstance
{
public:

    /**
     * A SCMOInstance can only be created by a SCMOClass
     */
    SCMOInstance();

    /**
     * Creating a SCMOInstance using a SCMOClass.
     * @param baseClass A SCMOClass.
     */
    SCMOInstance(SCMOClass& baseClass);


    /**
     * Creating a SCMOInstance using a CIMClass
     * using an optional name space name,
     * @param baseClass A SCMOClass.
     * @param nameSpaceName An optional name space name.
     */
    SCMOInstance(CIMClass& theCIMClass, const char* nameSpaceName=0);

    /**
     * Copy constructor for the SCMO instance, used to implement refcounting.
     * @param theSCMOClass The instance for which to create a copy
     * @return
     */
    SCMOInstance(const SCMOInstance& theSCMOInstance )
    {
        inst.hdr = theSCMOInstance.inst.hdr;
        Ref();
    }

    /**
     * Constructs a SCMOInstance from a memory object of type SCMBInstance_Main.
     * It incremets the referece counter of the memory object.
     * @param hdr A memory object of type SCMBInstance_Main.
     **/
    SCMOInstance(SCMBInstance_Main* hdr)
    {
        inst.hdr = hdr;
        Ref();
    }


    /**
     * Assignment operator for the SCMO instance,
     * @param theSCMOInstance The right hand value
     **/
    SCMOInstance& operator=(const SCMOInstance& theSCMOInstance)
    {
        if (inst.hdr != theSCMOInstance.inst.hdr)
        {
            Unref();
            inst.hdr = theSCMOInstance.inst.hdr;
            Ref();
        }
        return *this;
    }

    /**
     * Destructor is decrementing the refcount. If refcount is zero, the
     * singele chunk memory object is deallocated.
     */
    ~SCMOInstance()
    {
        Unref();
    }

    /**
     * Builds a SCMOInstance based on this SCMOClass.
     * The method arguments determine whether qualifiers are included,
     * the class origin attributes are included,
     * and which properties are included in the new instance.
     * @param baseClass The SCMOClass of this instance.
     * @param includeQualifiers A Boolean indicating whether qualifiers in
     * the class definition (and its properties) are to be added to the
     * instance.  The TOINSTANCE flavor is ignored.
     * @param includeClassOrigin A Boolean indicating whether ClassOrigin
     * attributes are to be added to the instance.
     *
     * Note that this function does NOT generate an error if a property name
     * is supplied that is NOT in the class;
     * it simply does not add that property to the instance.
     *
     */
    SCMOInstance(
        SCMOClass& baseClass,
        Boolean includeQualifiers,
        Boolean includeClassOrigin);

    /**
     * Builds a SCMOInstance from the given SCMOClass and copies all
     * CIMInstance data into the new SCMOInstance.
     * @param baseClass The SCMOClass of this instance.
     * @param cimInstance A CIMInstace of the same class.
     * @exception Exception if class name does not match.
     * @exception Exception if a property is not part of class definition.
     * @exception Exception if a property does not match the class definition.
     */
    SCMOInstance(SCMOClass& baseClass, const CIMInstance& cimInstance);

    /**
     * Builds a SCMOInstance from the given SCMOClass and copies all
     * CIMObjectPath data into the new SCMOInstance.
     * @param baseClass The SCMOClass of this instance.
     * @param cimInstance A CIMObjectpath of the same class.
     * @exception Exception if class name does not match.
     */
    SCMOInstance(SCMOClass& baseClass, const CIMObjectPath& cimObj);

    /**
     * Builds a SCMOInstance from the given CIMInstance copying all data.
     * The SCMOClass is retrieved from SCMOClassCache using
     * the class and name space of the CIMInstance.
     * If the SCMOClass was not found, an empty SCMOInstance will be returned
     * and the resulting SCMOInstance is compromized.
     * If the CIMInstance does not contain a name space, the optional fall back
     * name space is used.
     * @param cimInstance A CIMInstace with class name and name space.
     * @param altNameSpace An alternative name space name.
     * @exception Exception if a property is not part of class definition.
     * @exception Exception if a property does not match the class definition.
     */
    SCMOInstance(
        const CIMInstance& cimInstance,
        const char* altNameSpace=0,
        Uint32 altNSLen=0);

    /**
     * Builds a SCMOInstance from the given CIMObjectPath copying all data.
     * The SCMOClass is retrieved from SCMOClassCache using
     * the class and name space of the CIMObjectPath.
     * If the SCMOClass was not found, an empty SCMOInstance will be returned
     * and the resulting SCMOInstance is compromized.
     * If the CIMObjectPath does not contain a name space,
     * the optional fall back name space is used.
     * @param cimObj A CIMObjectpath with name space and name
     * @param altNameSpace An alternative name space name.
     * @
     */
    SCMOInstance(
        const CIMObjectPath& cimObj,
        const char* altNameSpace=0,
        Uint32 altNSLen=0);

    /**
     * Builds a SCMOInstance from the given CIMObject copying all data.
     * The SCMOClass is retrieved from SCMOClassCache using
     * the class and name space of the CIMObject.
     * If the SCMOClass was not found, an empty SCMOInstance will be returned
     * and the resulting SCMOInstance is compromized.
     * If the CIMInstance does not contain a name space, the optional fall back
     * name space is used.
     * @param cimInstance A CIMInstace with class name and name space.
     * @param altNameSpace An alternative name space name.
     * @exception Exception if a property is not part of class definition.
     * @exception Exception if a property does not match the class definition.
     */
    SCMOInstance(
        const CIMObject& cimObject,
        const char* altNameSpace=0,
        Uint32 altNSLen=0);

    /**
     * Converts the SCMOInstance into a CIMInstance.
     * It is a deep copy of the SCMOInstance into the CIMInstance.
     * @param cimInstance An empty CIMInstance.
     */
    SCMO_RC getCIMInstance(CIMInstance& cimInstance) const;

    /**
     * Makes a deep copy of the instance.
     * This creates a new copy of the instance.
     * @param objectPathOnly If set to true, only the object path relevant parts
     *     host name and key bindings are part of the cloned instance.
     * @return A new copy of the SCMOInstance object.
     */
    SCMOInstance clone(Boolean objectPathOnly = false) const;

    /**
     * Retrieves the objectpath part of the SCMOInstance as an instance
     * of class CIMObjectPath.                .
     * @param cimObj Reference to an instantiated CIMObjectPath to be
     *     populated with the data from the SCMOInstance.
     * @return void
     */
    void getCIMObjectPath(CIMObjectPath& cimObj) const;

    /**
     * Returns the number of properties of the instance.
     * @param Number of properties
     */
    Uint32 getPropertyCount() const;

    /**
     * Gets the property name, type, and value addressed by a positional index.
     * The property name and value has to be copied by the caller !
     * @param pos The positional index of the property
     * @param pname Returns the property name as '\0' terminated string.
     *              Has to be copied by caller.
     *              It is set to NULL if rc != SCMO_OK.
     * @param pvalue Returns a pointer to the value of property.
     *               The value is stored in a SCMBUnion
     *                and has to be copied by the caller !
     *               It returns NULL if rc != SCMO_OK.
     *
     *               If the value is an array, the
     *               value array is stored in continuous memory.
     *               e.g. (SCMBUnion*)value[0 to size-1]
     *
     *               If the value is type of CIMTYPE_STRING,
     *               the string is referenced by the structure
     *               SCMBUnion.extString:
     *                       pchar contains the absolut pointer to the string
     *                       length contains the size of the string
     *                              without trailing '\0'.
     *               Only for strings the caller has to free pvalue !
     * @param type Returns the CIMType of the property
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param isArray Returns if the value is an array.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param size Returns the size of the array.
     *             If it is not an array, 0 is returned.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     *
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The value is a null value.
     *             SCMO_INDEX_OUT_OF_BOUND : Given index not found
     *
     */
    SCMO_RC getPropertyAt(
        Uint32 pos,
        const char** pname,
        CIMType& type,
        const SCMBUnion** pvalue,
        Boolean& isArray,
        Uint32& size ) const;

    // KS_FUTURE removed this as not used. Compiles without it
////  void getSCMBValuePropertyAt(
////      Uint32 pos,
////      SCMBValue** value,
////      const char ** valueBase,
////      SCMBClassProperty ** propDef,
////      const char ** classBase) const;

    /**
     * Gets the type and value of the named property.
     * The value has to be copied by the caller !
     * @param name The property name
     * @param pvalue Returns a pointer to the value of property.
     *               The value is stored in a SCMBUnion
     *                and has to be copied by the caller !
     *               It returns NULL if rc != SCMO_OK.
     *
     *               If the value is an array, the
     *               value array is stored in continuous memory.
     *               e.g. (SCMBUnion*)value[0 to size-1]
     *
     *               If the value is type of CIMTYPE_STRING,
     *               the string is referenced by the structure
     *               SCMBUnion.extString:
     *                       pchar contains the absolut pointer to the string
     *                       length contains the size of the string
     *                              without trailing '\0'.
     *               Only for strings the caller has to free pvalue !
     * @param type Returns the CIMType of the property
     *             It is invalid if rc == SCMO_NOT_FOUND.
     * @param isArray Returns if the value is an array.
     *             It is invalid if rc == SCMO_NOT_FOUND.
     * @param size Returns the size of the array.
     *             If it is not an array, 0 is returned.
     *             It is invalid if rc == SCMO_NOT_FOUND.
     *
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The value is a null value.
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC getProperty(
        const char* name,
        CIMType& type,
        const SCMBUnion** pvalue,
        Boolean& isArray,
        Uint32& size ) const;

    /**
     * Set/replace a property in the instance.
     * If the class origin is specified, it is honored when the
     * property is identified within the instance.
     *
     * Note: Only properties which are already part of the instance/class can
     * be set/replaced except in for special case where the flag
     * lags.noClassForInstance is set when the instance is created.
     * If the class is not found in the repository, the instance is
     * are marked as noClassForInstance and properties are set in
     * the UserDefined properties area.
     * @param name The name of the property to be set.
     * @param theType The CIMType of the property
     * @param value A pointer to the value to be set at the named property.
     *              The value must be in a SCMBUnion.
     *              The value is copied into the instance
     *              If the value == NULL, a null value is assumed.
     *              If the value is an array, the value array has to be
     *              stored in continuous memory.
     *              e.g. (SCMBUnion*)value[0 to size-1]
     *
     *              To store an array of size 0, The value pointer has to
     *              not NULL ( value != NULL ) but the size has to be 0
     *              (size == 0).
     *
     *              If the value is type of CIMTYPE_STRING,
     *              the string is referenced by the structure
     *              SCMBUnion.extString:
     *                       pchar contains the absolute pointer to
     *                       the string length contains the size of
     *                       the string
     *                              without trailing '\0'.
     * @param isArray Indicate that the value is an array. Default false.
     * @param size Returns the size of the array. If not an array this
     *         this parameter is ignored. Default 0.
     * @param origin The class originality of the property.
     *               If NULL, then it is ignored. Default NULL.
     * @return     SCMO_OK
     *             SCMO_NOT_SAME_ORIGIN : The property name was found, but
     *                                    the origin was not the same.
     *             SCMO_NOT_FOUND : Given property name not found.
     *             SCMO_WRONG_TYPE : Named property has the wrong type.
     *             SCMO_NOT_AN_ARRAY : Named property is not an array.
     *             SCMO_IS_AN_ARRAY  : Named property is an array.
     */
    SCMO_RC setPropertyWithOrigin(
        const char* name,
        CIMType theType,
        const SCMBUnion* value,
        Boolean isArray=false,
        Uint32 size = 0,
        const char* origin = NULL);

    /**
     * Rebuild of the key bindings from the property values
     * if no or incomplete key properties are set on the instance.
     * @exception NoSuchProperty
     */
    void buildKeyBindingsFromProperties();

    /**
     * Gets the hash index for the named property. Filtering is ignored.
     * @param theName The property name
     * @param pos Returns the hash index.
     * @return     SCMO_OK
     *             SCMO_INVALID_PARAMETER: name was a NULL pointer.
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC getPropertyNodeIndex(const char* name, Uint32& pos) const;

    // KS_FUTURE confirm that the following is used in OpenPegasus
    // Apprently never referenced within any of OpenPegasus code
    /**
     * Set/replace a property in the instance at node index.
     * @param index The node index.
     * @param type The CIMType of the property
     * @param pInVal A pointer to the value to be set at the named property.
     *               The value has to be in a SCMBUnion.
     *               The value is copied into the instance
     *               If the value == NULL, a null value is assumed.
     *               If the value is an array, the value array has to be
     *               stored in continuous memory.
     *               e.g. (SCMBUnion*)value[0 to size-1]
     *
     *              To store an array of size 0, The value pointer has to
     *               not NULL ( value != NULL ) but the size has to be 0
     *                (size == 0).
     *
     *               If the value is type of CIMTYPE_STRING,
     *               the string is referenced by the structure
     *               SCMBUnion.extString:
     *                        pchar contains the absolut pointer to the string
     *                       length contains the size of the string
     *                                without trailing '\0'.
     * @param isArray Indicate that the value is an array. Default false.
     * @param size The size of the array. If not an array this
     *         this parameter is ignorer. Default 0.
     * @return     SCMO_OK
     *             SCMO_INDEX_OUT_OF_BOUND : Given index not found
     *             SCMO_WRONG_TYPE : The property at given node index
     *                               has the wrong type.
     *             SCMO_NOT_AN_ARRAY : The property at given node index
     *                                 is not an array.
     *             SCMO_IS_AN_ARRAY  : The property at given node index
     *                                 is an array.
     */
    SCMO_RC setPropertyWithNodeIndex(
        Uint32 node,
        CIMType type,
        const SCMBUnion* pInVal,
        Boolean isArray=false,
        Uint32 size = 0);

    /**
     * Set/replace the named key binding using binary data
     * @param name The key binding name.
     * @param type The type as CIMType.
     * @param keyvalue A pointer to the binary key value.
     *         The value is copied into the instance
     *         If the value == NULL, a null value is assumed.
     * @param keyvalue A pointer to the value to be set at the key binding,
     *               The keyvalue has to be in a SCMBUnion.
     *               The keyvalue is copied into the instance.
     *               If the keyvalue == NULL, a null value is assumed.
     *
     *               If the keyvalue is type of CIMTYPE_STRING,
     *               the string is referenced by the structure
     *               SCMBUnion.extString:
     *                        pchar contains the absolut pointer to the string
     *                       length contains the size of the string
     *                                without trailing '\0'.
     * @return     SCMO_OK
     *             SCMO_INVALID_PARAMETER : Given name or pvalue
     *                                      is a NULL pointer.
     *             SCMO_TYPE_MISSMATCH : Given type does not
     *                                   match to key binding type
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC setKeyBinding(
        const char* name,
        CIMType type,
        const SCMBUnion* keyvalue);

    /**
     * Set/replace the key binding at node
     * @param node The node index of the key.
     * @param type The type as CIMType.
     * @param keyvalue A pointer to the value to be set at the key binding,
     *               The keyvalue has to be in a SCMBUnion.
     *               The keyvalue is copied into the instance.
     *               If the keyvalue == NULL, a null value is assumed.
     *
     *               If the keyvalue is type of CIMTYPE_STRING,
     *               the string is referenced by the structure
     *               SCMBUnion.extString:
     *                        pchar contains the absolut pointer to the string
     *                       length contains the size of the string
     *                                without trailing '\0'.
     * @return     SCMO_OK
     *             SCMO_INVALID_PARAMETER : Given pvalue is a NULL pointer.
     *             SCMO_TYPE_MISSMATCH : Given type does not
     *                                   match to key binding type
     *             SCMO_INDEX_OUT_OF_BOUND : Given index is our of range.
     */
    SCMO_RC setKeyBindingAt(
        Uint32 node,
        CIMType type,
        const SCMBUnion* keyvalue);

    /**
     * Clears all key bindings in an instance.
     * Warning: External references are freed but only the internal
     * control structures are resetted. No memory is freed and at setting
     * new key bindings the instance will grow in memory usage.
     **/
    void clearKeyBindings();

    /**
     * Gets the key binding count.
     * @return the number of key bindings set.
     */
    Uint32 getKeyBindingCount() const;

    /**
     * Get the indexed key binding.
     * @parm idx The key bining index
     * @parm pname Returns the name.
     *             Has to be copied by caller.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param type Returns the type as CIMType.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param keyvalue A pointer to the binary key value.
     *             Has to be copied by caller.
     *             It is only valid if rc == SCMO_OK.
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The key binding is not set.
     *             SCMO_INDEX_OUT_OF_BOUND : Given index not found
     *
     */
    SCMO_RC getKeyBindingAt(
        Uint32 idx,
        const char** pname,
        CIMType& type,
        const SCMBUnion** keyvalue) const;

    SCMO_RC getKeyBindingAtUnresolved(
        Uint32 node,
        const char** pname,
        Uint32 & pnameLen,
        CIMType& type,
        const SCMBUnion** pdata,
        const char** valueBase) const;

    /**
     * Get the named key binding.
     * @parm name The name of the key binding.
     * @param type Returns the type as CIMType.
     *             It is invalid if rc == SCMO_INDEX_OUT_OF_BOUND.
     * @param keyvalue Returns a pointer to the value of keybinding.
     *               The value is stored in a SCMBUnion
     *                and has to be copied by the caller !
     *               It returns NULL if rc != SCMO_OK.
     *
     *               If the value is type of CIMTYPE_STRING,
     *               the string is referenced by the structure
     *               SCMBUnion.extString:
     *                       pchar contains the absolut pointer to the string
     *                       length contains the size of the string
     *                              without trailing '\0'.
     *               Only for strings the caller has to free pvalue !
     * @param keyvalue A pointer to the binary key value.
     *             Has to be copied by caller.
     *             It is only valid if rc == SCMO_OK.
     * @return     SCMO_OK
     *             SCMO_NULL_VALUE : The key binding is not set.
     *             SCMO_NOT_FOUND : Given property name not found.
     */
    SCMO_RC getKeyBinding(
        const char* name,
        CIMType& ptype,
        const SCMBUnion** keyvalue) const;

    /**
     * Determines whether the c++ object has been initialized.
     * @return True if the c++ object has not been initialized, false otherwise.
     */
    Boolean isUninitialized( ) const {return (0 == inst.base); };

    /**
     * Determines if the SCMOInstance does not contain any property information.
     * Maybe only the class name and/or name space are available.
     * @return True if the SCMOInstance is empty, false otherwise.
     */
    Boolean isEmpty( ) const;

    /**
     * Determines whether the instance is used as a class container.
     * @return True if the instance is used as a class container only.
     */
    Boolean getIsClassOnly( ) const
    {
        return inst.hdr->flags.isClassOnly;
    }

    /**
     * To mark if this instance is a class only container.
     */
    void setIsClassOnly( Boolean b )
    {
        inst.hdr->flags.isClassOnly = b;
    }

    /**
     * Determies if two objects are referencing to the same instance
     * @return True if the objects are referencing to the some instance.
     */
    Boolean isSame(SCMOInstance& theInstance) const;

    /**
     * Sets the provided host name at the instance.
     * @param hostName The host name as UTF8.
     */
    void setHostName(const char* hostName);

    /**
     * Get the host name of the instance. The caller has to make a copy !
     * @return The host name as UTF8.
     */
    const char* getHostName() const;

    /**
     * Get the host name of the instance.
     * @param Return strlen of result string.
     * @return The class name as UTF8.
     */
    const char* getHostName_l(Uint32 & length) const;

    /**
     * Sets the provided class name at the instance. By caling this function
     * the instance is in an inconsitacne state and is maked as isCompromised.
     * @param className The class name as UTF8.
     */
    void setClassName(const char* className);

    /**
     * Sets the provided class name at the instance. By caling this function
     * the instance is in an inconsitacne state and is maked as isCompromised.
     * @param className The class name as UTF8.
     * @param len The strlen of the name space.
     */
    void setClassName_l(const char* className, Uint32 len);

    /**
     * Get the class name of the instance. The caller has to make a copy !
     * @return The class name as UTF8.
     */
    const char* getClassName() const;

    /**
     * Get the class name of the instance. The caller has to make a copy !
     * @param lenght Return strlen of result string.
     * @return The class name as UTF8.
     */
    const char* getClassName_l(Uint32 & length) const;

    /**
     * Sets the provided name space name at the instance.
     * By caling this function the instance is in an inconsitacne state and
     * is maked as isCompromised.
     * @param nameSpaceName The name space name as UTF8.
     */
    void setNameSpace(const char* nameSpace);

    /**
     * Sets the provided name space name unchecked at the instance.
     * By caling this function the instance is in an inconsitacne state and
     * is maked as isCompromised.
     * @param nameSpaceName The name space name as UTF8.
     * @param len The strlen of the name space.
     */
    void setNameSpace_l(const char* nameSpace, Uint32 len);

    /**
     * Get the name space of the instance. The caller has to make a copy !
     * @return The name space as UTF8.
     */
    const char* getNameSpace() const;

    /**
     * Get the class name of the instance. The caller has to make a copy !
     * @param Return strlen of result string.
     * @return The class name as UTF8.
     */
    const char* getNameSpace_l(Uint32 & length) const;

    /**
     * If hostname or namespace of the SCMOInstance are NULL or empty string,
     * replace them with the given input.
     * @param hn The host name to apply to the SCMOInstance.
     * @param hnLen The length of the hostname in byte without closing zero.
     * @param ns The namespace name to apply to the SCMOInstance.
     * @param nsLen The length of the hostname in byte without closing zero.
     */
    void completeHostNameAndNamespace(
        const char* hn,
        Uint32 hnLen,
        const char* ns,
        Uint32 nsLen);

    /**
     * Is the name space or class name of the instance the origianl values
     * set by the used SCMOClass.
     * The class name and/or name space may differ with the associated class.
     * @return true if name space or class name was set manually by
     *          setNameSpace() or setClassName()
     */
    Boolean isCompromised() const
    {
        return inst.hdr->flags.isCompromised;
    };

    /**
        returns true if there is no class defined for this instance.
        The flag is set when the SCMO instance is created and the
        repository returns no class.
    */
    Boolean noClassForInstance() const
    {
        return inst.hdr->flags.noClassForInstance;
    };

    /**
     * Mark the instance as a non validated instance.
     */
    void markAsCompromised()
    {
        inst.hdr->flags.isCompromised = true;
    };

    /**
        Sets the flag indicating that there is no class for this
        instance. This property is used for Instances that are
        created where there is no class in the repository.
    */
    void markNoClassForInstance(Boolean x)
    {
        inst.hdr->flags.noClassForInstance = x;
    };

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to include qualifiers for this instance.
     */
    void includeQualifiers()
    {
        inst.hdr->flags.includeQualifiers = true;
    };

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to NOT to include (exclude) qualifiers for this instance.
     */
    void excludeQualifiers()
    {
        inst.hdr->flags.includeQualifiers = false;
    }

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to include class origins for this instance.
     */
    void includeClassOrigins()
    {
        inst.hdr->flags.includeClassOrigin = true;
    };

    /**
     *  To indicate the export processing ( eg. XMLWriter )
     *  to NOT to include (exclude) class origins for this instance.
     */
    void excludeClassOrigins()
    {
        inst.hdr->flags.includeClassOrigin = false;
    }


    /**
     * Returns the number of external references hosted by the instance.
     **/
    Uint32 numberExtRef() const
    {
        return inst.mem->numberExtRef;
    }

    /**
     * Gets the pointer of an external reference of the instance.
     * Warning: The pointer is purely returned. No management is done.
     * @parm idx The index of the external reference.
     **/
    SCMOInstance* getExtRef(Uint32 idx) const;

    /**
     * Sets a pointer of an external reference of the instance.
     * Warning: The pointer is purely returned. No management is done.
     * @parm idx The index of the external reference.
     * @parm ptr The pointer to an SCMOInstance
     **/
    void putExtRef(Uint32 idx,SCMOInstance* ptr);

private:

    void Ref()
    {
        inst.hdr->refCount++;
    };

    void Unref()
    {
        if (inst.hdr->refCount.decAndTestIfZero())
        {
            // All external references have to be destroyed.
            _destroyExternalReferences();
            // The class has also be dereferenced.
            delete inst.hdr->theClass.ptr;
            free(inst.base);
            inst.base=NULL;
        }

    };


    void _copyOnWrite()
    {
        if ( 1 < inst.hdr->refCount.get() )
        {
            SCMBInstance_Main * oldRef = inst.hdr;
            SCMBMgmt_Header* oldMgmt = inst.mem;

            _clone();
            if (oldRef->refCount.decAndTestIfZero())
            {
                // All external references have to be destroyed.
                _destroyExternalReferencesInternal(oldMgmt);
                // The class has also be dereferenced.
                delete oldRef->theClass.ptr;
                free((void*)oldRef);
                oldRef=0;
            }
        }
    };

    void _clone();

    void _destroyExternalReferences();

    void _destroyExternalKeyBindings();

    void _copyExternalReferences();

    void _setExtRefIndex(Uint64 idx);

    void _initSCMOInstance(SCMOClass* pClass);

    void _setCIMInstance(const CIMInstance& cimInstance);

    // Internal but used by friend class SCMOXmlWriter.cpp
    // This function accounts for user-defined and class-defined properties
    void _getPropertyAt(
        Uint32 pos,
        SCMBValue** value,
        const char ** valueBase,
        SCMBClassProperty ** propDef) const;

    SCMO_RC _getPropertyAtNodeIndex(
        Uint32 pos,
        const char** pname,
        CIMType& type,
        const SCMBUnion** pvalue,
        Boolean& isArray,
        Uint32& size ) const;

    void _setPropertyAtNodeIndex(
        Uint32 pos,
        CIMType type,
        const SCMBUnion* pInVal,
        Boolean isArray,
        Uint32 size);

    void _setCIMValueAtNodeIndex(
        Uint32 node,
        CIMValueRep* valRep,
        CIMType realType);

    static void _getCIMValueFromSCMBUnion(
        CIMValue& cimV,
        const CIMType type,
        const Boolean isNull,
        const Boolean isArray,
        const Uint32 arraySize,
        const SCMBUnion& scmbUn,
        const char * base);

    static void _getCIMValueFromSCMBValue(
        CIMValue& cimV,
        const SCMBValue& scmbV,
        const char * base);

    static SCMOClass _getSCMOClass(
        const CIMObjectPath& theCIMObj,
        const char* altNS,
        Uint32 altNSlength);

    CIMProperty _getCIMPropertyAtNodeIndex(Uint32 nodeIdx) const;

    void _setCIMObjectPath(const CIMObjectPath& cimObj);

    SCMBUnion* _resolveSCMBUnion(
        CIMType type,
        Boolean isArray,
        Uint32 size,
        Uint64 start,
        char* base) const;

    void _setSCMBUnion(
        const SCMBUnion* pInVal,
        CIMType type,
        Boolean isArray,
        Uint32 size,
        SCMBUnion & u);

    static void _setUnionValue(
        Uint64 start,
        SCMBMgmt_Header** pmem,
        CIMType type,
        Uint64 startNS,
        Uint32 lenNS,
        Union& u);

    static void _setUnionArrayValue(
        Uint64 start,
        SCMBMgmt_Header** pmem,
        CIMType type,
        Uint32& n,
        Uint64 startNS,
        Uint32 lenNS,
        Union& u);

    static void _setExtRefIndex(SCMBUnion* pInst, SCMBMgmt_Header** pmem);

    SCMO_RC _getKeyBindingDataAtNodeIndex(
        Uint32 node,
        const char** pname,
        Uint32 & pnameLen,
        CIMType& type,
        const SCMBUnion** pdata) const;

    void _copyKeyBindings(SCMOInstance& targetInst) const;

    void _setKeyBindingFromSCMBUnion(
        CIMType type,
        const SCMBUnion& u,
        const char * uBase,
        SCMBKeyBindingValue& keyData);

    SCMO_RC _setKeyBindingFromString(
        const char* name,
        CIMType type,
        String cimKeyBinding);

    SCMBUserKeyBindingElement* _getUserDefinedKeyBinding(
        const char* name,
        Uint32 nameLen,
        CIMType type);

    void _setUserDefinedKeyBinding(
        SCMBUserKeyBindingElement& theInsertElement,
        char* elementBase);
    /**
     * Set a SCMO user defined key binding using the class CIM type tolerating
     * CIM key binding types converted to CIM types by function
     *  _CIMTypeFromKeyBindingType().
     *
     * @parm classType The type of the key binding in the class definition
     * @parm setType The type of the key binding to be set.
     * @param keyValue A pointer to the key binding to be set.
     * @param kbValue Out parameter, the SCMO keybinding to be set.
     *
     **/
    SCMO_RC _setKeyBindingTypeTolerate(
        CIMType classType,
        CIMType setType,
        const SCMBUnion* keyValue,
        SCMBKeyBindingValue& kbValue);

    CIMType _CIMTypeFromKeyBindingType(
        const char* key,
        CIMKeyBinding::Type t);

    SCMO_RC _getUserKeyBindingNodeIndex(Uint32& node, const char* name) const;

    SCMBUserKeyBindingElement* _getUserDefinedKeyBindingAt(Uint32 index) const;

    Boolean _setCimKeyBindingStringToSCMOKeyBindingValue(
        const String& kbs,
        CIMType type,
        SCMBKeyBindingValue& scmoKBV
        );

    // Functions to support the use of user defined properties
    /*
        Get the node for a user defined property.
        @return node of the property and SCMO_OK or an error
        SCMO_NOT_FOUND indicating that the property cannot be
        found as a user-defined property.
    */
    SCMO_RC _getUserPropertyNodeIndex(Uint32& node, const char* name) const;

    /*
        get the Instance SCMBUserPropertyElement for the defined node index.
    */
    SCMBUserPropertyElement*
         _getUserDefinedPropertyElementAt(Uint32 index) const;

    void _setPropertyInUserDefinedElement(
        SCMBUserPropertyElement* ptrNewElement,
        CIMType theType,
        const SCMBUnion* pinVal,
        Boolean isArray,
        Uint32 size);
    /**
        Creates a new user-defined property element, chains it to
        the existing user-defined property chain, and populate it
        with name, type, isArray. isSet = false.
        @return pointer to new SCMBUserPropertyElement
    */
    SCMBUserPropertyElement* _createNewUserDefinedProperty(
        const char * name,
        Uint32 nameLen,
        CIMType theType);

    /*
        Returns true if the Property is in the set of
        properties defined in an including class
    */
    Boolean _isClassDefinedProperty(Uint32 node) const;

    SCMBValue& _getSCMBValueForNode(Uint32 node) const;

    /* Definition of inst. Pointer to:
            SCMBInstance_Manin
            SCMBMgmt_Header
            or generic base pointer.
    */
    union{
        // To access the instance main structure
        SCMBInstance_Main *hdr;
        // To access the memory management header
        SCMBMgmt_Header     *mem;
        // Generic access pointer
        char *base;
    }inst;

    friend class SCMOClass;
    friend class SCMODump;
    friend class SCMOXmlWriter;
    friend class SCMOStreamer;
};


inline SCMBValue& SCMOInstance::_getSCMBValueForNode(Uint32 node) const
{
    if (_isClassDefinedProperty(node))
    {
        SCMBValue *theInstPropNodeArray =
            (SCMBValue*)&(inst.base[inst.hdr->propertyArray.start]);

        return theInstPropNodeArray[node];
    }

    SCMBUserPropertyElement* pElement =
       _getUserDefinedPropertyElementAt(node);

    return pElement->value;
}


// KS_TODO clarify this test since our rule is more explicit
// We cannot have both user-defined and class-defined properties so this
// could be a more concrete tests.
inline Boolean SCMOInstance::_isClassDefinedProperty(Uint32 node) const
{
    return (node < inst.hdr->numberProperties);
}

// This internal function used by SCMOXmlWriter.cpp
// It is inline because it is called only once in the CIMServer.
inline void SCMOInstance::_getPropertyAt(
    Uint32 pos,
    SCMBValue** value,
    const char ** valueBase,
    SCMBClassProperty ** propertyDef) const
{
    if (_isClassDefinedProperty(pos))
    {
        SCMBValue *theInstPropNodeArray =
            (SCMBValue*)&(inst.base[inst.hdr->propertyArray.start]);

        // create a pointer to property node array of the class.
        Uint64 idx =
            inst.hdr->theClass.ptr->cls.hdr->propertySet.nodeArray.start;
        SCMBClassPropertyNode* theClassPropNodeArray =
            (SCMBClassPropertyNode*)&(inst.hdr->theClass.ptr->cls.base)[idx];

        // return the absolute pointer to the property definition
        *propertyDef = &(theClassPropNodeArray[pos].theProperty);

        // need check if property set or not, if not set use the default value
        if (theInstPropNodeArray[pos].flags.isSet)
        {
            // return the absolute pointer to the property value in the instance
            *value = &(theInstPropNodeArray[pos]);
            *valueBase = inst.base;
        }
        else
        {
            // return the absolute pointer to
            *value = &(theClassPropNodeArray[pos].theProperty.defaultValue);
            *valueBase = inst.hdr->theClass.ptr->cls.base;
        }
    }
    else           // User-defined property
    {
        // KS_TODO mostly same code as _getPropertyAtNodeIndex
        SCMBUserPropertyElement* pElement =
            _getUserDefinedPropertyElementAt(pos);

        PEGASUS_ASSERT(pElement != 0);

        *value = &(pElement->value);
        // KS_TODO do we have to deal with isSet == false here. Probably
        // no but confirm with Assert.
        *valueBase = inst.base;
    }
}

inline Uint32 SCMOInstance::getPropertyCount() const
{
    return(inst.hdr->numberProperties + inst.hdr->numberUserProperties);
}

// KS_FUTURE Remove this completely from code since apparently not used.
////// Apparently never used.
////inline void SCMOInstance::getSCMBValuePropertyAt(
////    Uint32 pos,
////    SCMBValue** value,
////    const char ** valueBase,
////    SCMBClassProperty ** propDef,
////    const char ** propDefBase) const
////{
////    _getPropertyAt(pos,value,valueBase,propDef);
////
////    *propDefBase = inst.hdr->theClass.ptr->cls.base;
////}

inline SCMO_RC SCMOInstance::getKeyBindingAtUnresolved(
        Uint32 node,
        const char** pname,
        Uint32 & pnameLen,
        CIMType& type,
        const SCMBUnion** pdata,
        const char** valueBase) const
{
    SCMO_RC rc = _getKeyBindingDataAtNodeIndex(node,pname,pnameLen,type,pdata);
    // Adjust size to string length
    if (pnameLen)
    {
        pnameLen--;
    }
    *valueBase = inst.base;
    return rc;
}

inline Boolean SCMOInstance::isEmpty( ) const
{
    if (noClassForInstance())
    {
        // KS_TODO need another test for empty. Number properties
        // != 0 might work but that would disallow an empty embedded
        // instance.
        return false;
    }
    else
    {
        return (inst.hdr->theClass.ptr->isEmpty());
    }
}



#define PEGASUS_ARRAY_T SCMOInstance
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END


#endif
