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
package Associations;

import java.util.Iterator;
import java.util.Vector;
import org.pegasus.jmpi.AssociatorProvider;
import org.pegasus.jmpi.CIMClass;
import org.pegasus.jmpi.CIMDataType;
import org.pegasus.jmpi.CIMException;
import org.pegasus.jmpi.CIMInstance;
import org.pegasus.jmpi.CIMObjectPath;
import org.pegasus.jmpi.CIMOMHandle;
import org.pegasus.jmpi.CIMProperty;
import org.pegasus.jmpi.CIMValue;
import org.pegasus.jmpi.InstanceProvider;
import org.pegasus.jmpi.OperationContext;
import org.pegasus.jmpi.UnsignedInt8;
import org.pegasus.jmpi.UnsignedInt64;

public class JMPIAssociationProvider
       implements InstanceProvider,
                  AssociatorProvider
{
   private CIMOMHandle    handle                 = null;
   private Vector         paths                  = new Vector ();
   private Vector         instances              = new Vector ();
   private boolean        fEnableModifications   = true;
   private final boolean  DEBUG                  = false;

   public void initialize (CIMOMHandle handle)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::initialize: handle = " + handle);
      }

      this.handle = handle;

      createDefaultInstances ();
   }

   public void cleanup ()
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::cleanup");
      }
   }

   public CIMObjectPath createInstance (CIMObjectPath    cop,
                                        CIMInstance      cimInstance)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::createInstance: cop         = " + cop);
         System.err.println ("JMPIAssociationProvider::createInstance: cimInstance = " + cimInstance);
      }

      throw new CIMException (CIMException.CIM_ERR_NOT_SUPPORTED);
   }

   public void deleteInstance (CIMObjectPath    cop)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::deleteInstance: cop  = " + cop);
      }

      throw new CIMException (CIMException.CIM_ERR_NOT_SUPPORTED);
   }

   // enumerateInstanceNames
   public Vector enumInstances (CIMObjectPath    cop,
                                boolean          deepInheritance,
                                CIMClass         cimClass)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::enumerateInstanceNames: cop             = " + cop);
         System.err.println ("JMPIAssociationProvider::enumerateInstanceNames: deepInheritance = " + deepInheritance);
         System.err.println ("JMPIAssociationProvider::enumerateInstanceNames: cimClass        = " + cimClass);
      }

      // Enusre that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHER))
      {
         return _enumerateInstanceNames (teacherInstances);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_STUDENT))
      {
         return _enumerateInstanceNames (studentInstances);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHERSTUDENT))
      {
         return _enumerateInstanceNames (TSassociationInstances);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_ADVISORSTUDENT))
      {
         return _enumerateInstanceNames (ASassociationInstances);
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public Vector enumInstances (CIMObjectPath    cop,
                                boolean          deepInheritance,
                                CIMClass         cimClass,
                                boolean          localOnly)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::enumerateInstances: cop             = " + cop);
         System.err.println ("JMPIAssociationProvider::enumerateInstances: deepInheritance = " + deepInheritance);
         System.err.println ("JMPIAssociationProvider::enumerateInstances: cimClass        = " + cimClass);
         System.err.println ("JMPIAssociationProvider::enumerateInstances: localOnly       = " + localOnly);
      }

      // Enusre that the namespace is valid
      if (!cop.getNameSpace ().equalsIgnoreCase (NAMESPACE))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      Vector vectorReturn = null;

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHER))
      {
         vectorReturn = _enumerateInstances (teacherInstances);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_STUDENT))
      {
         vectorReturn = _enumerateInstances (studentInstances);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHERSTUDENT))
      {
         vectorReturn = _enumerateInstances (TSassociationInstances);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_ADVISORSTUDENT))
      {
         vectorReturn = _enumerateInstances (ASassociationInstances);
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }

      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::enumerateInstances: returning " + vectorReturn.size () + " instances");
      }

      return vectorReturn;
   }

   public CIMInstance getInstance (CIMObjectPath    cop,
                                   CIMClass         cimClass,
                                   boolean          localOnly)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::getInstance: cop       = " + cop);
         System.err.println ("JMPIAssociationProvider::getInstance: cimClass  = " + cimClass);
         System.err.println ("JMPIAssociationProvider::getInstance: localOnly = " + localOnly);
      }

      CIMObjectPath localObjectPath = (CIMObjectPath)cop.clone ();
      localObjectPath.setHost ("");
      localObjectPath.setNameSpace (NAMESPACE);

      // Ensure that the class exists in the specified namespace
      if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHER))
      {
         return _getInstance (teacherInstances, localObjectPath);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_STUDENT))
      {
         return _getInstance (studentInstances, localObjectPath);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHERSTUDENT))
      {
         return _getInstance (TSassociationInstances, localObjectPath);
      }
      else if (cop.getObjectName ().equalsIgnoreCase (SAMPLE_ADVISORSTUDENT))
      {
         return _getInstance (ASassociationInstances, localObjectPath);
      }
      else
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS);
      }
   }

   public void setInstance (CIMObjectPath    cop,
                            CIMInstance      cimInstance)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::modifyInstance: cop          = " + cop);
         System.err.println ("JMPIAssociationProvider::modifyInstance: cimInstance  = " + cimInstance);
      }

      throw new CIMException (CIMException.CIM_ERR_NOT_SUPPORTED);
   }

   public Vector execQuery (CIMObjectPath    cop,
                            String           queryStatement,
                            int              queryLanguage,
                            CIMClass         cimClass)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::execQuery: cop            = " + cop);
         System.err.println ("JMPIAssociationProvider::execQuery: queryStatement = " + queryStatement);
         System.err.println ("JMPIAssociationProvider::execQuery: queryLanguage  = " + queryLanguage);
         System.err.println ("JMPIAssociationProvider::execQuery: cimClass       = " + cimClass);
      }

      throw new CIMException (CIMException.CIM_ERR_NOT_SUPPORTED);
   }

   public Vector associatorNames (CIMObjectPath    assocName,
                                  CIMObjectPath    objectName,
                                  String           resultClass,
                                  String           role,
                                  String           resultRole)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::associatorNames: assocName   = " + assocName);
         System.err.println ("JMPIAssociationProvider::associatorNames: objectName  = " + objectName);
         System.err.println ("JMPIAssociationProvider::associatorNames: resultClass = " + resultClass);
         System.err.println ("JMPIAssociationProvider::associatorNames: role        = " + role);
         System.err.println ("JMPIAssociationProvider::associatorNames: resultRole  = " + resultRole);
      }

      // Enusre that the namespace is valid
      if (!assocName.getNameSpace ().equalsIgnoreCase (NAMESPACE))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      CIMObjectPath localObjectPath = new CIMObjectPath (objectName.getObjectName (),
                                                         objectName.getKeys ());
      localObjectPath.setHost ("");
      localObjectPath.setNameSpace (NAMESPACE);

      Vector vectorReturn = null;

      if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHERSTUDENT))
      {
          vectorReturn = _associatorNames (TSassociationInstances,
                                           localObjectPath,
                                           role,
                                           resultClass,
                                           resultRole);
      }
      else if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_ADVISORSTUDENT))
      {
          vectorReturn = _associatorNames (ASassociationInstances,
                                           localObjectPath,
                                           role,
                                           resultClass,
                                           resultRole);
      }
      else
      {
          throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS,
                                   assocName.getObjectName () + " is not supported");
      }

      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::associatorNames: returning " + vectorReturn.size () + " instances");
      }

      return vectorReturn;
   }

   public Vector associators (CIMObjectPath    assocName,
                              CIMObjectPath    objectName,
                              String           resultClass,
                              String           role,
                              String           resultRole,
                              boolean          includeQualifiers,
                              boolean          includeClassOrigin,
                              String[]         propertyList)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::associators: assocName          = " + assocName);
         System.err.println ("JMPIAssociationProvider::associators: objectName         = " + objectName);
         System.err.println ("JMPIAssociationProvider::associators: resultClass        = " + resultClass);
         System.err.println ("JMPIAssociationProvider::associators: role               = " + role);
         System.err.println ("JMPIAssociationProvider::associators: resultRole         = " + resultRole);
         System.err.println ("JMPIAssociationProvider::associators: includeQualifiers  = " + includeQualifiers);
         System.err.println ("JMPIAssociationProvider::associators: includeClassOrigin = " + includeClassOrigin);
         System.err.println ("JMPIAssociationProvider::associators: propertyList       = " + propertyList);
      }

      // Enusre that the namespace is valid
      if (!assocName.getNameSpace ().equalsIgnoreCase (NAMESPACE))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      CIMObjectPath localObjectPath = new CIMObjectPath (objectName.getObjectName (),
                                                         objectName.getKeys ());
      localObjectPath.setHost ("");
      localObjectPath.setNameSpace (NAMESPACE);

      Vector vectorReturn = null;

      if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHERSTUDENT))
      {
          vectorReturn = _associators (TSassociationInstances,
                                       localObjectPath,
                                       role,
                                       resultClass,
                                       resultRole);
      }
      else if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_ADVISORSTUDENT))
      {
          vectorReturn = _associators (ASassociationInstances,
                                       localObjectPath,
                                       role,
                                       resultClass,
                                       resultRole);
      }
      else
      {
          throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS,
                                   assocName.getObjectName () + " is not supported");
      }

      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::associators: returning " + vectorReturn.size () + " instances");
      }

      return vectorReturn;
   }

   public Vector referenceNames (CIMObjectPath    assocName,
                                 CIMObjectPath    objectName,
                                 String           role)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::referenceNames: assocName  = " + assocName);
         System.err.println ("JMPIAssociationProvider::referenceNames: objectName = " + objectName);
         System.err.println ("JMPIAssociationProvider::referenceNames: role       = " + role);
      }

      // Enusre that the namespace is valid
      if (!assocName.getNameSpace ().equalsIgnoreCase (NAMESPACE))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      CIMObjectPath localObjectPath = new CIMObjectPath (objectName.getObjectName (),
                                                         objectName.getKeys ());
      localObjectPath.setHost ("");
      localObjectPath.setNameSpace (NAMESPACE);

      // Filter the instances from the list of association instances against
      // the specified role filter
      //
      Vector resultInstances;

      if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHERSTUDENT))
      {
         resultInstances = _filterAssociationInstancesByRole (TSassociationInstances,
                                                              localObjectPath,
                                                              role);
      }
      else if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_ADVISORSTUDENT))
      {
         resultInstances = _filterAssociationInstancesByRole (ASassociationInstances,
                                                              localObjectPath,
                                                              role);
      }
      else
      {
          throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS,
                                   assocName.getObjectName () + " is not supported");
      }

      Vector vectorReturn = new Vector ();

      // return the instance names
      for (int i = 0, si = resultInstances.size (); i < si; i++)
      {
          CIMObjectPath objectPath = ((CIMInstance)resultInstances.elementAt (i)).getObjectPath ();

          vectorReturn.addElement (objectPath);
      }

      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::referenceNames: returning " + vectorReturn.size () + " instances");
      }

      return vectorReturn;
   }

   public Vector references (CIMObjectPath    assocName,
                             CIMObjectPath    objectName,
                             String           role,
                             boolean          includeQualifiers,
                             boolean          includeClassOrigin,
                             String[]         propertyList)
      throws CIMException
   {
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::references: assocName          = " + assocName);
         System.err.println ("JMPIAssociationProvider::references: objectName         = " + objectName);
         System.err.println ("JMPIAssociationProvider::references: role               = " + role);
         System.err.println ("JMPIAssociationProvider::references: includeQualifiers  = " + includeQualifiers);
         System.err.println ("JMPIAssociationProvider::references: includeClassOrigin = " + includeClassOrigin);
         System.err.println ("JMPIAssociationProvider::references: propertyList       = " + propertyList);
      }

      // Enusre that the namespace is valid
      if (!assocName.getNameSpace ().equalsIgnoreCase (NAMESPACE))
      {
         throw new CIMException (CIMException.CIM_ERR_INVALID_NAMESPACE);
      }

      CIMObjectPath localObjectPath = new CIMObjectPath (objectName.getObjectName (),
                                                         objectName.getKeys ());
      localObjectPath.setHost ("");
      localObjectPath.setNameSpace (NAMESPACE);

      // Filter the instances from the list of association instances against
      // the specified role filter
      //
      Vector resultInstances;

      if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_TEACHERSTUDENT))
      {
         resultInstances = _filterAssociationInstancesByRole (TSassociationInstances,
                                                              localObjectPath,
                                                              role);
      }
      else if (assocName.getObjectName ().equalsIgnoreCase (SAMPLE_ADVISORSTUDENT))
      {
         resultInstances = _filterAssociationInstancesByRole (ASassociationInstances,
                                                              localObjectPath,
                                                              role);
      }
      else
      {
          throw new CIMException (CIMException.CIM_ERR_INVALID_CLASS,
                                  assocName.getObjectName () + " is not supported");
      }

      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::references: returning " + resultInstances.size () + " instances");
      }

      return resultInstances;
   }

   private void createDefaultInstances ()
   {
       //
       // create 4 instances of the SAMPLE_TEACHER class
       //
       teacherInstances.add (createInstance (SAMPLE_TEACHER, "Teacher1", 1));
       teacherInstances.add (createInstance (SAMPLE_TEACHER, "Teacher2", 2));
       teacherInstances.add (createInstance (SAMPLE_TEACHER, "Teacher3", 3));
       teacherInstances.add (createInstance (SAMPLE_TEACHER, "Teacher4", 4));

       //
       // create 3 instances of the SAMPLE_STUDENT class
       //
       studentInstances.add (createInstance (SAMPLE_STUDENT, "Student1", 1));
       studentInstances.add (createInstance (SAMPLE_STUDENT, "Student2", 2));
       studentInstances.add (createInstance (SAMPLE_STUDENT, "Student3", 3));

       //
       // create the instances for the SAMPLE_TEACHERSTUDENT association class
       //    (Teacher1, Student1)
       //    (Teacher1, Student2)
       //    (Teacher1, Student3)
       //    (Teacher2, Student1)
       //    (Teacher2, Student2)
       //    (Teacher3, Student2)
       //    (Teacher3, Student3)
       //    (Teacher4, Student1)
       //
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 0), getPath (studentInstances, 0)));
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 0), getPath (studentInstances, 1)));
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 0), getPath (studentInstances, 2)));
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 1), getPath (studentInstances, 0)));
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 1), getPath (studentInstances, 1)));
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 2), getPath (studentInstances, 1)));
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 2), getPath (studentInstances, 2)));
       TSassociationInstances.add (createTSAssociationInstance (getPath (teacherInstances, 3), getPath (studentInstances, 0)));

       //
       // create the instances for the SAMPLE_ADVISORSTUDENT association class
       //    (Teacher1, Student1)
       //    (Teacher1, Student2)
       //    (Teacher2, Student3)
       //
       ASassociationInstances.add (createASAssociationInstance (getPath (teacherInstances, 0), getPath (studentInstances, 0)));
       ASassociationInstances.add (createASAssociationInstance (getPath (teacherInstances, 0), getPath (studentInstances, 1)));
       ASassociationInstances.add (createASAssociationInstance (getPath (teacherInstances, 1), getPath (studentInstances, 2)));
   }

   private CIMObjectPath getPath (Vector v, int i)
   {
       try
       {
           CIMInstance c = (CIMInstance)v.elementAt (i);

           return c.getObjectPath ();
       }
       catch (Exception e)
       {
           System.err.println ("JMPIAssociationProvider::getPath: Caught Exception " + e);
       }

       return null;
   }

   private CIMInstance createInstance (String className, String name, int id)
   {
      try
      {
         CIMProperty[] props    = {
            new CIMProperty ("Name",
                             new CIMValue (name)),                                    /* @TBD: cimcli doesnt pass this in: className */
            new CIMProperty ("Identifier",
                             new CIMValue (new UnsignedInt8 (Integer.toString (id)))) /* @TBD: ... className */
         };
         CIMObjectPath cop      = new CIMObjectPath (className, NAMESPACE);
         CIMInstance   instance = new CIMInstance (className);

         for (int i = 0; i < props.length; i++)
         {
             instance.setProperty (props[i].getName (), props[i].getValue ());
             cop.addKey (props[i].getName (), props[i].getValue ());
         }

         cop.setHost ("");
         instance.setObjectPath (cop);

         return instance;
      }
      catch (CIMException e)
      {
          return null;
      }
   }

   private CIMInstance createTSAssociationInstance (CIMObjectPath ref1, CIMObjectPath ref2)
   {
      try
      {
         CIMProperty[] props    = {
            new CIMProperty ("Teaches",
                             new CIMValue (ref1)),
            new CIMProperty ("TaughtBy",
                             new CIMValue (ref2))
         };
         CIMObjectPath cop      = new CIMObjectPath (SAMPLE_TEACHERSTUDENT, NAMESPACE);
         CIMInstance   instance = new CIMInstance (SAMPLE_TEACHERSTUDENT);

         for (int i = 0; i < props.length; i++)
         {
             instance.setProperty (props[i].getName (), props[i].getValue ());
             cop.addKey (props[i].getName (), props[i].getValue ());
         }

         cop.setHost ("");
         instance.setObjectPath (cop);

         return instance;
      }
      catch (CIMException e)
      {
          return null;
      }
   }

   private CIMInstance createASAssociationInstance (CIMObjectPath ref1, CIMObjectPath ref2)
   {
      try
      {
         CIMProperty[] props    = {
            new CIMProperty ("Advises",
                             new CIMValue (ref1)),
            new CIMProperty ("AdvisedBy",
                             new CIMValue (ref2))
         };
         CIMObjectPath cop      = new CIMObjectPath (SAMPLE_ADVISORSTUDENT, NAMESPACE);
         CIMInstance   instance = new CIMInstance (SAMPLE_ADVISORSTUDENT);

         for (int i = 0; i < props.length; i++)
         {
             instance.setProperty (props[i].getName (), props[i].getValue ());
             cop.addKey (props[i].getName (), props[i].getValue ());
         }

         cop.setHost ("");
         instance.setObjectPath (cop);

         return instance;
      }
      catch (CIMException e)
      {
          return null;
      }
   }

   private CIMInstance _getInstance (Vector        instances,
                                     CIMObjectPath localObjectPath)
   {
      Iterator iteratorInstances = instances.iterator ();

      while (iteratorInstances.hasNext ())
      {
         CIMInstance   elm     = (CIMInstance)iteratorInstances.next ();
         CIMObjectPath elmPath = elm.getObjectPath ();

         if (equals (localObjectPath, elmPath))
         {
            return elm;
         }
      }

      return null;
   }

   private Vector _enumerateInstances (Vector instances)
   {
      return instances;
   }

   private Vector _enumerateInstanceNames (Vector instances)
   {
      Vector   returnPaths       = new Vector ();
      Iterator iteratorInstances = instances.iterator ();

      while (iteratorInstances.hasNext ())
      {
         CIMInstance elm = (CIMInstance)iteratorInstances.next ();

         returnPaths.addElement (elm.getObjectPath ());
      }

      return returnPaths;
   }

   private Vector _associators (Vector        associationInstances,
                                CIMObjectPath localObjectPath,
                                String        role,
                                String        resultClass,
                                String        resultRole)
   {
      Vector vectorReturn = new Vector ();

      // Filter the instances from the list of association instances against
      // the specified role filter
      //
      Vector assocInstances = null;

      assocInstances = _filterAssociationInstancesByRole (associationInstances,
                                                          localObjectPath,
                                                          role);
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::_associators: assocInstances.size () = " + assocInstances.size ());
      }

      // Now filter the result association instances against the specified
      // resultClass and resultRole filters
      //
      for (int i = 0, si = assocInstances.size (); i < si; i++)
      {
         Vector resultPaths;

         resultPaths = _filterAssociationInstances ((CIMInstance)assocInstances.elementAt (i),
                                                    localObjectPath,
                                                    resultClass,
                                                    resultRole);
         if (DEBUG)
         {
            System.err.println ("JMPIAssociationProvider::_associators: resultPaths.size () = " + resultPaths.size ());
         }

         for (int j = 0, sj = resultPaths.size (); j < sj; j++)
         {
            String className = ((CIMObjectPath)resultPaths.elementAt (j)).getObjectName ();

            if (className.equalsIgnoreCase (SAMPLE_TEACHER))
            {
               if (DEBUG)
               {
                  System.err.println ("JMPIAssociationProvider::_associators: teacherInstances.size () = " + teacherInstances.size ());
               }

               // find instance that corresponds to the reference
               for (int k = 0, s = teacherInstances.size (); k < s; k++)
               {
                  CIMObjectPath path = ((CIMInstance)teacherInstances.elementAt (k)).getObjectPath ();

                  if (DEBUG)
                  {
                     System.err.println ("JMPIAssociationProvider::_associators: path                      = " + path);
                     System.err.println ("JMPIAssociationProvider::_associators: resultPaths.elementAt (" + j + ") = " + resultPaths.elementAt (j));
                  }

                  if (equals (((CIMObjectPath)resultPaths.elementAt (j)), path))
                  {
                     if (DEBUG)
                     {
                        System.err.println ("JMPIAssociationProvider::_associators: adding!");
                     }

                     // return the instance
                     // Note: See below.
                     vectorReturn.addElement (((CIMInstance)teacherInstances.elementAt (k)).clone ());
                  }
               }
            }
            else if (className.equalsIgnoreCase (SAMPLE_STUDENT))
            {
               if (DEBUG)
               {
                  System.err.println ("JMPIAssociationProvider::_associators: studentInstances.size () = " + studentInstances.size ());
               }

               // find instance that corresponds to the reference
               for (int k = 0, s = studentInstances.size (); k < s; k++)
               {
                  CIMObjectPath path = ((CIMInstance)studentInstances.elementAt (k)).getObjectPath ();

                  if (DEBUG)
                  {
                     System.err.println ("JMPIAssociationProvider::_associators: path                      = " + path);
                     System.err.println ("JMPIAssociationProvider::_associators: resultPaths.elementAt (" + j + ") = " + resultPaths.elementAt (j));
                  }

                  if (equals (((CIMObjectPath)resultPaths.elementAt (j)), path))
                  {
                     if (DEBUG)
                     {
                        System.err.println ("JMPIAssociationProvider::_associators: adding!");
                     }

                     // return the instance
                     // Note: We must clone what is being returned since otherwise our
                     //       copy will be modified.
                     //       The CIMObjectPath will change from:
                     //          root/SampleProvider:JMPIAssociation_Student.Identifier=1,Name="Student1"
                     //       to:
                     //          root/SampleProvider:JMPIAssociation_Student.Name="Student1"
                     vectorReturn.addElement (((CIMInstance)studentInstances.elementAt (k)).clone ());
                  }
               }
            }
         }
      }

      return vectorReturn;
   }

   private Vector _associatorNames (Vector        associationInstances,
                                    CIMObjectPath localObjectPath,
                                    String        role,
                                    String        resultClass,
                                    String        resultRole)
   {
      Vector vectorReturn = new Vector ();

      // Filter the instances from the list of association instances against
      // the specified role filter
      //
      Vector assocInstances;

      assocInstances = _filterAssociationInstancesByRole (associationInstances,
                                                          localObjectPath,
                                                          role);
      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::_associatorNames: assocInstances.size () = " + assocInstances.size ());
      }

      // Now filter the result association instances against the specified
      // resultClass and resultRole filters
      //
      for (int i = 0, si = assocInstances.size (); i < si; i++)
      {
         Vector resultPaths;

         resultPaths = _filterAssociationInstances ((CIMInstance)assocInstances.elementAt (i),
                                                    localObjectPath,
                                                    resultClass,
                                                    resultRole);

         if (DEBUG)
         {
            System.err.println ("JMPIAssociationProvider::_associatorNames: resultPaths.size () = " + resultPaths.size ());
         }

         vectorReturn.addAll (resultPaths);
      }

      return vectorReturn;
   }

   private Vector _filterAssociationInstancesByRole (Vector        associationInstances,
                                                     CIMObjectPath targetObjectPath,
                                                     String        role)
   {
      Vector returnInstances = new Vector ();

      if (DEBUG)
      {
         System.err.println ("JMPIAssociationProvider::_filterAssociationInstancesByRole: associationInstances.size () = " + associationInstances.size ());
         System.err.println ("JMPIAssociationProvider::_filterAssociationInstancesByRole: targetObjectPath             = " + targetObjectPath);
         System.err.println ("JMPIAssociationProvider::_filterAssociationInstancesByRole: role                         = " + role);
      }

      try
      {
         // Filter the instances from the list of association instances against
         // the specified role filter
         //
         for (int i = 0, si = associationInstances.size (); i < si; i++)
         {
             CIMInstance instance = (CIMInstance)associationInstances.elementAt (i);

             // Search the association instance for all reference properties
             for (int j = 0, sj = instance.getPropertyCount (); j < sj; j++)
             {
                 CIMProperty p = instance.getProperty (j);

                 if (p.getType ().getType () == CIMDataType.REFERENCE)
                 {
                     CIMObjectPath copRef = (CIMObjectPath)p.getValue ().getValue ();

                     if (DEBUG)
                     {
                        System.err.println ("JMPIAssociationProvider::_filterAssociationInstancesByRole: p                            = " + p);
                        System.err.println ("JMPIAssociationProvider::_filterAssociationInstancesByRole: copRef                       = " + copRef);
                     }

                     if (  (role.equalsIgnoreCase (""))
                        || (p.getName ().equalsIgnoreCase (role))
                        )
                     {
                        if (DEBUG)
                        {
                            System.err.println ("JMPIAssociationProvider::_filterAssociationInstancesByRole: targetObjectPath             = " + targetObjectPath);
                        }

                        if (targetObjectPath.toString ().equalsIgnoreCase (copRef.toString ()))
                        {
                           if (DEBUG)
                           {
                               System.err.println ("JMPIAssociationProvider::_filterAssociationInstancesByRole: adding!");
                           }

                           returnInstances.addElement (instance);
                        }
                     }
                  }
              }
          }
      }
      catch (CIMException e)
      {
      }

      return returnInstances;
   }

   private Vector _filterAssociationInstances (CIMInstance   assocInstance,
                                               CIMObjectPath sourceObjectPath,
                                               String        resultClass,
                                               String        resultRole)
   {
      Vector returnPaths = new Vector ();

      try
      {
         if (DEBUG)
         {
            System.err.println ("JMPIAssociationProvider::_filterAssociationInstances: assocInstance.getPropertyCount () = " + assocInstance.getPropertyCount ());
            System.err.println ("JMPIAssociationProvider::_filterAssociationInstances: sourceObjectPath                  = " + sourceObjectPath);
            System.err.println ("JMPIAssociationProvider::_filterAssociationInstances: resultClass                       = " + resultClass);
            System.err.println ("JMPIAssociationProvider::_filterAssociationInstances: resultRole                        = " + resultRole);
         }

         // get all Reference properties
         for (int i = 0, si = assocInstance.getPropertyCount (); i < si; i++)
         {
            CIMProperty p = assocInstance.getProperty (i);

            if (p.getType ().getType () == CIMDataType.REFERENCE)
            {
               CIMObjectPath copRef = (CIMObjectPath)p.getValue ().getValue ();

               if (DEBUG)
               {
                  System.err.println ("JMPIAssociationProvider::_filterAssociationInstances: p                                 = " + p);
                  System.err.println ("JMPIAssociationProvider::_filterAssociationInstances: copRef                            = " + copRef);
               }

               if (!equals (sourceObjectPath, copRef))
               {
                  if (  resultClass.equalsIgnoreCase ("")
                     || resultClass == copRef.getObjectName ()
                     )
                  {
                     if (  (resultRole.equalsIgnoreCase (""))
                        || (p.getName ().equalsIgnoreCase (resultRole))
                        )
                     {
                        if (DEBUG)
                        {
                            System.err.println ("JMPIAssociationProvider::_filterAssociationInstances: adding!");
                        }

                        returnPaths.addElement (copRef);
                     }
                  }
               }
            }
         }
      }
      catch (CIMException e)
      {
      }

      return returnPaths;
   }

   private boolean equals (CIMObjectPath l, CIMObjectPath r)
   {
      String ls = l.toString ();
      String rs = r.toString ();

      return ls.equalsIgnoreCase (rs);
   }

   private Vector         teacherInstances       = new Vector ();
   private Vector         studentInstances       = new Vector ();
   private Vector         TSassociationInstances = new Vector ();
   private Vector         ASassociationInstances = new Vector ();

   private String         NAMESPACE              = new String ("root/SampleProvider");

   // Class names
   private String         SAMPLE_TEACHER         = new String ("JMPIAssociation_Teacher");
   private String         SAMPLE_STUDENT         = new String ("JMPIAssociation_Student");
   private String         SAMPLE_TEACHERSTUDENT  = new String ("JMPIAssociation_TeacherStudent");
   private String         SAMPLE_ADVISORSTUDENT  = new String ("JMPIAssociation_AdvisorStudent");
}
