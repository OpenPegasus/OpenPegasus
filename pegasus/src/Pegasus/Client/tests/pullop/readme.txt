Pegasus test client to test pull operations.  Executes a single pull
sequence determined by the Type parameter consisting of an open
request and a number of pull requests.

Generally the usage is:

    Usage: pullop [OPERATION_TYPE] [OPTIONS]

    WHERE OPERATION_TYPE is one of the following pull operation sequences types:
		e    enumerateInstances,
		en   enumerate instance names,
		r    reference, rn - referenceNames,
		a    associators
		an   assocatornames
		all  test of all classes in one namespace


The program options determine:
   - The server name (default is to the local host)
   - Parameters of the pull operation including class name,
     interoperation timeout, max object count, etc.
   - If the results are to be compared with the corresponding non pull
     operation (ex. e (the openEnumerateInstances, pull, ...) would
     be compared with enumerateInstances on the same class.  This automatically
     tests for things like properties returned, use of classorigin, etc to
     determine if the the two commands are returning the same information.
     Note that there is a time difference between the two operations so that
     there are many classes in OpenPegasus that will return different information
     even in the short time between the pull operation and corresponding operation.
     Thus, for example, threads would change so that the class returnin this
     information would be different.
   - Level or verbosity of status and error information

The goal of this program is to provide a relatively complete test for
the operation of the DMTF pull operations (client and server) that can
be executed from the command line and Makefiles.

The Makefile defines a broad set of tests that use the pullopexecutable
as the execution tool.

Geneally the pullop command line options are as follows:
    -V              Print version.
    -v  LEVEL       Integer defining level of displays (0 - 6)
                    0 - none, 1 - Errors, 2, warnings, 3 - statistical info,
                    4 - operation overview, 5 - Operation details, 6 - Steps
                    7 - display returned objects
    -h              Print this help message.
    --help          Print this help message.
    -n NAMESPACE    The operation namespace(default is "root/cimv2").
    -c OBJECTNAME   Name of class or object instance to process.
    -H HOST         Connect to this HOST where format for host is 
                    hostName [:port]. Default is to use connectLocal.
                    If hostName specified witout port(default is 5988)
    -u USER         String. Connect as this USER.
    -p PASSWORD     String. Connect with this PASSWORD.
    -t TIMEOUT      Integer interoperation timeout. (Default 0)
    -s seconds      Time to sleep between operations. Used to test timeouts
    -T              Show results of detailed timing of the operations
    -x              ContinueOnError flag set to true.
    -o              Request ClassOrigin flag true where used.
                    Default classOrigin = false 
    -d              Set deepInheritance false where used (i.e enumerate
                    instances. Default: deepInheritance = true.
    -M MAXOBJECTS   Integer Max objects for open operation.
                    (Default 16).
    -N MAXOBJECTS   Integer Max objects per pull operation.
                    (Default 16).
    -P PROPERTY     Property to include in propertyFilter.  May be repeated
                    to create a multiproperty property list.
                    Use "" to represent an empty property list
    -C              Compare the result against old equivalent CIM operation.
    -f QUERY FILTER String value for queryFilter parameterFilter.
                    Note not supported in pegasus now. Default "".
    -l QUERYFILTERLANG String value for queryFilterLanguage parameter.
                    Note - This parameter not supported in Pegasus now.
                    Default "" which tells server no value.
    -r REPEAT       Integer count of number of times to repeat pull sequence.
                    This is support for stress testing. Repeats are
                    serial today, not concurrent.
    -R              Reverse Exit code. Useful to add error tests to
                    Makefiles where we can conduct tests that expect
                    exception returns. If return code is zero this option
                    sets it to nonzero and viceversa.
    -X MAXOPERATIONS Integer. Maximum operations in enumeration sequence
                     before close executed. Default is not used. If set
                     close will be executed after that number of operations.
                     Zero(0) Not allowed since has no meaning. One(1) means
                     execute only open operation. This is not the same
                     as the MAXOBJECTS parameters.
    -W               Boolean. Treat Errors in Compare as warnings.


To see the exact options for pullop enter "pullop -h" or "pullop --help".

pullop uses its own input parameters to control verbosity of information
and does not depend on the PEGASUS_CLIENT_TRACE environment variable.


There is another client program that executes a test of a number of
Pull errors (See PullErrors).






