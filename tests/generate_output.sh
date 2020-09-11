#!/bin/bash

# Generate output for all the input files
# that don't have an associated output fil# Generate output for all the input files
# that don't have an associated output filee

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

# Candidates directory
candidatesdir=$root/../bin/candidates

mkdir -p $candidatesdir

# re is the regular expression for the
# input files (only numbers)
re="^[0-9]+$"

generatedcnt=0

# For every file in cases/input ...
for file in * ;
do

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
	cat $file | $tester > $candidatesdir/$file

	# Notify that the output has been generated...
	printf "Generated output for test #%s...\n" "$file"

	# Increment generated count
	((++generatedcnt))
done

if [[ $generatedcnt -ne 0 ]] ; then
	printf "$generatedcnt file(s) generated in %s\n" "$(realpath "$candidatesdir")"
fi

# Go back to the root directory
popd > /dev/null
