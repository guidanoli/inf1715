#!/bin/bash

# Compares input and output between
# initial and final (numbers)

# Usage: compare_io.sh <initial> <final>
# Should be called from the location of
# the script

initial=$1
final=$2

pushd cases
for i in $(seq $initial $final); do
	if [[ -f "output/$i" ]] ; then
		vi -O input/$i output/$i
	elif [[ -f "../../bin/candidates/$i" ]] ; then
		vi -O input/$i ../../bin/candidates/$i
	else
		echo "Could not find output for test #$i..."
	fi
done
popd
