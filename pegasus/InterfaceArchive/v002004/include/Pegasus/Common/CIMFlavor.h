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
/* NOCHKSRC */

//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_Flavor_h
#define Pegasus_Flavor_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/**
    The CIMFlavor class implements the concept of the CIM qualifier flavor that
    encapsulates the propagation and override rules for qualifiers.
    The propagation rules define whether a qualifier
    may be propagated from classes to derived classes, or from classes to
    instances.  The override rules define whether or not derived classes may
    override a qualifier value or whether it must be fixed for an entire
    class hierarchy.

    The possible values are: OVERRIDABLE, TOSUBCLASS, TOINSTANCE,
    TRANSLATABLE, DISABLEOVERRIDE, ENABLEOVERRID, RESTRICTED and DEFAULTS.
    The flavor is a set of zero or more of these possible values.
 */
class PEGASUS_COMMON_LINKAGE CIMFlavor
{
public:

    /**
        Constructs a CIMFlavor object with no flavor values set (default
        constructor). The parameters and values are null.
     */
    CIMFlavor ();

    /**
        Constructs a CIMFlavor object from an existing CIMFlavor object (copy
        constructor).

        @param   flavor  Specifies the name of the CIMFlavor instance of
        the CIMFlavor object.
     */
    CIMFlavor (const CIMFlavor & flavor);

    /**
        Assigns the value of one CIMFlavor object to another (assignment
        operator).

        @param   flavor   Specifies the name of the CIMFlavor instance of
        the CIMFlavor object.

        @return  The new instance of the CIMFlavor object.
     */
    CIMFlavor & operator= (const CIMFlavor & flavor);

    /**
        Adds the set of flavor values of the specified CIMFlavor object to this
        CIMFlavor object.

        @param   flavor  Specifies the name of the CIMFlavor object that
        contains the flavor values to add to the CIMFlavor object.
     */
    void addFlavor (const CIMFlavor & flavor);

    /**
        Removes the specified set of flavor values from the CIMFlavor object.

        @param   flavor   Specifies the name of the CIMFlavor object that
        contains the flavor values to remove from the CIMFlavor object.
     */
    void removeFlavor (const CIMFlavor & flavor);

    /**
        Determines if every value in the specified CIMFlavor object is included
        in this CIMFlavor object.

        @param   flavor  Specifies the name of the CIMFlavor instance whose
        values you want to compare to the CIMFlavor object.

        @return  True if every value in the specified instance of the CIMFlavor
        object is included in the CIMFlavor object.  Otherwise, a value of
        false is returned.
     */
    Boolean hasFlavor (const CIMFlavor & flavor) const;

    /**
        Compares two CIMFlavor objects and determines whether or not they are equal.

        @param   flavor  Specifies the name of the CIMFlavor instance that you want
        to compare to the CIMFlavor object.

        @return True if the two CIMFlavor objects are equal.  Otherwise, a value
        of false is returned.
     */
    Boolean equal (const CIMFlavor & flavor) const;

    /**
        Combines two CIMFlavor objects. The parameters and values of the specified
        CIMFlavor instance are included (added) to the CIMFlavor object.

        @param   flavor   Specifies the name of the CIMFlavor instance to add to the
        CIMFlavor object.

        @return A new CIMFlavor object that represents the combination of this
                flavor with the specified flavor.
     */
    CIMFlavor operator+ (const CIMFlavor & flavor) const;

    /**
        Returns a String representation of the CIMFlavor object.
        Use this method to aide in debugging problems.  Depending on the implementation,
        the string format may vary.  The returned value cannot be null but may be empty.
     */
    String toString () const;

    /**
        Indicates that the qualifier has no flavors. Thus, the qualifier does not have
         any parameters nor values.
     */
    static const CIMFlavor NONE;

    /**
        Indicates that the qualifier may be overridden.
     */
    static const CIMFlavor OVERRIDABLE;
    /**
        Indicates that the override feature is enable for the qualifier. Thus, the
        qualifier can be overridden.
     */
    static const CIMFlavor ENABLEOVERRIDE;

    /**
        Indicates that the qualifier may not be overridden.
     */
    static const CIMFlavor DISABLEOVERRIDE;

    /**
        Indicates that the qualifier is propagated to the qualifier in the
    subclass with the same name.
     */
    static const CIMFlavor TOSUBCLASS;

    /**
        Indicates that the qualifier is not propagated to the qualifier in the
    subclass with the same name; it applies only to the class in which it
        is declared.
     */
    static const CIMFlavor RESTRICTED;

    /**
        Indicates that the qualifier is propagated to the qualifier in the
    instance with the same name.
     */
    static const CIMFlavor TOINSTANCE;

    /**
        Indicates that the qualifier is translatable (for internationalization).
     */
    static const CIMFlavor TRANSLATABLE;

    /**
        Indicates the default flavor settings. (OVERRIDABLE | TOSUBCLASS).
     */
    static const CIMFlavor DEFAULTS;

    /**
        Indicates both toSubclass and toInstance (TOSUBCLASS | TOINSTANCE).
     */
    static const CIMFlavor TOSUBELEMENTS;

private:

    /*
        Constructs an instance of the CIMFlavor object with the specified set of values.

        @param   flavor    A Uint32 representing the set of flavor
                           values to initialize with.
     */
    CIMFlavor (const Uint32 flavor);

    Uint32 cimFlavor;

    friend class BinaryStreamer;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_Flavor_h */
