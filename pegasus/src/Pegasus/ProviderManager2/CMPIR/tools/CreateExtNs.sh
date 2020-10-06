if [ -z "$PEGASUS_HOME" ]; then
   echo Pegsus environment not set
   exit 5
fi
rdir=$PEGASUS_HOME/repository

while getopts "n:p:l:" flag; do
  if [ "$flag" = "?" ]; then
     error="?"
  elif [ "$flag" = "n" ]; then
    ns=$OPTARG
    nsd=${ns//\//#}
  elif [ "$flag" = "p" ]; then
     pns=$OPTARG
     pnsd=${pns//\//#}
  elif [ "$flag" = "l" ]; then
     loc=$OPTARG
     logd=${loc//:/#}
  fi
done

if [ -z "$ns" ]; then
   echo Required parameter -n missing
   error="?"
fi
if [ -n "$error" ]; then
   echo Usage: CreateExtNs -n namespace-name [-p parent-namespace ] [-l remote-location ]
   exit 5
fi
if [ ! -d "$PEGASUS_HOME/repository" ]; then
   echo $PEGASUS_HOME/repository directory not found
   exit 5
fi
if [ -e "$rdir/$nsd" ]; then
   echo Namespace $ns already exists
   exit 5
fi
if [ -n "$pns" ]; then
   if [ ! -e "$rdir/$pnsd" ]; then
      echo Parent namespace $pns does not exist
      exit 5
   fi
fi

mkdir $rdir/$nsd
mkdir $rdir/$nsd/classes
mkdir $rdir/$nsd/instances
mkdir $rdir/$nsd/qualifiers
if [ -n "$ns" ]; then
   mkdir $rdir/$nsd/SRS$pnsd
fi
if [ -n "$loc" ]; then
   mkdir $rdir/$nsd/r10$logd
fi
