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

#include "Closure.h"
#include <Pegasus/Common/Pair.h>
#include <Pegasus/Common/Constants.h>

PEGASUS_NAMESPACE_BEGIN

// ATTN-MEB: handle ModelCorrespondence() qualifier.

static Uint32 _findClass(const Array<CIMClass>& classes, const CIMName& cn)
{
    for (Uint32 i = 0; i < classes.size(); i++)
    {
        if (classes[i].getClassName() == cn)
            return i;
    }

    // Not found!
    return PEG_NOT_FOUND;
}

static Uint32 _findClass(const Array<CIMName>& classNames, const CIMName& cn)
{
    for (Uint32 i = 0; i < classNames.size(); i++)
    {
        if (classNames[i] == cn)
            return i;
    }

    // Not found!
    return PEG_NOT_FOUND;
}

static bool _isAssociation(const CIMClass& cc)
{
    Uint32 pos = cc.findQualifier("Association");

    if (pos == PEG_NOT_FOUND)
        return false;

    CIMConstQualifier cq = cc.getQualifier(pos);

    if (cq.getType() != CIMTYPE_BOOLEAN || cq.isArray())
    {
        return false;
    }

    return true;
}

template<class CONTAINER>
static int _getEmbeddedClassName(const CONTAINER& c, String& ecn)
{
    ecn.clear();
    Uint32 pos = c.findQualifier(PEGASUS_QUALIFIERNAME_EMBEDDEDINSTANCE);

    if (pos == PEG_NOT_FOUND)
    {
        return 0;
    }

    CIMConstQualifier cq = c.getQualifier(pos);

    if (cq.getType() != CIMTYPE_STRING || cq.isArray())
    {
        return -1;
    }

    cq.getValue().get(ecn);
    return 0;
}

static bool _isA(
    const Array<CIMClass>& classes,
    const CIMName& superClassName,
    const CIMName& className)
{
    // If same class, return true now:

    if (superClassName == className)
    {
        return true;
    }

    // Find the class:

    Uint32 pos = _findClass(classes, className);

    if (pos == PEG_NOT_FOUND)
    {
        return false;
    }

    const CIMClass& cc = classes[pos];

    // Get superclass:

    const CIMName& scn = cc.getSuperClassName();

    if (scn.isNull())
    {
        return false;
    }

    return _isA(classes, superClassName, scn);
}

int Closure(
    const CIMName& className,
    const Array<CIMClass>& classes,
    Array<CIMName>& closure)
{
    // Avoid if class already in closure:

    if (_findClass(closure, className) != PEG_NOT_FOUND)
    {
        return 0;
    }

    // Add class to closure:

    closure.append(className);

    // Find the class:

    Uint32 pos = _findClass(classes, className);

    if (pos == PEG_NOT_FOUND)
    {
        return -1;
    }

    const CIMClass& cc = classes[pos];

    // Add superclass to closure:

    const CIMName& scn = cc.getSuperClassName();

    if (!scn.isNull())
    {
        if (Closure(scn, classes, closure) != 0)
            return -1;
    }

    // References and EmbeddedInstances.

    for (Uint32 i = 0; i < cc.getPropertyCount(); i++)
    {
        const CIMConstProperty& cp = cc.getProperty(i);

        if (cp.getType() == CIMTYPE_REFERENCE)
        {
            const CIMName& rcn = cp.getReferenceClassName();

            if (Closure(rcn, classes, closure) != 0)
            {
                return -1;
            }
        }
        else if (cp.getType() == CIMTYPE_STRING)
        {
            String ecn;

            if (_getEmbeddedClassName(cp, ecn) != 0)
            {
                return -1;
            }

            if (ecn.size() && Closure(ecn, classes, closure) != 0)
            {
                return -1;
            }
        }
    }

    // Methods and EmbeddedInstances:

    for (Uint32 i = 0; i < cc.getMethodCount(); i++)
    {
        const CIMConstMethod& cm = cc.getMethod(i);

        if (cm.getType() == CIMTYPE_STRING)
        {
            String ecn;

            if (_getEmbeddedClassName(cm, ecn) != 0)
            {
                return -1;
            }

            if (ecn.size() && Closure(ecn, classes, closure) != 0)
            {
                return -1;
            }
        }

        // Parameters and EmbeddedInstances:

        for (Uint32 j = 0; j < cm.getParameterCount(); j++)
        {
            const CIMConstParameter& cp = cm.getParameter(j);

            if (cp.getType() == CIMTYPE_REFERENCE)
            {
                const CIMName& rcn = cp.getReferenceClassName();

                if (Closure(rcn, classes, closure) != 0)
                {
                    return -1;
                }
            }
            else if (cp.getType() == CIMTYPE_STRING)
            {
                String ecn;

                if (_getEmbeddedClassName(cp, ecn) != 0)
                {
                    return -1;
                }

                if (ecn.size() && Closure(ecn, classes, closure) != 0)
                {
                    return -1;
                }
            }
        }
    }

    // Experimental only!
#if 0

    // Include closure of all assosicate classes that refer to the source
    // class.

    for (Uint32 i = 0; i < classes.size(); i++)
    {
        const CIMClass& tcc = classes[i];

        if (!_isAssociation(tcc))
        {
            continue;
        }
        const CIMName& tcn = tcc.getClassName();

        if (_findClass(closure, tcn) != PEG_NOT_FOUND)
        {
            continue;
        }
        for (Uint32 j = 0; j < tcc.getPropertyCount(); j++)
        {
            const CIMConstProperty& cp = tcc.getProperty(j);

            if (cp.getType() == CIMTYPE_REFERENCE)
            {
                const CIMName& rcn = cp.getReferenceClassName();

                if (_isA(classes, rcn, cc.getClassName()))
                {
                    if (Closure(tcc.getClassName(), classes, closure) != 0)
                        return -1;
                }
            }
        }
    }

#endif

    return 0;
}

PEGASUS_NAMESPACE_END
