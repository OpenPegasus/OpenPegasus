# OpenPegasus

This repository is the OpenPegasus source code starting with OpenPegasus 14.1.


NOTE: This repository is a development repository to enable p
OpenPegasus from cvs to github in a manner that can be supported with
future versions of OpenPegasus and to support creating new versions of
OpenPegasus.  Before release, OpenPegasus may be moved to a separate github
project/repository.

The intial release of OpenPegasus in github is expected to be OpenPegasus
version 14.2

Version 14.2 Development status and goals

GOALS (This may change)

1. Update pegasus to remove compile warnings/errors resulting from use of
   new compilers (i.e. gcc move from 4.5 to 9.0)

2. Test with at least 1. Linux 0 gcc compiler, 2. Windows microsoft compiler
   since these are the default compilers for OpenPegasus

3. Provide an up-to-date OpenPegasus on github publicly available

4. Provide CI testing of OpenPegasus on github

6. Extend the github OpenPegasus to make user of github facilities such as
   better documentation tools, etc.

7. Extend OpenPegasus to support the OpenSSL 1.1 API which differs significantly
   from the 1.0.2 API that OpenPegasus currently supports.

Status Today:

1. Compile reduced from several hundered warnings to a very few on gcc
   9.x linux
