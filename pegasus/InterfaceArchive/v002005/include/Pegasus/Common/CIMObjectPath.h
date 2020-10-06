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

#ifndef Pegasus_ObjectPath_h
#define Pegasus_ObjectPath_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/Array.h>
#include <Pegasus/Common/Exception.h>

PEGASUS_NAMESPACE_BEGIN

class CIMObjectPath;
class CIMKeyBindingRep;
class CIMObjectPathRep;
class CIMValue;

/**
    The CIMKeyBinding class associates a key name, value, and type.
    It is used to represent a key binding in a CIMObjectPath.
*/
class PEGASUS_COMMON_LINKAGE CIMKeyBinding
{
public:

    enum Type { BOOLEAN, STRING, NUMERIC, REFERENCE };

    /**
        Constructs a CIMKeyBinding object with null values.
    */
    CIMKeyBinding();

    /**
        Constructs a CIMKeyBinding object from the value of a specified
        CIMKeyBinding object.
        @param x The CIMKeyBinding object from which to construct a new
            CIMKeyBinding object.
    */
    CIMKeyBinding(const CIMKeyBinding& x);

    /**
        Constructs a CIMKeyBinding with a name, value, and type.
        @param name A CIMName containing the key name.
        @param value A String value for this key.
        @param type A CIMKeyBinding::Type specifying the type of this key.
    */
    CIMKeyBinding(const CIMName& name, const String& value, Type type);

    /**
        Constructs a CIMKeyBinding with a key name and CIMValue.  The key
        value and type are taken from the CIMValue.  CIM types are converted
        to key binding types using this mapping:

        <pre>
        boolean - BOOLEAN
        uint8 - NUMERIC
        sint8 - NUMERIC
        uint16 - NUMERIC
        sint16 - NUMERIC
        uint32 - NUMERIC
        sint32 - NUMERIC
        uint64 - NUMERIC
        sint64 - NUMERIC
        real32 - NUMERIC
        real64 - NUMERIC
        char16 - STRING
        string - STRING
        datetime - STRING
        reference - REFERENCE
        </pre>

        A value of type CIMTYPE_OBJECT cannot be used in a key binding.

        @param name A CIMName containing the key name.
        @param value A CIMValue specifying the value and type of this key.
        @exception TypeMismatchException If the type is not a valid key type,
            false otherwise.
    */
    CIMKeyBinding(const CIMName& name, const CIMValue& value);

    /**
        Destructs the CIMKeyBinding object.
    */
    ~CIMKeyBinding();

    /**
        Assigns the value of the specified CIMKeyBinding object to this object.
        @param x The CIMKeyBinding object from which to assign this
            CIMKeyBinding object.
        @return A reference to this CIMKeyBinding object.
    */
    CIMKeyBinding& operator=(const CIMKeyBinding& x);

    /**
        Gets the key name for the key binding.
        @return A CIMName containing the key name.
    */
    const CIMName& getName() const;

    /**
        Sets the key name for the key binding.
        @param name A CIMName containing the key name.
    */
    void setName(const CIMName& name);

    /**
        Gets the key value for the key binding.
        @return A String containing the key value.
    */
    const String& getValue() const;

    /**
        Sets the key value for the key binding.
        @param value A String containing the key value.
    */
    void setValue(const String& value);

    /**
        Gets the key type for the key binding.
        @return A CIMKeyBinding::Type containing the key type.
    */
    Type getType() const;

    /**
        Sets the key type for the key binding.
        @param type A CIMKeyBinding::Type containing the key type.
    */
    void setType(Type type);

    /**
        Compares the value and type of the key binding with a specified
        CIMValue.
        @param value The CIMValue to be compared.
        @return True if the value and type of the key binding are the same as
            the specified CIMValue, false otherwise.
    */
    Boolean equal(CIMValue value);

private:

    CIMKeyBindingRep* _rep;

    friend class CIMObjectPath;
};

/**
    Compares two key bindings for equality.
    @param x The first CIMKeyBinding to be compared.
    @param y The second CIMKeyBinding to be compared.
    @return True if the names, values, and types of the two key bindings are
        equivalent, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMKeyBinding& x,
    const CIMKeyBinding& y);

#define PEGASUS_ARRAY_T CIMKeyBinding
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

class XmlWriter;

/**
    The CIMObjectPath class represents the DMTF standard CIM object name or
    reference.  A reference is a property type that is used in an association.
    Consider this MOF example:

    <pre>
    [Association]
    class MyAssociations
    {
        MyClass ref from;
        MyClass ref to;
    };
    </pre>

    The value of the "from" and "to" properties are represented using a
    CIMObjectPath.

    A CIM reference is used to uniquely identify a CIM class or CIM instance
    object.  A CIMObjectPath consists of:

    <ul>
    <li>Host - name of host that contains the object</li>
    <li>NameSpace - the namespace which contains the object</li>
    <li>ClassName - name of objects class</li>
    <li>KeyBindings key/value pairs which uniquely identify an instance</li>
    </ul>

    CIM references may also be expressed as simple strings (as opposed to
    being represented by the CIMObjectPath class). This string is known as
    the "Object Name". An object name has the following form:

    <pre>
    &lt;namespace-path&gt;:&lt;model-path&gt;
    </pre>

    As for the model-path mentioned above, its form is defined by the CIM
    Standard (more is defined by the "XML Mapping Specification v2.0.0"
    specification) as follows:

    <pre>

    &lt;Qualifyingclass&gt;.&lt;key-1&gt;=&lt;value-1&gt;[,&lt;key-n&gt;=
    &lt;value-n&gt;]*
    </pre>

    For example:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    </pre>

    This presupposes the existence of a class called "TennisPlayer"
    that has key properties named "first" and "last". For example, here is what
    the MOF might look like:

    <pre>
    class TennisPlayer : Person
    {
        [key] string first;
        [key] string last;
    };
    </pre>

    All keys must be present in the model path.

    Now the namespace-type and model-path are combined in the following
    string object name.

    //atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter"

    Now suppose we wish to create a CIMObjectPath from this above string. There
    are two constructors provided: one which takes the above string and the
    other that takes the constituent elements. Here are the signature of the
    two constructors:

    <pre>
    CIMObjectPath(const String& objectName);

    CIMObjectPath(
        const String& host,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const Array<CIMKeyBinding>& keyBindings);
    </pre>

    Following our example, the above object name may be used to initialize
    a CIMObjectPath like this:

        <pre>
        CIMObjectPath ref =
            "//atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter";
        </pre>

    A CIMObjectPath may also be initialized using the constituent elements
    of the object name (sometimes the object name is not available as a string:
    this is the case with CIM XML encodings). The arguments shown in that
    constructor above correspond elements of the object name in the following
    way:

    <ul>
    <li>host = "atp:9999"</li>
    <li>nameSpace = "root/cimv25"</li>
    <li>className = "TennisPlayer"</li>
    <li>keyBindings = "first=\"Patrick\",last=\"Rafter\""</li>
    </ul>

    Note that the host and nameSpace argument may be empty since object names
    need not necessarily include a namespace path according to the standard.

    The key bindings must be built up by appending CIMKeyBinding objects
    to an Array of CIMKeyBindings like this:

    <pre>
    Array<CIMKeyBinding> keyBindings;
    keyBindings.append(CIMKeyBinding("first", "Patrick", CIMKeyBinding::STRING));
    keyBindings.append(CIMKeyBinding("last", "Rafter", CIMKeyBinding::STRING));
    </pre>

    Notice that the keys in the object name may appear in any order.
    That is the following object names refer to the same object:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    TennisPlayer.last="Rafter",first="Patrick"
    </pre>

    And since CIM is not case sensitive, the following refer to the same
    object:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    tennisplayer.FIRST="Patrick",Last="Rafter"
    </pre>

    Therefore, the CIMObjectPaths::operator==() would return true for the last
    two examples.

    The CIM standard leaves it an open question whether model paths may have
    spaces around delimiters (like '.', '=', and ','). We assume they cannot.
    So the following is an invalid model path:

    <pre>
    TennisPlayer . first = "Patrick", last="Rafter"
    </pre>

    We require that the '.', '=', and ',' have no spaces around them.

    For reasons of efficiency, the key bindings are internally sorted
    during initialization. This allows the key bindings to be compared
    more easily. This means that when the string is converted back to
    string (by calling toString()) that the keys may have been rearranged.

    There are two forms an object name can take:

    <pre>
    &lt;namespace-path&gt;:&lt;model-path&gt;
    &lt;model-path&gt;
    </pre>

    In other words, the namespace-path is optional. Here is an example of
    each:

    <pre>
    //atp:9999/root/cimv25:TennisPlayer.first="Patrick",last="Rafter"
    TennisPlayer.first="Patrick",last="Rafter"
    </pre>

    If it begins with "//" then we assume the namespace-path is present and
    process it that way.

    It should also be noted that an object name may refer to an instance or
    a class. Here is an example of each:

    <pre>
    TennisPlayer.first="Patrick",last="Rafter"
    TennisPlayer
    </pre>

    In the second case--when it refers to a class--the key bindings are
    omitted.
*/
class PEGASUS_COMMON_LINKAGE CIMObjectPath
{
public:

    /**
        Constructs a CIMObjectPath object with null values.
    */
    CIMObjectPath();

    /**
        Constructs a CIMObjectPath object from the value of a specified
        CIMObjectPath object.
        @param x The CIMObjectPath object from which to construct a new
            CIMObjectPath object.
    */
    CIMObjectPath(const CIMObjectPath& x);

    /**
        Constructs a CIMObjectPath from a CIM object name in String form.
        <p><b>Example:</b>
        <pre>
            CIMObjectPath r1 = "MyClass.z=true,y=1234,x=\"Hello World\"";
        </pre>
        @param objectName A String representation of the object name.
        @exception MalformedObjectNameException If the String does not contain
            a properly formed object name.
    */
    CIMObjectPath(const String& objectName);

    /**
        Constructs a CIMObjectPath object with the specified attributes.
        @param host A String containing the host name (e.g., "nemesis:5988").
            An empty String indicates that the object path does not contain
            a host name attribute.
        @param nameSpace A CIMNamespaceName specifying the namespace name.
            A null name indicates that the object path does not contain
            a namespace attribute.
        @param nameSpace A CIMName specifying the class name.
        @param keyBindings An Array of CIMKeyBinding objects specifying the
            key bindings.
        @exception MalformedObjectNameException If the host name String
            contains an improperly formed host name.
    */
    CIMObjectPath(
        const String& host,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const Array<CIMKeyBinding>& keyBindings = Array<CIMKeyBinding>());

    /**
        Destructs the CIMObjectPath object.
    */
    ~CIMObjectPath();

    /**
        Assigns the value of the specified CIMObjectPath object to this object.
        @param x The CIMObjectPath object from which to assign this
            CIMObjectPath object.
        @return A reference to this CIMObjectPath object.
    */
    CIMObjectPath& operator=(const CIMObjectPath& x);

    /**
        Resets the attributes of the object path to null values.
        The result is equivalent to using the default constructor.
    */
    void clear();

    /**
        Sets the CIMObjectPath with the specified attributes.
        @param host A String containing the host name (e.g., "nemesis:5988").
            An empty String indicates that the object path does not contain
            a host name attribute.
        @param nameSpace A CIMNamespaceName specifying the namespace name.
            A null name indicates that the object path does not contain
            a namespace attribute.
        @param nameSpace A CIMName specifying the class name.
        @param keyBindings An Array of CIMKeyBinding objects specifying the
            key bindings.
        @exception MalformedObjectNameException If the host name String
            contains an improperly formed host name.
    */
    void set(
        const String& host,
        const CIMNamespaceName& nameSpace,
        const CIMName& className,
        const Array<CIMKeyBinding>& keyBindings = Array<CIMKeyBinding>());

    /**
        Sets the CIMObjectPath from a CIM object name in String form.
        @param objectName A String representation of the object name.
        @exception MalformedObjectNameException If the String does not contain
            a properly formed object name.
    */
    void set(const String& objectName);

    /**
        Sets the CIMObjectPath from a CIM object name in String form.
        @param objectName A String representation of the object name.
        @exception MalformedObjectNameException If the String does not contain
            a properly formed object name.
    */
    CIMObjectPath& operator=(const String& objectName);

    /**
        Gets the host name for the object path.
        @return A String containing the host name.
    */
    const String& getHost() const;

    /**
        Sets the host name for the object path.
        <p><b>Example:</b>
        <pre>
        CIMObjectPath r1;
        r1.setHost("fred:5988");
        </pre>
        @param host A String containing the host name.
        @exception MalformedObjectNameException If the host name String
            contains an improperly formed host name.
    */
    void setHost(const String& host);

    /**
        Gets the namespace name for the object path.
        @return A CIMNamespaceName containing the namespace name.
    */
    const CIMNamespaceName& getNameSpace() const;

    /**
        Sets the namespace name for the object path.
        @param nameSpace A CIMNamespaceName containing the namespace name.
    */
    void setNameSpace(const CIMNamespaceName& nameSpace);

    /**
        Gets the class name for the object path.
        @return A CIMName containing the class name.
    */
    const CIMName& getClassName() const;

    /**
        Sets the class name for the object path.
        @param className A CIMName containing the class name.
    */
    void setClassName(const CIMName& className);

    /**
        Gets the key bindings for the object path.
        @return An Array of CIMKeyBinding objects containing the key bindings.
    */
    const Array<CIMKeyBinding>& getKeyBindings() const;

    /**
        Sets the key bindings for the object path.
        @param keyBindings An Array of CIMKeyBinding objects containing the
            key bindings.
    */
    void setKeyBindings(const Array<CIMKeyBinding>& keyBindings);

    /**
        Generates a String form of the object path.
        The format is:

        <pre>
            "//" + hostname + "/" + namespace + ":" + classname +"." +
            (keyname) + "=" (keyvalue) +"," ...
        </pre>

        Special characters are escaped in the resulting String.

        @return A String form of the object path.
        @exception UninitializedObjectException If the class name attribute
            of the object path is null.
    */
    String toString() const;

    /**
        Compares the CIMObjectPath with a specified CIMObjectPath.
        Comparisons of CIM names are case-insensitive, per the CIM
        specification.
        @param x The CIMObjectPath to be compared.
        @return True if this object is identical to the one specified,
            false otherwise.
    */
    Boolean identical(const CIMObjectPath& x) const;

    /**
        Generates a hash code for the object path.  Identical references
        generate the same hash code (despite insignificant differences such as
        the case of names and the order of the key bindings).
    */
    Uint32 makeHashCode() const;

private:

    /**
        Generates a canonical String form of the object path (in which key
        bindings are sorted and classnames and keynames are converted to
        lower case).
        @return A canonical String form of the object path.
        @exception UninitializedObjectException If the class name attribute
            of the object path is null.
    */
    String _toStringCanonical() const;

    CIMObjectPathRep* _rep;
};

/**
    Compares two object paths for equality.
    @param x The first CIMObjectPath to be compared.
    @param y The second CIMObjectPath to be compared.
    @return True if the object paths are identical, false otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator==(
    const CIMObjectPath& x,
    const CIMObjectPath& y);

/**
    Compares two object paths for inequality.
    @param x The first CIMObjectPath to be compared.
    @param y The second CIMObjectPath to be compared.
    @return False if the object paths are identical, true otherwise.
*/
PEGASUS_COMMON_LINKAGE Boolean operator!=(
    const CIMObjectPath& x,
    const CIMObjectPath& y);

#define PEGASUS_ARRAY_T CIMObjectPath
# include <Pegasus/Common/ArrayInter.h>
#undef PEGASUS_ARRAY_T

PEGASUS_NAMESPACE_END

#endif /* Pegasus_ObjectPath_h */


