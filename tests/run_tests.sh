#!/bin/bash

# Run tests

# Usage: run_tests.sh <tester-application>
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

# logdir is the directory where logs are
# going to be saved (inside bin/, which
# is ignored by git)
logdir=$root/../bin/tests

# Make logdir if nonexistent
mkdir -p $logdir

# Tester application path
tester=$root/$1

# tmpdir is the directory where all the
# outputs will be written to so that
# they can be compared with the expected
# outputs
tmpdir=$(mktemp -d)

# re is the regular expression for the
# input files (only numbers)
re="^[0-9]+$"

# failedcnt is the number of tests that
# have failed
failedcnt=0

# colours useful for printing
LGREEN='\033[1;32m'
LRED='\033[1;31m'
NOCOLOR='\033[0m'

# For every file in cases/input ...
for file in * ; do

	# Check if its name matches the regex
	if ! [[ $file =~ $re ]] ; then
		continue 
	fi

	# Notify that test will be run...
	printf "[ TEST ] Running test #%s... " "$file"

	# Run test, saving the output in the temporary directory
	cat $file | $tester > $tmpdir/$file

	# Compare the expected and obtained outputs and
	# redirect the diff analysis to the log directory
	diff $tmpdir/$file ../output/$file > $logdir/$file

	# Check if diff returned 0 (i.e. the files are identical)
	# or else, the files are different
	if [ $? -eq 0 ] ; then
		printf $LGREEN"PASSED"
	else
		printf $LRED"FAILED"

		# Increase the count of failed tests
		((++failedcnt))
	fi
	printf $NOCOLOR"\n"
done

# Remove the temporary directory
rm -rf $tmpdir

# Go back to the root directory
popd > /dev/null

# Check if any tests failed and print an appropriate
# message to summarize the test results
if [ $failedcnt -eq 0 ] ; then
	printf "[ TEST ] "$LGREEN"All tests passed"
else
	printf "[ TEST ] "$LRED"$failedcnt tests failed"
fi
printf $NOCOLOR"\n"

# Exit the script with 0 if no tests failed (i.e. success)
# or else, if at least one failed (i.e. failed)
exit $failedcnt
