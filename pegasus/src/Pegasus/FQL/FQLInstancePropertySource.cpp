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

#include "FQLInstancePropertySource.h"
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/CIMInstance.h>

PEGASUS_NAMESPACE_BEGIN

#ifdef FQL_DOTRACE
#define ENABLE_LOCAL_DIAGNOSTICS
#endif

Boolean FQLInstancePropertySource::isArrayProperty(
   const CIMName& propertyName) const
{
   unsigned int pos=ci.findProperty(propertyName);
   PEGASUS_ASSERT(pos != PEG_NOT_FOUND);
   CIMConstProperty p = ci.getProperty(pos);
   return p.isArray();
}

/*
    Return the value in a property.
    If the property cannot be found, return false
*/
Boolean _getPropertyValue(const CIMInstance& inst,
    const CIMName name,
    CIMValue& val)
{
#ifdef ENABLE_LOCAL_DIAGNOSTICS
    DCOUT << "Instance from which to retrieve "
        << inst.getClassName().getString() << " propertyName "
        << name.getString() << endl;
#endif
    unsigned int pos = inst.findProperty(name);
    if (pos==PEG_NOT_FOUND)
    {

#ifdef ENABLE_LOCAL_DIAGNOSTICS
      DCOUT << "property " << name.getString() <<  " pos " << pos
          << " NOT found" << endl;
#endif
      return false;
    }

#ifdef ENABLE_LOCAL_DIAGNOSTICS
    DCOUT << "property " << name.getString() << " FOUND" << endl;
#endif
    val=inst.getProperty(pos).getValue();
    return true;
}

/*
    Get the value of the defined property (including looping through
    chained properties) and convert that value to an FQLOperand.
*/
Boolean FQLInstancePropertySource::getValue(
    const String& propertyName,
    FQLOperand& value) const
{
    CIMValue val;
    CIMType type;

#ifdef ENABLE_LOCAL_DIAGNOSTICS
    DCOUT << "getValue " << propertyName << " isChained "
        << boolToString(value.isChained()) << endl;
#endif
    // if dotted property, return the embedded instance or false if
    // the value is NOT an instance.
    if (value.isChained())
    {
        if (!_getPropertyValue(ci, propertyName, val))
        {
            // Property could not be found, return false.
            return false;
        }
        type=val.getType();

        if (type != CIMTYPE_INSTANCE)
        {
            return false;
        }
        else
        {
            CIMInstance ciLocal;
            val.get(ciLocal);
            if (value.isChained())
            {
                PEGASUS_ASSERT(value.chainSize() != 0);

                // If this property is chained, resolve the property chain
                FQLOperand x;
                Uint32 chainSize = value.chainSize();
                Uint32 lastEntry = chainSize - 1;

                for (Uint32 i = 0; i < chainSize; i++)
                {
                    // Get chained operand and get name from it
                    x = value.chainItem(i);
                    String pName  = x.getPropertyName();

                    // Get name from the chain item
                    if (!_getPropertyValue(ciLocal, pName, val))
                    {
                        // Property could not be found, return false.
                        return false;
                    }
                    type=val.getType();

                    if ((type == CIMTYPE_INSTANCE))
                    {
                        if (i == lastEntry)
                        {
                            return false;
                        }
                        else
                        {
                            val.get(ciLocal);
                        }
                    }
                    else
                    {
                        if (i != lastEntry)
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }
    else
    {
       unsigned int pos=ci.findProperty(propertyName);
       if (pos==PEG_NOT_FOUND)
       {
          // Property could not be found, return false.
          return false;
       }

        val=ci.getProperty(pos).getValue();
        type=val.getType();
    }

    if (val.isNull())
    {
      value=FQLOperand();
      return true;
    }

    if (val.isArray())
    {
      switch (type)
      {
         case CIMTYPE_UINT8:
            {
               Array<Uint8> propertyValueUint8;
               val.get(propertyValueUint8);
               Array<Sint64> propertyValueSint64;
               for (Uint32 i = 0; i < propertyValueUint8.size(); i++)
               {
                  propertyValueSint64.append((Sint64)propertyValueUint8[i]);
               }
               value = FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_UINT16:
            {
               Array<Uint16> propertyValueUint16;
               val.get(propertyValueUint16);
               Array<Sint64> propertyValueSint64;
               for (Uint32 i = 0; i < propertyValueUint16.size(); i++)
               {
                  propertyValueSint64.append((Sint64)propertyValueUint16[i]);
               }
               value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_UINT32:
            {
               Array<Uint32> propertyValueUint32;
               val.get(propertyValueUint32);
               Array<Sint64> propertyValueSint64;
               for (Uint32 i = 0; i < propertyValueUint32.size(); i++)
               {
                  propertyValueSint64.append((Sint64)propertyValueUint32[i]);
               }
               value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_UINT64:
            {
               Array<Uint64> propertyValueUint64;
               val.get(propertyValueUint64);
               Array<Sint64> propertyValueSint64;
               for (Uint32 i = 0; i < propertyValueUint64.size(); i++)
               {
                  propertyValueSint64.append((Sint64)propertyValueUint64[i]);
               }
               value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_SINT8:
            {
               Array<Sint8> propertyValueSint8;
               val.get(propertyValueSint8);
               Array<Sint64> propertyValueSint64;
               for (Uint32 i = 0; i < propertyValueSint8.size(); i++)
               {
                  propertyValueSint64.append((Sint64)propertyValueSint8[i]);
               }
               value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_SINT16:
            {
               Array<Sint16> propertyValueSint16;
               val.get(propertyValueSint16);
               Array<Sint64> propertyValueSint64;
               for (Uint32 i = 0; i < propertyValueSint16.size(); i++)
               {
                  propertyValueSint64.append((Sint64)propertyValueSint16[i]);
               }
               value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_SINT32:
            {
               Array<Sint32> propertyValueSint32;
               val.get(propertyValueSint32);
               Array<Sint64> propertyValueSint64;
               for (Uint32 i = 0; i < propertyValueSint32.size(); i++)
               {
                  propertyValueSint64.append(propertyValueSint32[i]);
               }
               value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_SINT64:
            {
               Array<Sint64> propertyValueSint64;
               val.get(propertyValueSint64);
               value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
            }
            break;

         case CIMTYPE_REAL32:
            {
               Array<Real32> propertyValueReal32;
               val.get(propertyValueReal32);
               Array<Real64> propertyValueReal64;
               for (Uint32 i = 0; i < propertyValueReal32.size(); i++)
               {
                  propertyValueReal64.append((Real64)propertyValueReal32[i]);
               }
               value=FQLOperand(propertyValueReal64, FQL_DOUBLE_VALUE_TAG);
            }
            break;

         case CIMTYPE_REAL64:
            {
               Array<Real64> propertyValueReal64;
               val.get(propertyValueReal64);
               value=FQLOperand(propertyValueReal64, FQL_DOUBLE_VALUE_TAG);
            }
            break;

         case CIMTYPE_BOOLEAN :
            {
               Array<Boolean> booleanValues;
               val.get(booleanValues);
               value=FQLOperand(booleanValues, FQL_BOOLEAN_VALUE_TAG);
            }
            break;

         case CIMTYPE_CHAR16:
            {
               Array <Char16> char16val;
               val.get(char16val);
               String str;
               for (Uint32 i = 0 ; i < char16val.size(); i++)
               {
                  str.append(char16val[i]);
               }
               value=FQLOperand(str, FQL_STRING_VALUE_TAG);
               break;
            }
         case CIMTYPE_DATETIME :
            {
               Array<CIMDateTime> datetimeValue;
               val.get(datetimeValue);
               value = FQLOperand(datetimeValue, FQL_DATETIME_VALUE_TAG);
               break;
            }
         case CIMTYPE_STRING :
            {
               Array<String> strValue;
               val.get(strValue);
               value=FQLOperand(strValue,FQL_STRING_VALUE_TAG);
               break;
            }
         case CIMTYPE_REFERENCE :
            {
               Array<CIMObjectPath> objPathValue;
               val.get(objPathValue);
               value=FQLOperand(objPathValue,FQL_REFERENCE_VALUE_TAG);
               break;
            }

         case CIMTYPE_OBJECT :
         case CIMTYPE_INSTANCE :
              PEGASUS_ASSERT(false);
      }
   }
   else
   {
      switch (type)
      {
       case CIMTYPE_UINT8:
          Uint8 propertyValueUint8;
          val.get(propertyValueUint8);
          value=FQLOperand(propertyValueUint8,FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_UINT16:
          Uint16 propertyValueUint16;
          val.get(propertyValueUint16);
          value=FQLOperand(propertyValueUint16, FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_UINT32:
          Uint32 propertyValueUint32;
          val.get(propertyValueUint32);
          value=FQLOperand(propertyValueUint32, FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_UINT64:
          Uint64 propertyValueUint64;
          val.get(propertyValueUint64);
          value=FQLOperand(propertyValueUint64, FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_SINT8:
          Sint8 propertyValueSint8;
          val.get(propertyValueSint8);
          value=FQLOperand(propertyValueSint8, FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_SINT16:
          Sint16 propertyValueSint16;
          val.get(propertyValueSint16);
          value=FQLOperand(propertyValueSint16, FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_SINT32:
          Sint32 propertyValueSint32;
          val.get(propertyValueSint32);
          value=FQLOperand(propertyValueSint32, FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_SINT64:
          Sint64 propertyValueSint64;
          val.get(propertyValueSint64);
          value=FQLOperand(propertyValueSint64, FQL_INTEGER_VALUE_TAG);
          break;

       case CIMTYPE_REAL32:
          Real32 propertyValueReal32;
          val.get(propertyValueReal32);
          value=FQLOperand(propertyValueReal32, FQL_DOUBLE_VALUE_TAG);
          break;

       case CIMTYPE_REAL64:
          Real64 propertyValueReal64;
          val.get(propertyValueReal64);
          value=FQLOperand(propertyValueReal64, FQL_DOUBLE_VALUE_TAG);
          break;

       case CIMTYPE_BOOLEAN :
          Boolean booleanValue;
          val.get(booleanValue);
          value=FQLOperand(booleanValue, FQL_BOOLEAN_VALUE_TAG);
          break;

       case CIMTYPE_CHAR16:
       {
          Char16 char16Value;
          val.get(char16Value);
          String str;
          str.append(char16Value);
          value=FQLOperand(str, FQL_STRING_VALUE_TAG);
          break;
       }
       case CIMTYPE_DATETIME :
       {
          CIMDateTime datetimeValue;
          val.get(datetimeValue);
          value=FQLOperand(datetimeValue, FQL_DATETIME_VALUE_TAG);
          break;
       }
       case CIMTYPE_STRING :
       {
          String strValue;
          val.get(strValue);
          value=FQLOperand(strValue,FQL_STRING_VALUE_TAG);
          break;
       }
      case CIMTYPE_REFERENCE :
      {
          CIMObjectPath objPathValue;
          val.get(objPathValue);
          value = FQLOperand(objPathValue, FQL_REFERENCE_VALUE_TAG);
          break;
      }
      // The following are not valid FQL types
        case CIMTYPE_OBJECT :
        case CIMTYPE_INSTANCE :
          PEGASUS_ASSERT(false);
      }
   }
   value.setCIMType(type);
   return true;
 }

PEGASUS_NAMESPACE_END

