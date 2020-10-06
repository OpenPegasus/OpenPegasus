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

#ifndef Pegasus_RepositoryUpgrade_h
#define Pegasus_RepositoryUpgrade_h

#include <Pegasus/Repository/CIMRepository.h>

#include <Pegasus/Common/MessageQueue.h>

#include <Pegasus/General/DynamicLibrary.h>

#include <Pegasus/Client/ClientAuthenticator.h>
#include <Pegasus/Client/CIMOperationRequestEncoder.h>

#include <Clients/cliutils/Command.h>
#include <Clients/cliutils/CommandException.h>

#include "RepositoryUpgradeException.h"

#include "SSPModule.h"
#include "SSPModuleTable.h"

PEGASUS_NAMESPACE_BEGIN

#ifdef NS_INTEROP
const CIMNamespaceName PEGASUS_NAMESPACE_PGINTEROP=
    CIMNamespaceName ("root/PG_InterOp");
#endif

/**

     The RepositoryUpgrade utility enables upgrade of a repository
     to a newer version of CIM schema.

     The utility will migrate schema extensions from an
     older repository to a new repository that is already installed with a
     new version of the CIM schema.

     The new and old repository paths are accepted as command line
     arguments. The new Repository path specifes the location of the repository
     initialized with a newer version of the CIM Schema. The old Repository path
     specifies the location of the Repository that contains the user defined
     elements to be migrated.

     Class extensions, qualifier extensions, instances that have been
     added to the pre-existing namespaces and new namespaces
     (includes qualifiers, classes and instances) that were
     added  to the old repository will be created in the
     new Repository.

     During the process of upgrade if an error occurs, an error message is
     displayed and the upgrade is aborted. If an Exception occurs during
     the creation of a Schema element, the CIM/XML request for creating the
     Schema element is logged to a file, an error message is displayed
     and the upgrade process is aborted.

*/

//
// In order to create CIM/XML request messages inherit from MessageQueue class.
//
class RepositoryUpgrade : public MessageQueue, public Command
{

public:

    /**

        Constructs a RepositoryUpgrade and initializes instance variables.

    */
    RepositoryUpgrade();

    /**
        Destructor.
    */
    ~RepositoryUpgrade ();

    /**
        Parses the command line, validates the options, and sets instance
        variables based on the option arguments.

        @param  argc   the number of command line arguments
        @param  argv   the string vector of command line arguments

        @exception  CommandFormatException  if an error is encountered in
                                            parsing the command line
    */
    void setCommand (Uint32 argc, char* argv []);

    /**
        Executes the command and writes the results to the PrintWriters.

        @param  outPrintWriter    the ostream to which output should be
                                  written
        @param  errPrintWriter    the ostream to which error output should be
                                  written

        @return  0                if the command is successful
                 1                if an error occurs in executing the command
    */
    Uint32 execute (
        PEGASUS_STD(ostream)& outPrintWriter,
        PEGASUS_STD(ostream)& errPrintWriter);

    /**
        The command name.
    */
    static const char   COMMAND_NAME [];

    /**

        Updates the new repository by importing class extensions, qualifiers,
        instances and namespace additions that have been made to the
        old repository.

    */
    void upgradeRepository();

    //
    // Derived from the MessageQueue base class.
    //
    virtual void handleEnqueue() {};

private:
    //
    // The type of operation specified on the command line.
    //
    Uint32 _optionType;

    //
    // The usage string.
    //
    String _usage;

    //
    // The option character used to specify the old repository path.
    //
    static const char   _OPTION_OLD_REPOSITORY_PATH;

    //
    // The option character used to specify the new repository path.
    //
    static const char   _OPTION_NEW_REPOSITORY_PATH;

    //
    // The option character used to display help info.
    //
    static const char   _OPTION_HELP;

    //
    // The option character used to display version info.
    //
    static const char   _OPTION_VERSION;

    //
    // Label for the usage string for this command.
    //
    static const char   _USAGE [];

    //
    // This constant signifies that an operation option has not been recorded
    //
    static const Uint32 _OPTION_TYPE_UNINITIALIZED;

    //
    // This constant signifies that the old repository path option has
    // been recorded
    //
    static const Uint32 _OPTION_TYPE_OLD_REPOSITORY_PATH;

    //
    // This constant signifies that the new repository path option has
    // been recorded
    //
    static const Uint32 _OPTION_TYPE_NEW_REPOSITORY_PATH;

    //
    // The constant representing a help operation
    //
    static const Uint32 _OPTION_TYPE_HELP;

    //
    // The constant representing a version display operation
    //
    static const Uint32 _OPTION_TYPE_VERSION;

#ifdef NS_INTEROP
    //
    // The option character used for interop support.
    //
    static const char   _OPTION_INTEROP;

    //
    // The constant representing that interop option has been specified
    //
    static const Uint32 _OPTION_TYPE_INTEROP;

    //
    // Indicates whether root/PG_InterOp has to be changed to interop
    //
    Boolean _optionInterop;
#endif

    //
    // Contains the old repository path.
    //
    String _oldRepositoryPath;

    //
    // Contains the new repository path.
    //
    String _newRepositoryPath;

    //
    // Indicates that the old repository path is set.
    //
    Boolean _oldRepositoryPathSet;

    //
    // Indicates that the new repository path is set.
    //
    Boolean _newRepositoryPathSet;

    //
    // Indicates whether the Special Processing modules have been initialized.
    //
    Boolean _modulesInitialized;

    //
    // Instance of CIMRepository used to access classes and namespaces from
    // the old repository.
    //
    CIMRepository* _oldRepository;

    //
    // Instance of CIMRepository used to access classes and namespaces from
    // the new repository.
    //
    CIMRepository* _newRepository;

    //
    // Instance of Client Authenticator used by Error processing to generate
    // a log of a failed CIM/XML request.
    //
    ClientAuthenticator _authenticator;

    //
    // Instance of request encoder used by error processing to generate
    // a log of a failed CIM/XML request.
    //
    CIMOperationRequestEncoder* _requestEncoder;

    //
    // Contains the list of classes to be ignored in the InterOp namespace.
    //
    Array<CIMName> _interopIgnoreClasses;

    //
    // Constant representing the name of the VERSION qualifier.
    //
    static const String     _VERSION_QUALIFIER_NAME;

    //
    // Defines the file extension for a CIM/XML request file.
    //
    static const String     _FILE_EXTENSION;

    //
    // Defines the path to store CIM/XML log file for a failed request.
    //
    static const String     _LOG_PATH;

    //
    // Count to keep track of failed instance creations. This count
    // is used in the filename that will log the failed request.
    //
    Uint32 instanceCount;

    //
    // Count to keep track of failed qualifier creations. This count
    // is used in the filename that will log the failed request.
    //
    Uint32 qualifierCount;

#ifdef ENABLE_MODULE_PROCESSING
    //
    // Contains handles to the Special Processing shared libraries.
    //
    DynamicLibrary _library[SSPModuleTable::NUM_MODULES];

    //
    // Indicates that the Special Processing Module is interested in
    // processing classes.
    //
    static const char* _CLASS_ONLY;

    //
    // Indicates that the Special Processing Module is interested in
    // processing qualifiers.
    //
    static const char* _QUALIFIER_ONLY;

    //
    // Indicates that the Special Processing Module is interested in processing
    // instances.
    //
    static const char* _INSTANCE_ONLY;

    //
    // Indicates that the Special Processing Module is interested in
    // processing classes, qualifiers and instances.
    //
    static const char* _ALL;

    //
    //  Special Processing Module entry point.
    //
    typedef SchemaSpecialProcessModule
             * (*CREATE_SSPMODULE_FUNCTION)();

    //
    //  Contains the handles to Special Processing Modules.
    //
    SchemaSpecialProcessModule* _sspModule[SSPModuleTable::NUM_MODULES];
#endif

    //
    // Contains the PEGASUS_HOME environment variable value used in
    // locating the Special Processing shared libraries.
    //
    String _pegasusHome;

    //
    // Compares the namespaces passed and returns namespaces that are present
    // in the old but not in the new repository.
    //
    // @param oldNamespaces a list of namespaces in old repository
    // @param newNamespaces a list of namespaces in new repository
    //
    // @return                  a list of missing namespaces
    //
    Array<CIMNamespaceName> _compareNamespaces(
                              const Array<CIMNamespaceName>& oldNamespaces ,
                              const Array<CIMNamespaceName>& newNamespaces);

    //
    // Creates namespaces in the new repository. Also adds the classes,
    // qualifiers from the namespaces into the new repository. Any instances
    // that exist in the given namespaces will be created by the _addInstances
    // method.
    //
    // @param namespaces    a list of namespaces to be added.
    //
    void _addNamespaces(const Array<CIMNamespaceName>& namespaces);

    //
    // Identifies the missing classes and existing classes and
    // passes them to the _processNewClasses and _processExistingClasses
    // methods.
    //
    // @param namespaceName namespace that is getting compared.
    //
    // @param oldClasses    classes in the old repository.
    //
    // @param newClasses    classes in the new repository.
    //
    void _processClasses( const CIMNamespaceName&   namespaceName,
                          const Array<CIMName>&         oldClasses,
                          Array<CIMName>&       newClasses);

#ifdef NS_INTEROP
    void _processInstance( CIMInstance&, Array<CIMName>);
#endif

    //
    // Processes the new classes in the hierarchical order and passes them to
    // _addClassToRepository.
    //
    // @param namespaceName namespace that is getting compared.
    // @param oldClasses    classes in the old repository.
    // @param newClasses    classes in the new repository.
    //
    void _processNewClasses( const CIMNamespaceName&    namespaceName,
                             Array<CIMName>&            oldClasses,
                             Array<CIMName>&        newClasses);


    //
    // Performs a version comparison with the existing classes in the new
    // repository. The following lists the behavior:
    //
    // 1. The version number existed in the old but not the new class.
    //    A warning message is displayed. The class is not imported to the
    //    new repository.
    // 2. Both classes contain versions and the old version is greater
    //    than the new version. A warning message is displayed. The class
    //    is not imported to the new repository.
    //
    //    If the new Repository contains a class with a higher version number
    //    then no messages are displayed.
    //
    //   @param namespaceName      namespacename
    //   @param existingClasses    list of classes already existing in the new
    //                             repository
    //
    void _processExistingClasses (const CIMNamespaceName& namespaceName,
                                  const Array<CIMName>&   existingClasses);


    //
    // Adds a class to the new repository.
    //
    // @param namespaceName     namespace to which the class belongs
    //
    // @param className          name of the class to be added
    //
    // @param existingClasses   list of classes that are already existing
    //                          in the new repository
    // @return   0              if the class was added successfully
    //
    //           1              if the class addition failed because a
    //                          dependent class did not exist. In this case
    //                          the class creation will be retried.
    //
    Uint32 _addClassToRepository (const CIMNamespaceName& namespaceName,
                                  const CIMName&          className,
                                  const Array<CIMName>    existingClasses);

    //
    // Check if the class exists in the old repository.
    //
    // @param namespaceName     namespace to which the class belongs
    //
    // @param className         name of the class to be added.
    //
    // @param dependenClassName class on which className is dependent on.

    // @return   true           if dependenClassName exists.
    //
    //           exception      if the dependentClassName does not exist.
    CIMClass _checkIfDependentClassExists(
        const CIMNamespaceName& namespaceName,
        const CIMName&          className,
        const CIMName&          dependentClassName);

    //
    // Adds instances from the old repository into the new repository.
    // If an instance already exists in the new repository then it
    // is not imported from the old repository.
    //
    void _addInstances ();

    //
    // Removes instances from the new repository when no loger used.
    //
    void _removeInstances();

    //
    // Adds qualifiers from the old repository into the new repository.
    // If a qualifier already exists in the new repository then it
    // is not imported.
    //
    // @param CIMNamespaceName      contains the Namespace name.
    //
    void _addQualifiers (const CIMNamespaceName &namespaceName);

    //
    // Logs a failed CIM/XML request in to an output file.
    //
    // @param outputFile    output filename.
    //
    void _logRequestToFile ( const String& outputFile );

    //
    // Logs an error message to indicate an error while adding a class.
    //
    // @param CIMNamespaceName  contains the Namespace name.
    //
    // @param CIMClass          contains the class.
    //
    // @param message           contains the error message if available,
    //                          otherwise set to String::EMPTY
    //
    // @exception               logs the request and propagates the
    //                          error encountered during create class.
    //
    void _logCreateClassError(
             const CIMNamespaceName& namespaceName,
             const CIMClass& oldClass,
             const String& message);

    //
    // Logs an error message to indicate an error while adding an instance.
    //
    // @param CIMNamespaceName  contains the Namespace name.
    //
    // @param CIMInstance       contains the instance.
    //
    // @param message           contains the error message if available
    //                          otherwise set to String::EMPTY
    //
    // @exception               logs the request and propagates the
    //                          error encountered during create instance.
    //
    void _logCreateInstanceError(
           const CIMNamespaceName& namespaceName,
           const CIMInstance& instance,
           const String&           message);

    //
    // Logs an error message to indicate an error while deleting an instance.
    //
    // @param CIMNamespaceName  contains the Namespace name.
    //
    // @param CIMObjectPath     contains the required instance object path.
    //
    // @param message           contains the error message if available
    //                          otherwise set to String::EMPTY
    //
    // @exception               logs the request and propagates the
    //                          error encountered during delete instance.
    //
    void _logDeleteInstanceError(
        const CIMNamespaceName& namespaceName,
        const CIMObjectPath& instanceName,
        const String& message);

    //
    // Logs an error message to indicate an error while adding an qualifier.
    //
    // @param CIMNamespaceName  contains the Namespace name.
    //
    // @param CIMQualifierDecl  contains the qualifier.
    //
    // @param message           contains the error message if available
    //                          otherwise set to String::EMPTY
    //
    // @exception               logs the request and propagates the
    //                          error encountered during set qualifier.
    //
    void _logSetQualifierError(
          const CIMNamespaceName& namespaceName,
          const CIMQualifierDecl& qualifier,
          const String&           message);

    /**
        Updates the Subscription instances in the repository. It calls
        _updateFilterHandlerReferences() to update the SystemName key
        in filter and hadler references.
        @param   nameSpace     The current namespace being considered
        @param   className     The name of the subscription, handler or filter
                               class whose instances in the current namespace
                               are to be checked and updated in case there
                               is any inconsistency in system namecurrent
                               namespace being considered
    */
    void _updateSubscriptionInstancesInRepository(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    /**
        Updates the Handler, Filter and ObjectManager instances in the 
        repository. It checks for the SystemName property value. If there
        is any inconsistency with the current system name retrieved using
        System::getFullyQualifiedSystemName() then the SystemName property
        and keyBindings are updated to the current value.

        @param   nameSpace     The current namespace being considered
        @param   className     The name of the handler, filter or objectManager
                               class whose instances in the current namespace
                               are to be checked and updated in case there
                               is any inconsistency in system namecurrent
                               namespace being considered
    */
    void _updateSystemNameKeyPropertyOfInstancesForClass(
        const CIMNamespaceName& nameSpace,
        const CIMName& className);

    /**
        Updates the SytemName key property in Filter and Handler
        references. It checks for the SystemName value in the
        keyBindings of Filter and Handler references. If there is any
        inconsistency with the current system name retrieved using
        System::getFullyQualifiedSystemName() then the SystemName
        keyBindings are updated to the current value.

        @param   instance      The current subscription instance
        @param   propertyName  The property of susbscription which needs to be
                               updated for correct system name. Its value is
                               either Filter or Handler
    */
    Boolean _updateFilterHandlerReference(
        CIMInstance& instance,
        const CIMName& propertyName);

    /**
        This function updates SystemName key property in subscription,
        filter, handler and objectmanager instances to current System Name
    */
    void _updateSystemNameKeyProperty();

#ifdef ENABLE_MODULE_PROCESSING
    //
    // Initializes the Special Processing Modules.
    //
    void _initSSPModule();

    //
    // Unloads the Special Processing Modules.
    //
    void _cleanupSSPModule();

    //
    // Loads the Special Processing shared library module.
    //
    DynamicLibrary _loadSSPModule(const String& moduleName);

    //
    //  Invokes the Qualifier Special Processing Modules.
    //
    //  @param  oldQualifier    input qualifier to be processed
    //
    //  @param  newQualifier    processed qualifier to be returned
    //
    //  @return true           if the qualifier must be created
    //          false          if the qualifier must be ignored
    //
    Boolean _invokeModules( CIMQualifierDecl& oldQualifier,
                            CIMQualifierDecl& newQualifir);

    //
    //  Invokes the Class Special Processing Modules.
    //
    //  @param  oldClass    input class to be processed
    //
    //  @param  newClass    processed class to be returned
    //
    //  @return true           if the class must be created
    //          false          if the class must be ignored
    //
    Boolean _invokeModules( CIMClass& oldClass,
                            CIMClass& newClass);

    //
    //  Invokes the Instance Special Processing Modules.
    //
    //  @param  oldInstance    input instance to be processed
    //
    //  @param  newInstance    processed instance to be returned
    //
    //  @return true           if the instance must be created
    //          false          if the instance must be ignored
    //
    Boolean _invokeModules( CIMInstance& oldInstance,
                            CIMInstance& newInstance);

    //
    //  Creates a Special Processing Module.
    //
    //  @param  library                      handle to the shared library
    //
    //  @return SchemaSpecialProcessModule   handle to the Special Processing
    //                                       library
    //
    SchemaSpecialProcessModule *  _createSSPModule(DynamicLibrary& library);
#endif

    //
    //  Compares two version strings.
    //
    //  @param  oldVersion     version number of the old class
    //
    //  @param  newVersion     version number of the new class
    //
    //  @return true           if the old class has a higher version number
    //
    //          false          if the old class has a lower or the same
    //                         version number
    //
    Boolean _compareVersion( const String& oldVersion,
                            const String& newVersion );

};

PEGASUS_NAMESPACE_END

#endif // Pegasus_RepositoryUpgrade_h
