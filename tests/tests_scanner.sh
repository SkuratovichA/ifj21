#!/bin/bash
# Format of test app names: test_<structure>_<tested routines>

echo "Building tests...."
cd ../cmake-build-debug
make
cd ../tests

status=$? # exit status of the last command
if [ "$status" != "0" ]
then
	echo "Build error status code: $status";
	exit $status
fi

echo
echo

rm failed.txt

. ./tests_scanner_lib.sh

echo "Starting..." >> failed.txt

declare -i num_name=0 # name of the file to be tested

echo
echo "Test 1 - float numbers, good"
test_one_file "./number_ok.tl" 0

echo "Test 2 - float numbers, bad"
test_one_file "./number_bad.tl" 1

echo "Test 3 - integer numbers, good"
test_one_file "./integer_ok.tl" 0

echo "Test 4 - integer numbers, bad"
test_one_file "./integer_bad.tl" 1

echo "Test 5 - identifiers, good"
test_one_file "./identif_ok.tl" 0

echo "Test 6 - identifiers, bad"
test_one_file "./identif_bad.tl" 1

echo "Test 7 - strings, good"
test_one_file "./string_ok.tl" 0

echo "Test 8 - strings, bad"
test_one_file "./string_bad.tl" 1

echo ""
echo "Finished." >> failed.txt