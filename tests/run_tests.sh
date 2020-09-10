#!/bin/bash
root=../..
pushd cases/input > /dev/null
logdir=$root/../bin/tests
mkdir -p $logdir
tester=$root/$1
tmpdir=$(mktemp -d)
re="^[0-9]+$"
failedcnt=0
for file in * ; do
	if ! [[ $file =~ $re ]] ; then
		continue # Only match numeric filenames
	fi
	printf "Running test #%s... " "$file"
	cat $file | $tester > $tmpdir/$file
	diff $tmpdir/$file ../output/$file > $logdir/$file
	if [ $? -eq 0 ] ; then
		printf "\033[1;32mPASSED"
	else
		printf "\033[1;31mFAILED"
		((++failedcnt))
	fi
	printf "\033[0m\n"
done
popd > /dev/null
rm -rf $tmpdir
exit $failedcnt
