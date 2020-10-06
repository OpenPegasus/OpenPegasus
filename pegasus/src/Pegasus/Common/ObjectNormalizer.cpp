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

#include <Pegasus/Common/ObjectNormalizer.h>
#include <Pegasus/Common/ArrayInternal.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_NAMESPACE_BEGIN

Boolean ObjectNormalizer::_enableNormalization = false;

void ObjectNormalizer::setEnableNormalization(Boolean value)
{
    _enableNormalization = value;
}

Boolean ObjectNormalizer::getEnableNormalization()
{
    return _enableNormalization;
}

CIMQualifier _processQualifier(
    CIMConstQualifier& referenceQualifier,
    CIMConstQualifier& cimQualifier)
{
    // check name
    if (!referenceQualifier.getName().equal(cimQualifier.getName()))
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_QUALIFIER_NAME",
            "Invalid qualifier name: $0",
            cimQualifier.getName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }

    // check type
    if (referenceQualifier.getType() != cimQualifier.getType())
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_QUALIFIER_TYPE",
            "Invalid qualifier type: $0",
            cimQualifier.getName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }

    CIMQualifier normalizedQualifier(
        referenceQualifier.getName(),
        referenceQualifier.getValue(),  // default value
        referenceQualifier.getFlavor(),
        referenceQualifier.getPropagated() == 0 ? false : true);

    // TODO: check override

    // update value
    if (!cimQualifier.getValue().isNull())
    {
        normalizedQualifier.setValue(cimQualifier.getValue());
    }

    return normalizedQualifier;
}

CIMProperty ObjectNormalizer::processProperty(
    CIMConstProperty& referenceProperty,
    CIMConstProperty& instProperty,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    NormalizerContext* context,
    const CIMNamespaceName& nameSpace)
{
    // check name
    if (!referenceProperty.getName().equal(instProperty.getName()))
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_PROPERTY_NAME",
            "Invalid property name: $0",
            instProperty.getName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }

    // check type
    CIMType referencePropType = referenceProperty.getType();
    CIMType instPropType = instProperty.getType();
    if (referencePropType != instPropType)
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_PROPERTY_TYPE",
            "Invalid property type: $0",
            instProperty.getName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }

    // TODO: check array size?

    CIMProperty normalizedProperty(
        referenceProperty.getName(),
        referenceProperty.getValue(),   // default value
        referenceProperty.getArraySize(),
        referenceProperty.getReferenceClassName(),
        CIMName(),
        false);

    // TODO: check override (especially for references)?

    // update value
    if (!instProperty.getValue().isNull())
    {
        normalizedProperty.setValue(instProperty.getValue());
    }

    // update class origin
    if (includeClassOrigin)
    {
        normalizedProperty.setClassOrigin(referenceProperty.getClassOrigin());
    }

    // add qualifiers
    if (includeQualifiers)
    {
        // propagate class property qualifiers
        for (Uint32 i=0, n = referenceProperty.getQualifierCount(); i < n; i++)
        {
            CIMConstQualifier referenceQualifier =
                referenceProperty.getQualifier(i);

            Uint32 pos =
                instProperty.findQualifier(referenceQualifier.getName());

            // update value if qualifier is present in the specified property
            if (pos != PEG_NOT_FOUND)
            {
                CIMConstQualifier cimQualifier = instProperty.getQualifier(pos);

                CIMQualifier normalizedQualifier =
                    _processQualifier(
                        referenceQualifier,
                        cimQualifier);

                normalizedProperty.addQualifier(normalizedQualifier);
            }
            else
            {
                normalizedProperty.addQualifier(referenceQualifier.clone());
            }
        }
    }
#ifdef PEGASUS_SNIA_INTEROP_COMPATIBILITY
    else if (referenceProperty.getType() == CIMTYPE_INSTANCE)
    {
        Uin32 refPos = referenceProperty.findQualifier(
                           PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE);
        Uin32 cimPos = instProperty.findQualifier(
                           PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE);
        if (refPos != PEG_NOT_FOUND && cimPos == PEG_NOT_FOUND)
        {
            instProperty.addQualifier(refProperty.getQualifier(pos));
        }
    }
#endif

    // Check the type of the embedded instance against the class specified by
    // the EmbeddedInstance qualifier. We can only do this if the context
    // is non-zero.
    if (context != 0)
    {
        if (referenceProperty.getType() == CIMTYPE_INSTANCE)
        {
            Uint32 pos = referenceProperty.findQualifier(
                             PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE);

            PEGASUS_ASSERT(pos != PEG_NOT_FOUND);

            String qualClassStr;
            referenceProperty.getQualifier(pos).getValue().get(
                qualClassStr);
            CIMName embedInstClassName(qualClassStr);
            Array<CIMName> embeddedInstSubclasses =
                context->enumerateClassNames(nameSpace, embedInstClassName,
                    true);
            embeddedInstSubclasses.append(embedInstClassName);

            Array<CIMInstance> embeddedInstances;
            if (referenceProperty.isArray())
            {
                instProperty.getValue().get(embeddedInstances);
            }
            else
            {
                CIMInstance embeddedInst;
                instProperty.getValue().get(embeddedInst);
                embeddedInstances.append(embeddedInst);
            }

            Array<CIMClass> embeddedClassDefs;
            for (Uint32 i = 0, n = embeddedInstances.size(); i < n; ++i)
            {
                CIMInstance& currentInstance = embeddedInstances[i];
                CIMName currentClassName = currentInstance.getClassName();
                if (Contains(embeddedInstSubclasses, currentClassName))
                {
                    CIMClass embeddedClassDef;
                    bool found = false;
                    for (Uint32 j = 0, m = embeddedClassDefs.size(); j < m;
                        ++j)
                    {
                        CIMClass& tmpClassDef = embeddedClassDefs[j];
                        if (tmpClassDef.getClassName() == currentClassName)
                        {
                            embeddedClassDef = tmpClassDef;
                            found = true;
                        }
                    }

                    if (!found)
                    {
                        embeddedClassDef = context->getClass(nameSpace,
                            currentClassName);
                        embeddedClassDefs.append(embeddedClassDef);
                    }

                    SharedPtr<NormalizerContext> tmpContext(context->clone());
                    ObjectNormalizer tmpNormalizer(embeddedClassDef,
                        includeQualifiers, includeClassOrigin, nameSpace,
                        tmpContext);
                    if (currentInstance.getPath().getKeyBindings().size()==0)
                    {
                        currentInstance.setPath(
                            currentInstance.buildPath(embeddedClassDef));
                    }
                    embeddedInstances[i] =
                        tmpNormalizer.processInstance(currentInstance);
                }
                else
                {
                    MessageLoaderParms message(
                        "Common.ObjectNormalizer."
                            "INVALID_EMBEDDED_INSTANCE_TYPE",
                        "Found embedded instance of type $0: was expecting "
                            "$1 for property $2",
                        currentClassName.getString(),
                        qualClassStr,
                        instProperty.getName().getString());

                    throw CIMException(CIM_ERR_FAILED, message);
                }
            }

            if (referenceProperty.isArray())
            {
              normalizedProperty.setValue(CIMValue(embeddedInstances));
            }
            else
            {
              normalizedProperty.setValue(CIMValue(embeddedInstances[0]));
            }
        }
    }
    return normalizedProperty;
}

ObjectNormalizer::ObjectNormalizer()
    : _includeQualifiers(false),
    _includeClassOrigin(false),
    _context(0)
{
}

ObjectNormalizer::ObjectNormalizer(
    const CIMClass& cimClass,
    Boolean includeQualifiers,
    Boolean includeClassOrigin,
    const CIMNamespaceName& nameSpace,
    SharedPtr<NormalizerContext>& context)
  : _cimClass(cimClass),
    _includeQualifiers(includeQualifiers),
    _includeClassOrigin(includeClassOrigin),
    _context(context),
    _nameSpace(nameSpace)
{
    if (!_cimClass.isUninitialized())
    {
        // ATTN: the following code is intended to expedite normalizing
        // instances and instance object paths by establishing the keys
        // once now rather than multiple times later. it is biased
        // toward providers that return many instances with many properties.

        // build a reference object path within the class
        Array<CIMKeyBinding> keys;

        for (Uint32 i = 0, n = _cimClass.getPropertyCount(); i < n; i++)
        {
            CIMConstProperty referenceProperty = _cimClass.getProperty(i);

            Uint32 pos = referenceProperty.findQualifier("key");

            if ((pos != PEG_NOT_FOUND) &&
                (referenceProperty.getQualifier(pos).getValue().equal(
                     CIMValue(true))))
            {
                if (referenceProperty.getType() == CIMTYPE_REFERENCE)
                {
                    // ATTN: a fake reference is inserted in the key so that
                    // the _BubbleSort() method in CIMObjectPath does not
                    // throw and exception. It implicitly validates keys of
                    // type REFERENCE so just place a dummy value for now.
                    // The value will be replaced by the normalized object
                    // later.
                    keys.append(CIMKeyBinding(referenceProperty.getName(),
                        "class.key=\"value\"", CIMKeyBinding::REFERENCE));
                }
                else
                {
                    keys.append(CIMKeyBinding(referenceProperty.getName(),
                        referenceProperty.getValue()));
                }
            }
        }

        // update class object path
        CIMObjectPath cimObjectPath(_cimClass.getPath());

        cimObjectPath.setKeyBindings(keys);

        _cimClass.setPath(cimObjectPath);
    }
}

CIMObjectPath ObjectNormalizer::processClassObjectPath(
    const CIMObjectPath& cimObjectPath) const
{
    // pre-check
    if (!_enableNormalization || _cimClass.isUninitialized())
    {
        // do nothing
        return cimObjectPath;
    }

    /*
    // ATTN: moving similar logic to the response handlers because this
    // type of error should be checked regardless with or without
    // normalization enabled.
    if (cimObjectPath.getClassName().isNull())
    {
        throw CIMException(CIM_ERR_FAILED, "uninitialized object path");
    }
    */

    /*
    // ATTN: The following code is currently redundant because the CIMName
    // object validates legal names when it is constructed. It is included
    // here for completeness.
    // check class name
    if (!CIMName(cimObjectPath.getClassName()).legal())
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_CLASS_NAME",
            "Invalid class name: $0",
            cimObjectPath.getClassName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }
    */

    // check class type
    if (!_cimClass.getClassName().equal(cimObjectPath.getClassName()))
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_CLASS_TYPE",
            "Invalid class type: $0",
            cimObjectPath.getClassName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }

    CIMObjectPath normalizedObjectPath(
        _cimClass.getPath().getHost(),
        _cimClass.getPath().getNameSpace(),
        _cimClass.getClassName());

    // ignore any keys, they are not part of a class object path

    return normalizedObjectPath;
}

CIMObjectPath ObjectNormalizer::processInstanceObjectPath(
    const CIMObjectPath& cimObjectPath) const
{
    // pre-check
    if (!_enableNormalization || _cimClass.isUninitialized())
    {
        // do nothing
        return cimObjectPath;
    }

    /*
    // ATTN: moving similar logic to the response handlers because this
    // type of error should be checked regardless with or without
    // normalization enabled.
    if (cimObjectPath.getClassName().isNull())
    {
        throw CIMException(CIM_ERR_FAILED, "uninitialized object path");
    }
    */

    /*
    // ATTN: The following code is currently redundant because the CIMName
    // object validates legal names when it is constructed. It is included
    // here for completeness.
    // check class name
    if (!CIMName(cimObjectPath.getClassName()).legal())
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_CLASS_NAME",
            "Invalid class name: $0",
            cimObjectPath.getClassName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }
    */

    // check class type
    if (!_cimClass.getClassName().equal(cimObjectPath.getClassName()))
    {
        MessageLoaderParms message(
            "Common.ObjectNormalizer.INVALID_CLASS_TYPE",
            "Invalid class type: $0",
            cimObjectPath.getClassName().getString());

        throw CIMException(CIM_ERR_FAILED, message);
    }

    CIMObjectPath normalizedObjectPath(
        cimObjectPath.getHost(),
        cimObjectPath.getNameSpace(),
        cimObjectPath.getClassName());

    Array<CIMKeyBinding> normalizedKeys;

    Array<CIMKeyBinding> referenceKeys = _cimClass.getPath().getKeyBindings();
    Array<CIMKeyBinding> cimKeys = cimObjectPath.getKeyBindings();

    for (Uint32 i = 0, n = referenceKeys.size(); i < n; i++)
    {
        CIMKeyBinding key;

        // override the value from the specified object
        for (Uint32 j = 0, m = cimKeys.size(); j < m; j++)
        {
            if (referenceKeys[i].getName().equal(cimKeys[j].getName()))
            {
                // check type
                if (referenceKeys[i].getType() != cimKeys[j].getType())
                {
                    MessageLoaderParms message(
                        "Common.ObjectNormalizer.INVALID_KEY_TYPE",
                        "Invalid key type: $0",
                        referenceKeys[i].getName().getString());

                    throw CIMException(CIM_ERR_FAILED, message);
                }

                key = CIMKeyBinding(referenceKeys[i].getName(),
                    cimKeys[j].getValue(), referenceKeys[i].getType());

                break;
            }
        }

        // key not found
        if (key.getName().isNull())
        {
            MessageLoaderParms message(
                "Common.ObjectNormalizer.MISSING_KEY",
                "Missing key: $0",
                referenceKeys[i].getName().getString());

            throw CIMException(CIM_ERR_FAILED, message);
        }

        normalizedKeys.append(key);
    }

    normalizedObjectPath.setKeyBindings(normalizedKeys);

    return normalizedObjectPath;
}

CIMInstance ObjectNormalizer::processInstance(
    const CIMInstance& cimInstance) const
{
    // pre-checks
    if (!_enableNormalization || _cimClass.isUninitialized())
    {
        // do nothing
        return cimInstance;
    }

    /*
    // ATTN: moving similar logic to the response handlers because this
    // type of error should be checked regardless with or without
    // normalization enabled.
    if (cimInstance.isUninitialized())
    {
        throw CIMException(CIM_ERR_FAILED, "unintialized instance object.");
    }
    */

    CIMInstance normalizedInstance(_cimClass.getClassName());

    // proces object path
    normalizedInstance.setPath(
        processInstanceObjectPath(cimInstance.getPath()));

    // process instance qualifiers
    if (_includeQualifiers)
    {
        // propagate class qualifiers
        for (Uint32 i = 0, n = _cimClass.getQualifierCount(); i < n; i++)
        {
            CIMConstQualifier referenceQualifier = _cimClass.getQualifier(i);

            Uint32 pos =
                cimInstance.findQualifier(referenceQualifier.getName());

            // update value if qualifier is present in the specified property
            if (pos != PEG_NOT_FOUND)
            {
                CIMConstQualifier cimQualifier = cimInstance.getQualifier(pos);

                CIMQualifier normalizedQualifier =
                    _processQualifier(
                        referenceQualifier,
                        cimQualifier);

                normalizedInstance.addQualifier(normalizedQualifier);
            }
            else
            {
                normalizedInstance.addQualifier(referenceQualifier.clone());
            }
        }
    }

    // check property names and types. any properties in the class but not
    // in the instance are implicitly dropped.
    for (Uint32 i = 0, n = cimInstance.getPropertyCount(); i < n; i++)
    {
        CIMConstProperty instProperty = cimInstance.getProperty(i);

        Uint32 pos = _cimClass.findProperty(instProperty.getName());

        if (pos != PEG_NOT_FOUND)
        {
            CIMConstProperty cimProperty = _cimClass.getProperty(pos);

            CIMProperty normalizedProperty =
                processProperty(
                    cimProperty,
                    instProperty,
                    _includeQualifiers,
                    _includeClassOrigin,
                    _context.get(),
                    _nameSpace);

            normalizedInstance.addProperty(normalizedProperty);
        }
    }

    return normalizedInstance;
}

PEGASUS_NAMESPACE_END
