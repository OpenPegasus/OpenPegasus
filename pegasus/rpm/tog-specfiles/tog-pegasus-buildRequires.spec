# Start of section pegasus/rpm/tog-specfiles/tog-pegasus-buildRequires.spec
#
BuildRequires:      bash, sed, grep, coreutils, procps, gcc, gcc-c++
BuildRequires:      libstdc++, make, pam-devel
BuildRequires:      openssl-devel >= 0.9.6, e2fsprogs

#Following is commented because, Currently could not find clang shipped
#Should be changed or uncommented when distros ship clang
#and expects that system has clang 3 and above installed by other means
#%if %{PEGASUS_BUILD_WITH_CLANG}
#BuildRequires:      clang
#%endif

%if %{JMPI_PROVIDER_REQUESTED}
BuildRequires:      gcc-java, libgcj-devel, libgcj, java-1.4.2-gcj-compat
Requires:           libgcj, java-1.4.2-gcj-compat
%endif
%if %{EXTERNAL_SLP_REQUESTED}
BuildRequires:      openslp, openslp-devel
Requires:           openslp
%endif

