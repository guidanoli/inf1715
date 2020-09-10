#!/bin/bash
root=../..
pushd cases/input > /dev/null
tester=$root/$1
re="^[0-9]+$"
for file in * ; do
	if ! [[ $file =~ $re ]] ; then
		continue # Only match numeric filenames
	fi
	if [[ -f ../output/$file ]] ; then
		continue # Only match inputs without output
	fi
	cat $file | $tester > ../output/$file
	printf "Generated output for test #%s...\n" "$file"
done
popd > /dev/null
