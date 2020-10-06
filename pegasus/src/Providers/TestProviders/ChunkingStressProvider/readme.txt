
     The ChunkingStressProvider test provider is intended to exercise the
HTTP 1.1 Transfer Encoding support, or "chunked response" support, from
PEP 140. This test provider intends to test this support in a stress
environment to verify functionality and at the same time look for
serialization exposures, etc.

     The following CIM operations can potentially return responses in
"chunks" at a time, instead of all at once:

- enumerateInstances (abbreviated EI)
- enumerateInstanceNames (NI)
- associators (A)
- associatorNames (AN)
- references (R)
- referenceNames (RN)

There is a size vs speed tradeoff, so currently Pegasus will returned a
chunked response once it has accumulated 100 objects from a specific
provider (configurable via the PEGASUS_RESPONSE_OBJECT_COUNT_THREASHOLD
compile-time switch -- see PEP 140 for details).

The TestChunkingStressProviderModule is actually composed of three
separate providers:

- TestChunkingStressProviderEI, an Instance Provider for the base class
  TST_ChunkingStressInstance. This provider is currently hardcoded to
  return 3003 instances on an enumerateInstances call, and 3003 instance
  names on an enumerateInstanceNames call. No other instance methods are
  supported.

- TestChunkingStressProviderEIS, an Instance Provider for the derived class
  TST_ChunkingStressInstanceS. This provider is currently hardcoded to
  return 1001 instances on an enumerateInstances call, and 1001 instance
  names on an enumerateInstanceNames call. No other instance methods are
  supported.

- TestChunkingStressProviderASSOC, an Association Provider for the class
  TST_ChunkingStressAssoc. This provider implements the four associator-
  related operations, although the objects or objectnames that are
  returned are meaningless. This test provider is more about "pushing
  data" through the pipe and testing the chunk support. The association
  provider operations get their data from an enumerateInstances or
  enumerateInstanceNames call through the CIMOMHandle for the
  TST_ChunkingStressInstance class. Returning chunked responses through
  the CIMOMHandle tests different paths through the code.

The client to drive this provider is in
testclient/TestChunkingStressProviderClient.cpp. Here's the usage info:

Usage:
    TestChunkingStressProviderClient xxxxxx abcdef
    where:
       xxxxxx - is the time in seconds that the test will run
       abcdef - is a 6 digit number where each digit represents
                the number of threads to run for each chunking
                CIM operation:
                  a - enumerateInstances
                  b - enumerateInstanceNames
                  c - associators
                  d - associatorNames
                  e - references
                  f - referenceNames
       For example:
          TestChunkingStressProviderClient 60 210777
       will run a total of 24 threads for about 60 seconds:
          2 threads will loop making enumerateInstances calls
          1 thread  will loop making enumerateInstanceNames calls
          7 threads will loop making associatorNames calls
          7 threads will loop making references calls
          7 threads will loop making referenceNames calls
          No threads will make any associator calls

