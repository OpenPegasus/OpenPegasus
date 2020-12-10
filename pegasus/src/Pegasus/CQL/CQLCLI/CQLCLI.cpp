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

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/FileSystem.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <Pegasus/Common/String.h>
#include <Pegasus/CQL/CQLParser.h>
#include <Pegasus/CQL/CQLParserState.h>
#include <Pegasus/CQL/CQLSelectStatement.h>
#include <Pegasus/Repository/RepositoryQueryContext.h>
#include <Pegasus/Common/CIMName.h>
#include <Pegasus/Common/MessageLoader.h>
#include <Pegasus/Repository/CIMRepository.h>
#include <Pegasus/Common/CIMInstance.h>
#include <Pegasus/Common/CIMObjectPath.h>

#define PEGASUS_SINT64_MIN (PEGASUS_SINT64_LITERAL(0x8000000000000000))
#define PEGASUS_UINT64_MAX PEGASUS_UINT64_LITERAL(0xFFFFFFFFFFFFFFFF)
#define PEGASUS_SINT64_MAX (PEGASUS_SINT64_LITERAL(0x7FFFFFFFFFFFFFFF))

PEGASUS_USING_PEGASUS;
PEGASUS_USING_STD;
int CQL_parse();

Boolean cqlcli_verbose = false;

String getStatementString(const String& stmt)
{
    // Returns the select statement string, but takes
    // non-ascii chars (> 0x7f) into account by turning
    // them into hex strings.
    // This is needed because some tests contain
    // non-ascii in their select statements, and we
    // want a consistent output on all platforms.
    String res;

    for (Uint32 i = 0, n = stmt.size(); i < n; i++)
    {
        Uint16 code = stmt[i];

        if (isascii(code))
        {
            res.append((char)code);
        }
        else
        {
            // turn into hex format:
            char hex[8];
            sprintf(hex, "\\x%04X", code);
            res.append(hex);
        }
    }

    return res;
}


void printProperty(
    const CIMProperty& prop,
    Uint32 propNum,
    const String& prefix)
{
    // Recursive function to handle embedded object trees

    cout << prefix << "Prop #" << propNum << " Name = "
         << prop.getName().getString();

    CIMValue val = prop.getValue();

    CIMType valType = val.getType();
    if (valType != CIMTYPE_OBJECT && valType != CIMTYPE_INSTANCE)
    {
        // Not embedded object
        if (val.isNull())
        {
            cout << ", Value = NULL" << endl;
        }
        else
        {
            cout << ", Value = " << val.toString() << endl;
        }
    }
    else
    {
        // Embedded object, or array of objects
        Array<CIMObject> embObjs;
        if (val.isArray())
        {
            if (valType == CIMTYPE_INSTANCE)
            {
                Array<CIMInstance> embInsts;
                val.get(embInsts);
                int instCount = embInsts.size();
                for (int i = 0; i < instCount; i++)
                {
                    embObjs.append((CIMObject)embInsts[i]);
                }
            }
            else
            {
                val.get(embObjs);
            }
    }
    else
    {
        if (valType == CIMTYPE_INSTANCE)
        {
            CIMInstance tmpInst;
            val.get(tmpInst);
            embObjs.append((CIMObject)tmpInst);
        }
        else
        {
            CIMObject tmpObj;
            val.get(tmpObj);
            embObjs.append(tmpObj);
        }
    }

    for (Uint32 j = 0; j < embObjs.size(); j++)
    {
        CIMObject embObj = embObjs[j];
        if (embObj.isClass())
        {
            // Embedded class
            CIMClass embCls(embObj);
            cout << ", Value = class of "
                 << embCls.getClassName().getString() << endl;
        }
        else
        {
            // Embedded instance, need to recurse on each property
            CIMInstance embInst(embObj);

            String newPrefix = prefix;
            newPrefix.append(prefix);

            cout << endl << newPrefix << "Instance of class "
                 << embInst.getClassName().getString() << endl;

            Uint32 cnt = embInst.getPropertyCount();
            if (cnt == 0)
            {
                cout << newPrefix << "No properties left after projection"
                     << endl;
            }

            if (cnt > 10 && !cqlcli_verbose)
            {
                cout << newPrefix << "Instance has " << cnt << " properties"
                     << endl;
            }
            else
            {
                for (Uint32 n = 0; n < cnt; n++)
                {
                    printProperty(embInst.getProperty(n), n, newPrefix);
                }
            }
        }
    }
  }
}

Boolean _applyProjection(Array<CQLSelectStatement>& _statements,
                         Array<CIMInstance>& _instances,
                         String testOption)
{
  if (testOption == String::EMPTY || testOption == "2")
  {
    cout << "========Apply Projection Results========" << endl;

    for (Uint32 i = 0; i < _statements.size(); i++)
    {
      cout << "======================================" << i << endl;
      cout << _statements[i].toString() << endl;

      for (Uint32 j = 0; j < _instances.size(); j++)
      {
        cout << "Instance of class "
             << _instances[j].getClassName().getString() << endl;

        try
        {
          CIMInstance projInst = _instances[j].clone();

          // Remove the property "MissingProperty" for the
          // testcases that depend on the property being missing.
          Uint32 missing = projInst.findProperty("MissingProperty");
          if (missing != PEG_NOT_FOUND)
          {
            projInst.removeProperty(missing);
          }

          CIMInstance cloneInst = projInst.clone();
          Boolean gotPropExc = false;
          try
          {
            _statements[i].applyProjection(projInst, false);
          }
          catch (QueryRuntimePropertyException & qrpe)
          {
            // Got a missing property exception.
            cout << "-----" << qrpe.getMessage() << endl;
            gotPropExc = true;
          }

          if (gotPropExc)
          {
            // Got a missing property exception.
            // Try again, allowing missing properties.
            // Need to use a cloned instance because the original instance
            // was partially projected.
            cout << "Instance of class "
                 << _instances[j].getClassName().getString()
                 << ".  Allow missing properties." << endl;
            projInst = cloneInst;
            _statements[i].applyProjection(projInst, true);
          }

          Uint32 cnt = projInst.getPropertyCount();
          if (cnt == 0)
          {
            cout << "-----No properties left after projection" << endl;
          }

          String prefix("-----");

          if (cnt > 10 && !cqlcli_verbose)
          {
            cout << "-----Instance has " << cnt << " properties" << endl;
          }
          else
          {
            for (Uint32 n = 0; n < cnt; n++)
            {
              CIMProperty prop = projInst.getProperty(n);
              printProperty(prop, n, prefix);
            }
          }
        }
        catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
        catch(...){ cout << "Unknown Exception" << endl;}
      }
    }
  }

  return true;
}

Boolean _validateProperties(Array<CQLSelectStatement>& _statements,
                            Array<CIMInstance>& _instances,
                            String testOption)
{
    if (testOption == String::EMPTY || testOption == "4")
    {
        cout << "======Validate Properties Results=======" << endl;

        for (Uint32 i = 0; i < _statements.size(); i++)
        {
            cout << "======================================" << i << endl;
            cout << _statements[i].toString() << endl;

            try
            {
                _statements[i].validate();
                cout << "----- validate ok" << endl;
            }
            catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
            catch(...){ cout << "Unknown Exception" << endl;}
        }
    }

    return true;
}

void _printPropertyList(CIMPropertyList& propList)
{
    if (propList.isNull())
    {
        cout << "-----all properties required" << endl;
    }
    else if (propList.size() == 0)
    {
        cout << "-----no properties required" << endl;
    }
    else
    {
        for (Uint32 n = 0; n < propList.size(); n++)
        {
            cout << "-----Required property " << propList[n].getString()
                 << endl;
        }
    }
}

Boolean _getPropertyList(Array<CQLSelectStatement>& _statements,
                         Array<CIMInstance>& _instances,
                         CIMNamespaceName ns,
                         String testOption)
{
  if (testOption == String::EMPTY || testOption == "3")
  {
    cout << "========Get Property List Results=======" << endl;

    CIMPropertyList propList;

    for (Uint32 i = 0; i < _statements.size(); i++)
    {
      cout << "======================================" << i << endl;
      cout << _statements[i].toString() << endl;

      try
      {
        cout << endl << "Get Class Path List" << endl;
        Array<CIMObjectPath> fromPaths = _statements[i].getClassPathList();
        for (Uint32 k = 0; k < fromPaths.size(); k++)
        {
          cout << "-----" << fromPaths[k].toString() << endl;
        }
      }
      catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
      catch(...){ cout << "Unknown Exception" << endl;}

      try
      {
        cout << "SELECT Chained Identifiers" << endl;
        Array<CQLChainedIdentifier> selIds =
            _statements[i].getSelectChainedIdentifiers();
        for (Uint32 k = 0; k < selIds.size(); k++)
        {
          cout << "-----" << selIds[k].toString() << endl;
        }
      }
      catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
      catch(...){ cout << "Unknown Exception" << endl;}

      try
      {
        cout << "WHERE Chained Identifiers" << endl;
        Array<CQLChainedIdentifier> whereIds =
            _statements[i].getWhereChainedIdentifiers();
        if (whereIds.size() == 0)
        {
          cout << "-----none" << endl;
        }
        for (Uint32 k = 0; k < whereIds.size(); k++)
        {
          cout << "-----" << whereIds[k].toString() << endl;
        }
      }
      catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
      catch(...){ cout << "Unknown Exception" << endl;}

      try
      {
        cout << "Property List for the FROM class " << endl;
        propList.clear();
        propList = _statements[i].getPropertyList();
        _printPropertyList(propList);
      }
      catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
      catch(...){ cout << "Unknown Exception" << endl;}

      try
      {
        cout << "SELECT Property List for the FROM class " << endl;
        propList.clear();
        propList = _statements[i].getSelectPropertyList();
        _printPropertyList(propList);
      }
      catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
      catch(...){ cout << "Unknown Exception" << endl;}

      try
      {
        cout << "WHERE Property List for the FROM class " << endl;
        propList.clear();
        propList = _statements[i].getWherePropertyList();
        _printPropertyList(propList);
      }
      catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
      catch(...){ cout << "Unknown Exception" << endl;}

      // Build a list of unique class names from the instances
      Array<CIMName> classNames;
      for (Uint32 j = 0; j < _instances.size(); j++)
      {
        Boolean found = false;
        for (Uint32 k = 0; k < classNames.size(); k++)
        {
          if (_instances[j].getClassName() == classNames[k])
          {
            found = true;
          }
        }

        if (!found)
        {
          classNames.append(_instances[j].getClassName());
        }
      }

      for (Uint32 j = 0; j < classNames.size(); j++)
      {
        CIMName className = classNames[j];
        CIMObjectPath classPath (String::EMPTY,
                                 ns,
                                 className);

        try
        {
          cout << "Property List for " << className.getString() << endl;
          propList.clear();
          propList = _statements[i].getPropertyList(classPath);
          _printPropertyList(propList);
        }
        catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
        catch(...){ cout << "Unknown Exception" << endl;}

        try
        {
          cout << "SELECT Property List for " << className.getString() << endl;
          propList.clear();
          propList = _statements[i].getSelectPropertyList(classPath);
          _printPropertyList(propList);
        }
        catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
        catch(...){ cout << "Unknown Exception" << endl;}

        try
        {
          cout << "WHERE Property List for " << className.getString() << endl;
          propList.clear();
          propList = _statements[i].getWherePropertyList(classPath);
          _printPropertyList(propList);
        }
        catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
        catch(...){ cout << "Unknown Exception" << endl;}
      }
    }
  }

  return true;
}

Boolean _evaluate(Array<CQLSelectStatement>& _statements,
                  Array<CIMInstance>& _instances,
                  String testOption)
{
    if (testOption == String::EMPTY || testOption == "1")
    {
        cout << "=========Evaluate Query==============" << endl;

        for (Uint32 i = 0; i < _statements.size(); i++)
        {
            cout << "=========     " << i << "     =========" << endl;
            cout << "-----Query: "
                 << getStatementString(_statements[i].toString())
                 << endl << endl;;

            for (Uint32 j = 0; j < _instances.size(); j++)
            {
                try
                {
                  cout << "-----Instance: "
                       << _instances[j].getPath().toString()
                       << endl;
                  Boolean result = _statements[i].evaluate(_instances[j]);

                  if (cqlcli_verbose)
                  {
                    cout << "Inst # " << j << ": "
                         <<  _statements[i].toString()
                         << " = ";
                  }
                  if (result)
                       cout << "TRUE" << endl;
                  else
                       cout << "FALSE" << endl;
                }
                catch(Exception e)
                {
                  if (cqlcli_verbose)
                    cout << "ERROR! -- " << _statements[i].toString() << endl
                         << e.getMessage() << endl << endl;
                  else
                    cout << "ERROR!" << endl << e.getMessage() << endl << endl;
                }
                catch(...)
                {
                    cout << "Unknown Exception" << endl;
                }
            }
        }
    }

    return true;
}

Boolean _normalize(Array<CQLSelectStatement>& _statements,
                   Array<CIMInstance>& _instances,
                   String testOption)
{
    if (testOption == String::EMPTY || testOption == "5")
    {
        cout << "=========Normalize Results==============" << endl;

        for (Uint32 i = 0; i < _statements.size(); i++)
        {
            cout << "======================================" << i << endl;

            try
            {
                cout << "-----Statement before normalize" << endl;
                _statements[i].applyContext();
                cout << _statements[i].toString() << endl;

                _statements[i].normalizeToDOC();
                cout << "-----Statement after normalize" << endl;
                cout << _statements[i].toString() << endl;

                cout << "-----Traversing the predicates" << endl;
                CQLPredicate topPred = _statements[i].getPredicate();
                if (topPred.isSimple())
                {
                    cout << "-----Top predicate is simple: "
                         << topPred.toString() << endl;
                }
                else
                {
                    cout << "-----Top predicate is not simple: "
                         << topPred.toString() << endl;
                    if ( topPred.getInverted() )
                        cout << "-----ERROR - Top predicate is inverted: "
                             << topPred.toString() << endl;

                    Array<CQLPredicate> secondLevelPreds =
                        topPred.getPredicates();
                    for (Uint32 n = 0; n < secondLevelPreds.size(); n++)
                    {
                        if (secondLevelPreds[n].isSimple())
                        {
                            cout << "-----2nd level predicate is simple: "
                               << secondLevelPreds[n].toString() << endl;
                        }
                        else
                        {
                            if ( secondLevelPreds[n].getInverted() )
                                cout << "-----ERROR - 2nd level predicate"
                                        " is inverted: "
                                     << secondLevelPreds[n].toString() << endl;

                            cout << "-----ERROR - 2nd level predicate is"
                                    " NOT simple!: "
                                << secondLevelPreds[n].toString() << endl;
                        }
                    }
                }
            }
            catch(Exception& e){ cout << "-----" << e.getMessage() << endl;}
            catch(...){ cout << "Unknown Exception" << endl;}
        }
    }

    return true;
}

void buildEmbeddedObjects(CIMNamespaceName& ns,
                          Array<CIMInstance>& instances,
                          CIMRepository* rep)
{
    //
    // Sort out the instances of CQL_TestElement, CQL_TestPropertyTypes,
    // CIM_ComputerSystem that will be added as embedded objects to
    // the embedded object test class
    //

    CIMName nameTE("CQL_TestElement");
    CIMName instIdName("InstanceId");
    CIMName nameTPT("CQL_TestPropertyTypes");
    CIMName nameCS("CIM_ComputerSystem");

    CIMInstance testElem;
    Array<CIMObject> testElemArray;
    Boolean foundTestElem = false;

    CIMInstance testCS;
    Boolean foundCS = false;

    Array<CIMObject> testPropTypesArray;

    for (Uint32 i = 0; i < instances.size(); i++)
    {
        // Find the CQL_TestElement with InstanceId = 0
        if (instances[i].getClassName() == nameTE)
        {
            Uint32 index = instances[i].findProperty(instIdName);
            if (index == PEG_NOT_FOUND)
            {
                throw Exception("Error building embedded objects."
                                "  CQL_TestElement with InstanceId prop"
                                " not found");
            }

            Uint64 instId;
            instances[i].getProperty(index).getValue().get(instId);
            if (instId == 0)
            {
                // Found the CQL_TestElement with InstanceId = 0.
                // Change to InstanceId to 10, and save it.
                foundTestElem = true;
                instances[i].removeProperty(index);
                instances[i].addProperty(CIMProperty(instIdName,
                    CIMValue((Uint64)10)));
                testElem = instances[i].clone();
            }
        }

        // Save the CQL_TestPropertyType instances as we find them
        if (instances[i].getClassName() == nameTPT)
        {
            testPropTypesArray.append(instances[i].clone());
        }

        // Save the CIM_ComputerSystem instance
        if (instances[i].getClassName() == nameCS)
        {
            foundCS = true;
            testCS = instances[i].clone();

            // Remove the PrimaryOwnerName property so that apply projection
            // using the wildcard will fail
            Uint32 tmpIdx = testCS.findProperty("PrimaryOwnerName");
            if (tmpIdx != PEG_NOT_FOUND)
            {
                testCS.removeProperty(tmpIdx);
            }
        }
    }

    if (!foundTestElem)
    {
        throw Exception("Error building embedded objects."
                        "  CQL_TestElement with InstanceId = 0 not found");
    }

    if (!foundCS)
    {
        throw Exception("Error building embedded objects."
                        "  CIM_ComputerSystem not found");
    }

    if (testPropTypesArray.size() <= 1)
    {
        throw Exception("Error building embedded objects."
                        " Not enough CQL_TestPropertyTypes found");
    }

    //
    // Now build the array of CQL_TestElement (see below for
    // why we are doing this)
    //

    testElemArray.append(testElem.clone());

    Uint32 index = testElem.findProperty(instIdName);
    testElem.removeProperty(index);
    testElem.addProperty(CIMProperty(instIdName, CIMValue((Uint64)11)));
    testElemArray.append(testElem.clone());

    index = testElem.findProperty(instIdName);
    testElem.removeProperty(index);
    testElem.addProperty(CIMProperty(instIdName, CIMValue((Uint64)12)));
    testElemArray.append(testElem.clone());

    //
    // Get the class that will be added as an embedded object
    //

    CIMClass someClass = rep->getClass(ns,
                                     "CIM_Process",
                                     false,  // local only
                                     true,   // include quals
                                     true);   // include class origin

    // Build the embedded object structure.
    //
    // The structure looks like this:
    //
    // class CQL_EmbeddedSubClass is subclass of CQL_EmbeddedBase
    // class CQL_EmbeddedTestPropertyTypes is subclass of
    //      CQL_EmbeddedTestElement
    //
    // instance of CQL_EmbeddedSubClass
    //
    //   property InstanceID has value 100
    //
    //   property EmbObjBase has
    //      instance of CQL_EmbeddedTestElement
    //         property InstanceID has value 1000
    //         property TEArray has array of instance of CQL_TestElement
    //             the array elements have InstanceID properties
    //             set to 10, 11, 12
    //         property TE has instance of CQL_TestElement
    //             property InstanceID has value 10
    //         property CS has instance of CIM_ComputerSystem
    //             note that the PrimaryOwnerName property is removed
    //         property SomeClass has class of CIM_Process
    //         property SomeString has a string
    //
    //   property EmbObjSub has
    //      instance of CQL_EmbeddedTestPropertyTypes
    //         property InstanceID has value 1001
    //         property TEArray has array of instance of CQL_TestElement
    //             the array elements have InstanceID properties
    //             set to 10, 11, 12
    //         property TE has instance of CQL_TestElement
    //             property InstanceID has value 10
    //         property CS has instance of CIM_ComputerSystem
    //             note that the PrimaryOwnerName property is removed
    //         property TPTArray has array of instance of CQL_TestPropertyTypes
    //             the array is built from the instances compiled in the
    //             repository
    //         property TPT has instance of CQL_TestPropertyTypes
    //             this instance is the first instance found in the repository
    //         property SomeClass has class of CIM_Process
    //         property SomeString has a string
    //         property SomeUint8 has a uint8
    //
    CIMInstance embTE("CQL_EmbeddedTestElement");
    embTE.addProperty(CIMProperty("InstanceID", CIMValue((Uint64)1000)));
    embTE.addProperty(CIMProperty("TEArray", CIMValue(testElemArray)));
    CIMObject _obj1 = testElemArray[0];
    CIMValue testElemVal(_obj1);
    embTE.addProperty(CIMProperty("TE", testElemVal));
    CIMValue csVal(testCS);
    embTE.addProperty(CIMProperty("CS", csVal));
    embTE.addProperty(CIMProperty("SomeClass", CIMValue(someClass)));
    embTE.addProperty(CIMProperty("SomeString", CIMValue(String("Huh?"))));

    CIMInstance embTPT("CQL_EmbeddedTestPropertyTypes");
    embTPT.addProperty(CIMProperty("InstanceID", CIMValue((Uint64)1001)));
    embTPT.addProperty(CIMProperty("TEArray", CIMValue(testElemArray)));
    embTPT.addProperty(CIMProperty("TE", testElemVal));
    embTPT.addProperty(CIMProperty("CS", csVal));
    embTPT.addProperty(CIMProperty("TPTArray", CIMValue(testPropTypesArray)));
    embTPT.addProperty(CIMProperty("TPT", CIMValue(testPropTypesArray[0])));
    embTPT.addProperty(CIMProperty("SomeClass", CIMValue(someClass)));
    embTPT.addProperty(CIMProperty("SomeString", CIMValue(String("What?"))));
    embTPT.addProperty(CIMProperty("SomeUint8", CIMValue((Uint8)3)));

    CIMInstance embSub("CQL_EmbeddedSubClass");
    embSub.addProperty(CIMProperty("InstanceID", CIMValue((Uint64)100)));
    embSub.addProperty(CIMProperty("EmbObjBase", CIMValue(embTE)));
    embSub.addProperty(CIMProperty("EmbObjSub", CIMValue(embTPT)));

    instances.clear();
    instances.append(embSub);
}

void sortInstances(Array<CIMInstance>& x)
{
    Uint32 n = x.size();

    if (n < 2)
        return;

    for (Uint32 i = 0; i < n - 1; i++)
    {
        for (Uint32 j = 0; j < n - 1; j++)
        {
            // This is not quite a lexigraphical sort because
            // CQL_TestPropertyTypesMissing will sort before
            // CQL_TestPropertyTypes.  It was done this way
            // so that the .resgood files don't need to change.

            String str1 = x[j].getClassName().getString();
            String str2 = x[j+1].getClassName().getString();

            Uint32 sz1 = str1.size();
            Uint32 sz2 = str2.size();

            if (sz1 > sz2)
            {
                str1.remove(sz2);
            }
            else if (sz2 > sz1)
            {
                str2.remove(sz1);
            }

            int comp = String::compareNoCase(str1, str2);
            if ((comp == 0 && sz2 > sz1) || comp > 0)
            {
                CIMInstance t = x[j];
                x[j] = x[j+1];
                x[j+1] = t;
            }
        }
    }
}

Boolean populateInstances(Array<CIMInstance>& _instances,
                          String& className,
                          CIMNamespaceName& _ns,
                          CIMRepository* _rep)
{
    String embSubName("CQL_EmbeddedSubClass");
    String embBaseName("CQL_EmbeddedBase");

    // IF the class is CIM_RunningOS, then we will setup
    // some references to CIM_ComputerSystem
    if (className == "CIM_RunningOS")
    {
        Array<CIMInstance> cSystems;
        const CIMName CSClass(String("CIM_ComputerSystem"));

        try
        {
            cSystems.appendArray(_rep->enumerateInstancesForClass( _ns,
                CSClass));
        }
        catch(Exception& e)
        {
            cout << endl << endl << "Exception: Invalid namespace/class: "
                 << e.getMessage() << endl << endl;
            return false;
        }

        // For every computer system instance, make a runningOS that has a
        // reference to it.
        //The RunningOS will be the instance that is stored.
        for (Uint32 i=0; i < cSystems.size(); i++)
        {
            CIMInstance runOS("CIM_RunningOS");
            runOS.addProperty(CIMProperty("Dependent",
                CIMValue(cSystems[i].getPath())));

            _instances.append(runOS);
        }
        return true;
    }

    if (className != String::EMPTY
       && className != embSubName
       && className != embBaseName)
    {
        // If the classname was specified, and was not an embedded object
        // class, then load its instances from the repository.
        try
        {
            const CIMName _testclass(className);
            _instances =_rep->enumerateInstancesForSubtree( _ns,
                _testclass, true);  // deep inh true

            // Sort the CQL instances to avoid the class ordering problem
            // that happens because the order depends on how the file system
            // orders the class files in the repository.
            sortInstances(_instances);
        }
        catch(Exception& e)
        {
          cout << endl << endl << "Exception: Invalid namespace/class: "
               << e.getMessage() << endl << endl;
          return false;
        }
    }
    else
    {
        // load all the non-embedded instances we support
        cout << endl << "Using default class names to test queries. "
             << endl << endl;
        const CIMName _testclass1(String("CQL_TestElement"));
        const CIMName _testclass2(String("CIM_ComputerSystem"));
        try
        {
            // Deep inh = true for CQL_TestElement to also get
            // CQL_TestPropertyTypes and CQL_TestPropertyTypesMissing
            _instances = _rep->enumerateInstancesForSubtree( _ns,
                _testclass1, true); // deep inh true

            // Sort the CQL instances to avoid the class ordering problem that
            // happens because the order depends on how the file system orders
            // the class files in the repository.
            // NOTE - do not sort the CIM_ComputerSystem because the resgood
            // files expect the CIM_ComputerSystem to be after the CQL
            // instances.
            sortInstances(_instances);

            // only get the CIM_ComputerSystem
            _instances.appendArray(_rep->enumerateInstancesForClass( _ns,
                _testclass2));
        }
        catch(Exception& e)
        {
            cout << endl << endl << "Exception: Invalid namespace/class: "
                 << e.getMessage() << endl << endl;
            return false;
        }

        if (className == embSubName || className == embBaseName)
        {
            // If the embedded object classname was specified, then build its
            // instances.
            // Note: this will remove the other instances from the array.
            buildEmbeddedObjects(_ns,
                               _instances,
                               _rep);
        }
    }
    return true;
}

void help(const char* command){
        cout << command << " queryFile [option]" << endl;
        cout << " options:" << endl;
        cout << " -test: ";
        cout << "1 = evaluate" << endl
            << "        2 = apply projection" << endl
            << "        3 = get property list" << endl;
        cout << "        4 = validate properties" << endl;
        cout << "        5 = normalize to DOC" << endl;
        cout << " -className class" << endl;
        cout << " -nameSpace namespace (Example: root/SampleProvider)" << endl;
        cout << " -verbose" << endl << endl;
}

int main(int argc, char ** argv)
{
    // process options
    if (argc == 1 || (argc > 1 && strcmp(argv[1],"-h") == 0) )
    {
        help(argv[0]);
        exit(0);
    }

    // Since the output of this program will be compared with
    // a master output file, and the master file will have default
    // messages, turn off ICU message loading.
    MessageLoader::_useDefaultMsg = true;

    String testOption;
    String className;
    String nameSpace;

    for (int i = 0; i < argc; i++)
    {
        if ((strcmp(argv[i],"-test") == 0) && (i+1 < argc))
          testOption = argv[i+1];
        if ((strcmp(argv[i],"-className") == 0) && (i+1 < argc))
          className = argv[i+1];
        if ((strcmp(argv[i],"-nameSpace") == 0) && (i+1 < argc))
          nameSpace = argv[i+1];
        if ((strcmp(argv[i],"-verbose") == 0))
          cqlcli_verbose = true;
    }

    { // necessary when testing with Purifty so that _statements goes out of
      // scope before the program exits

        Array<CQLSelectStatement> _statements;

        // setup test environment

        // get the configuration variable PEGASUS_HOME
        const char* peg_home = getenv("PEGASUS_HOME");
        if (peg_home == NULL)
        {
            cout << "PEGASUS_HOME needs to be set to run this test." << endl;
            exit(-1);
        }
        String repositoryDir(peg_home);
        repositoryDir.append("/");

        // get the makefile build config variable REPOSITORY_NAME
        const char* repo_name = getenv("REPOSITORY_NAME");
        if (repo_name == NULL)
            repositoryDir.append("repository");
        else
            repositoryDir.append(repo_name);

        //
        // Comment out the above 3 lines and umcomment the line
        // below when testing with Rational Purify
        //

        //String repositoryDir("c:/pegasus-cvs/pegasus");

        CIMNamespaceName _ns;
        if (nameSpace != String::EMPTY)
        {
            _ns = nameSpace;
        }else
        {
            cout << "Using root/SampleProvider as default namespace." << endl;
            _ns = String("root/SampleProvider");
        }

        CIMRepository* _rep = new CIMRepository(repositoryDir);
        RepositoryQueryContext _ctx(_ns, _rep);
        String lang("CIM:CQL");
        String query("dummy statement");
        CQLSelectStatement _ss(lang,query,_ctx);
        if (_ss.getQuery() != query || _ss.getQueryLanguage() != lang)
        {
            cout << "ERROR: unable to get query or query language from"
                    " select statement" << endl;
            return 1;
        }

        char text[1024];
        char* _text;

        // setup Test Instances
        Array<CIMInstance> _instances;

        if (!populateInstances(_instances, className, _ns, _rep))
            return 1;

        // demo setup
        if (argc == 3 && strcmp(argv[2],"Demo") == 0)
        {
            cout << "Running Demo..." << endl;
            _instances.clear();
            const CIMName _testclassDEMO(String("CIM_Process"));
            _instances.appendArray(_rep->enumerateInstancesForSubtree( _ns,
                _testclassDEMO, true));
            _instances.remove(6,6);
        }

        for (Uint32 i = 0; i < _instances.size(); i++)
        {
            CIMObjectPath op = _instances[i].getPath();
            op.setHost("a.b.com");
            op.setNameSpace(_ns);
            _instances[i].setPath(op);
        }
        // setup input stream
        if (argc >= 2)
        {
            ifstream queryInputSource(argv[1]);
            if (!queryInputSource)
            {
                cout << "Cannot open input file.\n" << endl;
                return 1;
            }
            int statementsInError = 0;
            int lineNum = 0;
            while (!queryInputSource.eof())
            {
                lineNum++;
                queryInputSource.getline(text, 1024);
                char* _ptr = text;
                _text = strcat(_ptr,"\n");
                // check for comments and ignore
                // a comment starts with a # as the first non whitespace
                // character on the line
                //
                char _comment = '#';
                int i = 0;

                // While loop to ignore whitespace
                while (text[i] == ' ' || text[i] == '\t') i++;

                if (text[i] != _comment)
                {
                    if (!(strlen(_text) < 2) && i == 0)
                    {
                        try
                        {
                            CQLParser::parse(text,_ss);
                            _statements.append(_ss);
                        } // end-try
                        catch(Exception& e)
                        {
                            cout << "Caught Exception: "
                                 << getStatementString(e.getMessage())
                                 << endl;
                            try
                            {
                                String stmt(text);
                                cout << "Statement with error = "
                                     << getStatementString(stmt) << endl;
                            }
                            catch (Exception & e1)
                            {
                                cout << "Error printing statement: "
                                     << getStatementString(e1.getMessage())
                                     << endl;
                            }

                            _ss.clear();
                            statementsInError++;
                        } // end-catch
                    } // end if (strlen
                    else
                    {
                        if (cqlcli_verbose)
                            cout << "IGNORING line " << lineNum << endl;
                    }
                }  // end if (text[i] != _comment)

                // while !eof behaves differently on HP-UX, seems like it takes
                // an extra iteration to hit eof, this leaves "text" with
                // the previous value from getline(..), which causes a
                // duplicate parse of the last select statement in the query
                // file,
                // FIX: we clear text before doing another getline(..)
                //
                text[0] = 0;

            } // end-while
            queryInputSource.close();
            if (statementsInError)
            {
                cout << "There were " << statementsInError
                     << " statements that did NOT parse." << endl;
                // return 1;
            }
            try
            {
                _applyProjection(_statements,_instances, testOption);
                _validateProperties(_statements,_instances, testOption);
                _getPropertyList(_statements,_instances, _ns, testOption);
                _evaluate(_statements,_instances, testOption);
                _normalize(_statements,_instances, testOption);
            }
            catch(Exception e)
            {
                cout << getStatementString(e.getMessage()) << endl;
            }
            catch(...)
            {
            cout << "CAUGHT ... BADNESS HAPPENED!!!" << endl;
            }
        }
        else
        {
            cout << "Invalid number of arguments.\n" << endl;
        }
        delete _rep; // cleanup repository pointer
    }// necessary when testing with Purifty so that _statements goes
     // out of scope before the program exits
     //
    return 0;
}

