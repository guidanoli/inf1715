#!/bin/bash

# Usage: generate_output.sh <tester-application>
#
# tester-application - path of the tester
# 	application, relative to the
# 	directory where this script is
# 	located
#
# Should be run in the same directory as
# the script

# Go to input folder so that file names
# are merely numbers
pushd cases/input > /dev/null

# root is the directory where this file
# is located (tests/)
root=../..

# Tester application path
tester=$root/$1

# re is the regular expression for the
# input files (only numbers)
re="^[0-9]+$"

# For every file in cases/input ...
for file in * ; do

	# Check if its name matches the regex
	if ! [[ $file =~ $re ]] ; then
		continue
	fi

	# Check if the output was not generated yet
	# for the given input ($file)
	if [[ -f ../output/$file ]] ; then
		continue
	fi

	# Run test, saving the output
	cat $file | $tester > ../output/$file

	# Notify that the output has been generated...
	printf "Generated output for test #%s...\n" "$file"
done

# Go back to the root directory
popd > /dev/null
