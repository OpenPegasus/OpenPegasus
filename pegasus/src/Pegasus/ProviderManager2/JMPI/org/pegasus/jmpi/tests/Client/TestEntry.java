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

package Client;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.util.jar.JarEntry;
import java.util.zip.ZipEntry;

/**
 * This is the class that contains information about a drawing class.
 *
 * @author  Mark Hamzy
 * @version "%I%"
 */
public class TestEntry
{
   /**
    * The drawing class resides in a ZIP file.
    */
   public static final int ZIPFILE = 1;
   /**
    * The drawing class resides in a ZIP file.
    */
   public static final int JARFILE = 2;
   /**
    * The drawing class resides in a normal .class file.
    */
   public static final int FILE    = 3;

   /**
    * This is the class constructor.
    *
    * @param szMenuName This is the name that is displayed on menus.
    * @param iType      This is the information about the type of storage.
    * @param o          This is a general storage object that is dependant on
    *                   the iType parameter.  For ZIPFILE types, it is the
    *                   ZipEntry object.  For FILE types, it is the file name
    *                   without the szFileEnd ending.
    * @param szFileEnd  This is the ending file extension.  It is normally <code>".class"</code>.
    */
   TestEntry (String szMenuName, int iType, Object o, String szFileEnd)
   {
      szMenuName_d  = szMenuName;
      o_d           = o;
      iType_d       = iType;
      szFileEnd_d   = szFileEnd;
      szGroupName_d = null;
      instance_d    = null;
   }

   /**
    * Returns the menu name.
    *
    * @return String The menu name.
    */
   public String getMenuName ()
   {
      return szMenuName_d;
   }

   /**
    * Returns the loadable class name.
    *
    * @return String The class name.
    */
   public String getClassName ()
   {
      String s = "";

      if (FILE == iType_d)
      {
         return (String)o_d;
      }
      else if (JARFILE == iType_d)
      {
         JarEntry je = (JarEntry)o_d;

         s = je.getName ();

         // The class loader does not like loading DIR/CLASSNAME for packages.
         s = s.replaceAll ("/", ".");
      }
      else if (ZIPFILE == iType_d)
      {
         ZipEntry ze = (ZipEntry)o_d;

         s = ze.getName ();
      }

      return s.substring (0, s.length () - szFileEnd_d.length ());
   }

   public Object getObject ()
   {
      return o_d;
   }

   public void setObject (Object o)
   {
      o_d = o;
   }

   public Object getInstance ()
   {
      return instance_d;
   }

   public void setInstance (Object o)
   {
      instance_d = o;
   }

   /**
    * Returns the group name.
    *
    * @return String The group name.
    */
   public String getGroup ()
   {
      return szGroupName_d;
   }

   /**
    * Sets the group name.
    *
    * @param szGrouName This is the name of the group.
    */
   public void setGroup (String szGroupName)
   {
      szGroupName_d = szGroupName;
   }

   /**
    * Returns the selection status.
    *
    * @return boolean The selection status.
    */
   public boolean getSelected ()
   {
      return fIsSelected_d;
   }

   /**
    * Sets the selection status.
    *
    * @param fState This is the selection state.
    */
   public void setSelected (boolean fState)
   {
      fIsSelected_d = fState;
   }

   /**
    * Class printing out method <code>System.out.println ()</code>
    *
    * @return String The human readable version.
    */
   public String toString ()
   {
      java.util.Hashtable h = new java.util.Hashtable ();
      Class               c = getClass ();
      Field[]             f = c.getDeclaredFields ();

      for (int i = 0; i < f.length; i++)
      {
         if (Modifier.isFinal (f[i].getModifiers ()))
            // Don't print #defines
            continue;

         try
         {
            Object o = f[i].get (this);

            if (o == null)
               o = "null";
            h.put (f[i].getName (), o);
         }
         catch (IllegalAccessException e)
         {
            h.put (f[i].getName (), "???");
         }
      }

      return c.getName () + "@" + this.hashCode () + h;
   }

   /**
    * This contains the menu name.
    */
   private String  szMenuName_d;
   /**
    * This contains an enumerated file storage type.
    */
   private int     iType_d;
   /**
    * This contains secondary information based on iType_d.
    */
   private Object  o_d;
   /**
    * This is the file ending extension.  It is normally <code>".class"</code>
    */
   private String  szFileEnd_d;
   /**
    * This is the group name.
    */
   private String  szGroupName_d;
   /**
    * This is the selection status.
    */
   private boolean fIsSelected_d = false;
   /**
    * This is the instantiated instance.
    */
   private Object  instance_d    = null;
}
