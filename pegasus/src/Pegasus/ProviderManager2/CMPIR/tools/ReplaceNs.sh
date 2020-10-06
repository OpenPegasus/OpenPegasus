if [ "$#" = 0 -o "$#" = 1 ]; then
   echo Usage: sh ReplaceNs.sh name-space [...] input-file
   exit 6
fi

ans=""
more=""
while [ ! "$#" = 1 ]; do
   ans=$more$ans\"$1\"
   more=","
   shift
done
ans=${ans//\//\\\/}

sed1="/[Nn]amespaces *= *{/s/[ ,\"\/0-9A-Za-z]*} *;/"
sed2="};/g"

echo $sed1$ans$sed2 >/tmp/$$.ReplaceNs
sed -f /tmp/$$.ReplaceNs $1
rm /tmp/$$.ReplaceNs