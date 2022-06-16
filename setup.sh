# Test of a fixed configure as bash shell. This example
# configures for Linux build with gcc compiler and using
# an X86-64 bit platform.

echo $PWD

echo "This is an example of a build setup script that"
echo "sets the required environment variables for"
echo "Linux x86-64 hardware and the gcc compiler"
echo "and a logical set of build options. It enables"
echo "more options than most impplementations require"
echo "because it is used as a basis for tests."
echo "Execute as source from OpenPegasus directory."

# Set Root to the current directory.
# Set up minimum required environment variables for
# platform/os/compiler, location of source (the pegasus directory).
# and the output target for the compiler and test environment PEGASUS_HOME)

# Assumes user is the OpenPegasus directory is the current directory
export ROOT=$PWD
export PEGASUS_ROOT=$ROOT/pegasus
export PEGASUS_HOME=$ROOT/home
export PEGASUS_PLATFORM=LINUX_X86_64_GNU

# Add pegasus bin directory to path and create home directory
# Path addition required to run the OpenPegasus server since that is where
# executables are placed.
export PATH=$PEGASUS_HOME/bin:$PATH
mkdir $PEGASUS_HOME -p
export LD_LIBRARY_PATH=$PEGASUS_HOME/lib64:

# Storage mode of the CIM repository created on initial build
# This may be modified using cimconfig.
# The default if this is not uses is XML
# the possible modes are BIN and XML
export PEGASUS_REPOSITORY_MODE=BIN

# Enable the Audit logger that logs all requests that
# modify instances or classes
export PEGASUS_ENABLE_AUDIT_LOGGER=true

# Enable use of the DMTF indication profile definition as
# basis for Profile implementation.
export PEGASUS_ENABLE_DMTF_INDICATION_PROFILE_SUPPORT=true

# Enable the indication statistics.
export PEGASUS_ENABLE_INDICATION_COUNT=true

# Use syslog for log output
export PEGASUS_ENABLE_SYSTEM_LOG_HANDLER=true

# Enable the Interop Provider.
export PEGASUS_ENABLE_INTEROP_PROVIDER=true

# Miscellaneous handlers and functions
export PEGASUS_ENABLE_EMAIL_HANDLER=true
export PEGASUS_ENABLE_PROTOCOL_WSMAN=true
export PEGASUS_ENABLE_USERGROUP_AUTHORIZATION=true
export PEGASUS_ENABLE_CQL=true
export PEGASUS_ENABLE_WQL=true
export PEGASUS_ENABLE_IPV6=true
export PEGASUS_DISABLE_PROV_USERCTXT=true

# Set debug mode. Compiles extra debug functionality
export PEGASUS_DEBUG=1

# SSL configuration options to enable ssl with OpenSSL
export PEGASUS_HAS_SSL=true
export PEGASUS_PAM_AUTHENTICATION=true
export PEGASUS_USE_PAM_STANDALONE_PROC=false

# Enable the CMPI provider manager so CMPI providers can be used
export PEGASUS_ENABLE_CMPI_PROVIDER_MANAGER=true
export PEGASUS_PLATFORM_FOR_32BIT_PROVIDER_SUPPORT=LINUX_X86_64_GNU

# show created environment
export | grep PEGASUS
