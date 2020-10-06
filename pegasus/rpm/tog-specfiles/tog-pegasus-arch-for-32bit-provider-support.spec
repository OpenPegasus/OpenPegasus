# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-arch-for-32bit-provider-support.spec
# This is required only when PEGASUS_32BIT_PROVIDER_SUPPORT is set

%if %{PEGASUS_32BIT_PROVIDER_SUPPORT}

%ifarch x86_64

%if %{PEGASUS_BUILD_WITH_CLANG}
%global PEGASUS_HARDWARE_PLATFORM_FOR_32BIT LINUX_IX86_CLANG
%global PEGASUS_EXTRA_CXX_FLAGS_32BIT  "-O2 -g -pipe -fexceptions -fstack-protector -march=i386 -mtune=generic -fasynchronous-unwind-tables -m32"
% global PEGASUS_EXTRA_LINK_FLAGS_32BIT "-O2 -g -pipe -fexceptions -fstack-protector -march=i386 -mtune=generic -fasynchronous-unwind-tables -m32"
%else
%global PEGASUS_HARDWARE_PLATFORM_FOR_32BIT LINUX_IX86_GNU
%global PEGASUS_EXTRA_CXX_FLAGS_32BIT  "-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -march=i386 -mtune=generic -fasynchronous-unwind-tables -Wno-unused -m32"
%global PEGASUS_EXTRA_LINK_FLAGS_32BIT "-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -march=i386 -mtune=generic -fasynchronous-unwind-tables -m32"
%endif
%else
%ifarch ppc64 pseries
%global PEGASUS_HARDWARE_PLATFORM_FOR_32BIT LINUX_PPC_GNU
%global PEGASUS_EXTRA_CXX_FLAGS_32BIT  "-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2 -m32 -Wno-unused"
%global PEGASUS_EXTRA_LINK_FLAGS_32BIT "-O2 -g -m64 -fmessage-length=0 -D_FORTIFY_SOURCE=2 -m32"
%else
%ifarch s390x zseries
%global PEGASUS_HARDWARE_PLATFORM_FOR_32BIT LINUX_ZSERIES_GNU
%global PEGASUS_EXTRA_CXX_FLAGS_32BIT  "-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m31 -Wno-unused"
%global PEGASUS_EXTRA_LINK_FLAGS_32BIT "-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m31"
%endif
%endif
%endif

%endif

#
# End of section pegasus/rpm/tog-specfiles/tog-pegasus-arch-for-32bit-provider-support.spec
