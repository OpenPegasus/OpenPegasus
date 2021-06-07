# PEP 368 update for OpenPegasus Version 2,14.3

## Overview of the changes
This is an incremental update to OpenPegasus 2.14.2 to fix significant issues
affecting use of OpenPegasus and some particular issues associated with SNIA
SMI-S

The primary changes were:

1. Update OpenPegasus to support include OpenSSL versions 1.1.0+. This was a
significant change because OpenSSL made major changes to the OpenSSL environment
with this version.

2. Update the certificates, etc. in the OpenPegasus test and development
environment to bring them up-to-date with the newer version of OpenSSL,
reactivate many of them that had expired subsequent to OpenPegasus 2.14.2 release,
and add scripts so that they can be updated in the future.

3. Limit the tested support for OpenPegasus to Windows and Linux platforms
because those are the only platforms which the development team has access to
any more.  While we tried to avoid changes involving the many platform specific
code, we did not test on other platforms such as HPUX, etc.

4. Add ability to compile experimental schema into OpenPegasus.

5. Add specifically the DMTF schema version 2.41.0 experimental to the set
of schemas defined for OpenPegasus specifically because that schema is used as
the basis for the SNIA SMI-S specification 2.61.  Note that the basic
OpenPegasus tests (ex. Make world, make unittest, make servertests, etc.) still
operates with the DMTF schema 2.41 and fails with 2.41 experimental because the
tests are very literal and even minor changes to the model impact them.

6. Update the code so that OpenPegasus compiles with the gcc compiler up to
version 9 whereas OpenPegasus 2.14.2 only compiled with gcc version 4 and lower.
This involved many minor changes to the code to fix issues where the newer
compilers are stricter than the old compilers both in generating warnings and
errors. In some cases, the issues were hidden by ignoring the error/warning rather
that fixing the code itself. In some cases the code was updated.

7. Moved the development environment and release of this version of OpenPegasus to
github under the protect name OpenPegasus because the OpenGroup is no longer making
updates to the OpenPegasus web pages.

These changes are documented more thoroughly in the file OpenPegasus/docs/changes.rst.

For the moment, the only distribution mechanism for the OpenPegasus source is the
git repository itself and cloning the repository.

## Reporting problems

Problems can be reported for version 2.14.3 using the github issues that are in the
OpenPegasus repository of the OpenPegaus project.
