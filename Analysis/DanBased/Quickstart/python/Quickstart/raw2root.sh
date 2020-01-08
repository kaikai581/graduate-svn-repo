#!/bin/sh
storepath=/u/sklin/eliza7/RootFile/DanModified
command="nuwa.py -A None -n -1 -m"MyRawEventTree" "
findpath=/u/sklin/DryRunData/2010/TestDAQ/NoTag

while read line
do
	for fn in `find $findpath -name "*000$line*"|grep data`
	do
		outfn=${fn#*_}
		outfn=${outfn%.*}
		outfn=${line}_${outfn}
		${command} ${fn}
	        for dmy in `ls|grep eventTree`
		do
        	        if [ "$dmy" = "eventTree.root" ]; then
                	        finalfn=${outfn}.root
	                else
        	                finalfn=${outfn}_${dmy#*_}
	                fi
        	        mv -v $dmy ${storepath}/${finalfn}
	        done
	done
done < "runlist"
