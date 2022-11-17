# OpenPegasus Change log

##Change log

## openpegasus 2.14.4

Status: **Development**

### Enhancements

### Bugs

### Enhancements

* Add sanitytests target to Makefile.  This initializes the repository and
  executes a short set of server tests to determine if the server is runnable.
  Note that it does not restore the repository or certificates to the state
  in which they might have existed before the sanity test was executed.
  (see issue #)

### Cleanup

## openpegasus 2.14.3

Status: **Released**

This release was based on the main branch, since there is no 2.14.0 branch
to serve as the base (We started this repository from OpenPegasus 2.14.1)

Released: 16 May 2022

### Bugs

* Add code to set the system name into Indication subscription instances
  when getInstance and EnumerateInstance called in IndicationServices.
  This property was left empty before meaning that the value of the
  paths was not equal to the value of the corresponding property
  components because the missing key property.   Also reorganized
  some code to simplify logic. (See issue #82)


### Enhancements

* Add test capability to test delays from the server with a provider.  This
  enhances the CLITestProvider to add a method delayedMethodResponse that
  includes a parameter delayInSeconds.  When this method is called, it delays
  the response by the value of delayInSeconds (see issue #84)

### Cleanup

* Add use of github actions to test capability so CI tests are run for each
  push.  The tests are currently limited because each execution of the test
  suite currently takes about an hour.  The CI test is on Linux with current
  gcc compiler

* Update a number of tests  that apparently were not executed because we were
  not executing tests in the the user non-privileged mode.  This includes tests in
  Clients/wbemexec and TestProviders/CLIProviderTests. This was primarily
  changes to the regression compare files and not changes to the test code
  or OpenPegasus code.

* Removed a number of compile warning messages by modifying the code to
  eliminate the warnings.  NOTE: There are still warnings in the
  compile but they are in test providers. (see issue #22)

* Added a new OpenPegasus/docs directory where new documents are being placed
  so that the existing OpenPegasus/pegasus/doc directory is not modified. The
  new documents define: changes for each version, tutorials on building and
  running OpenPegasus, and a recreation of the BuildAndReleaseOptions.html
  document into markdown so that it is readable from the README.md file. The
  new documents are all in the github markdown format.

## openpegasus 2.14.2 - Release

Status: Released 21 June 2021

NOTE: No branch was created for this release just the tag v.2.14.2.  Therefore
to use the code that this branch represents, the user must access it with the tag
v2.14.2.

The release is available by checkout of the tag or by getting either the
zip or tar file that are created by github as part of the release process and
located on the release page for version 2.14.2 (See the top page of the
OpenPegasus github repository)

### Bugs

1. Fix issue with gcc compile warnings for class_memaccess and deprecated-copy
   and deprecated. These were fixes for now by hiding the tests in the compiler
   call. Fix warnings about implicit-fallthrough in case statements
   where the by adding /* FALLTHROUGH */ which is a comment that the compiler
   versions that detect the error recognize as an indication that fallback
   is desired. (Issue #1)

2. Fix issue with literal use of root/PG_Interop in Makefile for
   Client/test/pullop/pullop.cpp. (See issue #2)

3. Issue with pulltest and namespaces fixed (Issue #3)

3. Fix code from warning messages for vsprintf, etc. where the buffer could
   be too small. Enlarged output buffer. (See issue #5)

4. Correct numerous compiler check-value warnings where they were all caused
   by using a value as the parameter of an exception statement.  Changed them
   to references.  See Issue #6)

5. Hide gcc compiler warning for function readdir_r as deprecated. This was
   corrected by setting the Wno-deprecated flag in configure_linux (see issue #7)

6. Fix issue with function valid() in Pegasus/Common/Message.cpp that was causing
   issue in non-debug mode.  (see issue #12 and #19)

7. MOF compiler fails with mof where EmbeddedInstance is in a non-leaf class (see
   issue #50)

8. The certificate and key files in pegasus/src/Server were built a number of years
   ago and were obsolete, expired and with SHA1 rather than SHA256 for digest
   definition. They would not pass tests.  These certificates and keys were rebuilt
   to update the dates and the characteristics to be usable with OpenSSL 1.1.1. See
   issue #63)

9. Upgrade OpenPegasus SSL functions to interface with OpenSSL 1.1.x.  This version
   of OpenSSL is a significatn API change from the 1.0.2 version. (see issue #17)

10. Fix issue in Message.cpp causing failure in not-debug mode. (see issue #19)

11. Fixed a number of minor spelling bugs in the help, specifically  the
   configuration help.


### Enhancements

1. Integrate DMTF schema 2.41 experimental into OpenPegasus schemas.  This required
   extending the schema definition capability to allow for DMTF experimental schemas and
   modifying the list of mof files in the Schemas directory for CIMExperimental241
   (See issue #46, #62). Note that the test suite cannot be run with this schema because
   the test suite depends on particular class definitions. Isee issue #52 and #62

2. Allow defintion of Experimental DMTF schema as OpenPegasus repositories (see issue #51)

3. Expand SSL support to include OpenSSL versions 1.1.0+ including updated
   OpenSSL APIs. (See issue #17)


### Cleanup

1. Update version to 2.14.2.

2. Change executable names on several test executables. (See issue #23 and #27)

3. Change variable names in SSLContext to keep internal variable names consistent with
   the cimconfig names for the variables.

4. Implemented shell scripts to be able to create specific security keys, and certificates
   for tests.  Previously the complete keys and certificates were presented but when
   the required changes because they were out-of-date or had invalid parameters for
   newer versions of OpenSSL, there was no way to recreate them.  Note that these
   scripts are bash based so that the scripts must be rebuilt on Linux systems or other
   *nix systems. This should not be an issue since all of the before dates were set
   about 10 years in the future beyond 2021 so that requiring rebuild should be
   straight forward.


## openpegasus 2.14.1 - Release

branch: release_14_1

This version the of OpenPegasus is maintained in the OpenGroup CVS repository and the
changes are documented in the OpenPegasus WEB site.  It is also in this repository
as the starting  point for subsequent releases in this github repository.

We chose not to attempt to update the complete CVS history of OpenPegasus but to
start the github repository from the last public release tar file.
