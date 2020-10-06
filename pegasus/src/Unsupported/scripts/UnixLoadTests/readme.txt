     The OpenPegasus Unix load test scripts

11 December 2014
Updated 10 feb 2015

The 3 bash scripts in this directory are the basis for common load testing
of the CIM Server starting with Pegasu 2.14.  Since part of the release of
each new version of OpenPegasus is a multiday load test (The CHO test) these
scripts will form the basis for that test in the future.

Each script contains detailed help information on function and input
arguments that can be viewed by using the -h or --help option on the
script command line.

The scripts are:

1. watchcimserver - generates a regular server information output from the
ps command while the server is running. Output goes to both console and 
a file.

The watchcimserver runs and outputs snapshots of overall status of the
server until:
   a. Server stopped.
   b. Time defined by -t option runs out
   c. Stopped by Ctrl-C

Generally the display is as follows:
  PID   RSS    VSZ %MEM %CPU NLWP     TIME STIME     ELAPSED    SZ COMMAND
 7830 14124 396724  0.0  0.5    6 00:00:00 10:18       00:36 99181 cimserver
 7830 14124 396724  0.0  0.0    6 00:00:00 10:18       04:36 99181 cimserver

which are output parameters from the ps command.

To get more detailed information on the input arguments for watchcimserver
use the help option:
   watchcimserver --help

2. choparalleltest - generates a number of concurrent clients that execute
   operations against the running server and as an option, a listener
   that subscribes to indications.  Each client operation is operated as
   a background task that repeats until the completion time is reached,
   ctrl-C executed to stop the clients, to the cimserver stopped.

   NOTE: A third script "chosimpletest" is generally the same as choparallel
   script except that it waits for all clients to complete before repeating
   the loop and therefor is much less load intensive than choparalleltest.
   NOTE: This script will not be maintained in the future and should be
   considered obsolete.

The requests continue each in its own background process to create a number of
parallel requests to the server.

The client script is stopped by:
   a. Stopping the cimserver.
   b. Reaching the time limit defined by the command line options on the
      script
   c. The operator entering ctrl-C which kills the parent and child processes

The difference between the two scripts is that the simple scripts executes
all of the background tasks until they complete and then restarts the loop
so that since they finish unevenly, some end their background tasks faster
than others.

The parallel script sets each background client to operate in a continuous
loop until loop and runs these loops repeatedly with a very small sleep 
at the end of each loop.

Typical usage might be:

1. Start the cimserver
2. open a terminal window and start the watchcimserver script. For example:

    watchcimserver -i 240 -t 3d -t 10m

This will generate output both to the console and to the file watchcimserver.txt
every 240 seconds for 3 days and 10 minutes and then will shutdown the watch.

or (each in its own terminal window):
   watchcimserver -i 240 -t 8h -t 10m
   choparalleltest -l -t 8h

   We included the extra 10m on watchcimserver because the -t option on
   choparalleltest defines the time after which it will NOT start a new
   loop for any client.  The clients may run several minutes longer than
   the time defined by -t

3. Start the choparallel script to start generating requests to the server.
Generally the command line on each is the same with one difference,
the parallel script outputs results to either the console or the console
and a log file for each client based on the -l option

For example:
    choparalleltest -t 3d -l

Would cause the script to initiate the clients defined in the script and
let them run for 3 days. At that point it would shut all the clients down.

or (each in its own terminal window):
   watchcimserver -i 60 -t 8h -t 10m -s
   choparalleltest -l -t 8h

To start the server, run a test for 8 hours and display statistics each minute.

STATUS:

These scripts are considered unsupported but since they provide a common basis
for load testing the CIMServer, we expect to continue to grow them and add
functionality.

Expect them to change at any time without warning since they are in the
unsupported directory.

