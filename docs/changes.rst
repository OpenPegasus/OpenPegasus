

.. _`Change log`:

Change log
==========

.. ifconfig:: '.dev' in version

   This version of the documentation is development version |version| and
   contains the `master` branch up to this commit:

   .. git_changelog::
      :revisions: 1


openpegasus 14.2.dev1
---------------------


openpegasus 14.1 - Release
branch: release_14_1

Bugs

This stable branch represents the 14.1 release of OpenPegasus taken from the
OpenPegasus web site gzip file.  It includes only extensions to add a
.gitignore and simple README.md.

Users may use this branch or the gzip/zip files available from the OpenPegasus
website.

1. Fix issue with gcc compile warnings for class_memaccess and deprecated-copy
   and deprecated. These were fixes for now by hiding the tests in the compiler
   call. Fix warnings about implicit-fallthrough in case statements
   where the by adding /* FALLTHROUGH */ which is a comment that the compiler
   versions that detect the error recognize as an indication that fallback
   is desired. (Issue #1)

2. Fix issue with literal use of root/PG_Interop in Makefile for
   Client/test/pullop/pullop.cpp. (See issue #2)

3. Fix code from warning messages for vsprintf, etc. where the buffer could
   be too small. Enlarged output buffer. (See issue #5)

4. Correct numerous compiler check-value warnings where they were all caused
   by using a value as the parameter of an exception statement.  Changed them
   to references.  See Issue #6)

5. Hide gcc compiler warning for function readdir_r as deprecated. This was
   corrected by setting the Wno-deprecated flag in configure_linux (see #7)

6. Fix issue with function valid() in Pegasus/Common/Message.cpp that was causing
   issue in non-debug mode.   (see issue #12)

cleanup

1. Update version to 2.14.2 and status development.

2. Change executable names on several test executables. (See issue # 27)
