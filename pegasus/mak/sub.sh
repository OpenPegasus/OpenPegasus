for i in *.h *.cpp
do
    if [ -f $i ]
    then
	echo $i
	cp $i $i.bak
	sed -f /tmp/sub.sed $i.bak > $i
	rm -f $i.bak
    fi
done
