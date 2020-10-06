if [ "$#" = 0 -o "$#" = 1 ]; then
   echo Usage: sh AppendNs.sh name-space [...] input-file
   exit 6
fi

ans=""
while [ ! "$#" = 1 ]; do
   ans="$ans,\"$1\""
   shift
done
ans=${ans//\//\\\/}

sed1="/[Nn]amespaces *= *{[ ,\"\/0-9A-Za-z]/s/};/"
sed2="};/g"

echo $sed1$ans$sed2 >/tmp/$$.AppendNs
sed -f /tmp/$$.AppendNs $1
rm /tmp/$$.AppendNs
