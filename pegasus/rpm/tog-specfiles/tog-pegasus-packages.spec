# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-packages.spec
#
%package devel
Summary: The OpenPegasus Software Development Kit
Group: Systems Management/Base
Requires: %{Flavor}-pegasus >= %{version}
Requires(preun): bash, procps, grep, coreutils, make
Obsoletes: %{Flavor}-pegasus-sdk

%description devel
The OpenPegasus WBEM Services for Linux SDK is the developer's kit for the
OpenPegasus WBEM Services for Linux release. It provides Linux C++ developers
with the WBEM files required to build WBEM Clients and Providers. It also
supports C provider developers via the CMPI interface.

%if %{PEGASUS_BUILD_TEST_RPM}
%package test
Summary: The OpenPegasus Tests
Group: Systems Management/Base
Requires: %{Flavor}-pegasus >= %{version}

%description test
The OpenPegasus WBEM tests for the OpenPegasus %{version} Linux rpm.
%endif
#
#End of section pegasus/rpm/tog-specfiles/tog-pegasus-packages.spec
