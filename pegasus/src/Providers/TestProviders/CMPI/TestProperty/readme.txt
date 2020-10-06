
OpenPegasus does not support CMPI Property providers. However, the getProperty()
and setProperty() CIM operations are supported indirectly by mapping these to
calls to getInstance() and modifyInstance(), with the propertyList filter for
each containing only the property specified. This CMPI test verifies that
mapping (see TestProperty/tests/TestCMPIProperty.cpp).

