#!/bin/bash
# Format of test app names: test_<structure>_<tested routines>

echo "Building tests...."
cd ../cmake-build-debug || exit
make
cd ../tests || exit

status=$? # exit status of the last command
if [ "$status" != "0" ]
then
	echo "Build error status code: $status";
	exit $status
fi

echo
echo

test_type="scanner"
output_file="failed_scanner.txt"

rm "$output_file"

. ./tests_lib.sh

echo "Starting scanner tests..." >> "$output_file"

# Return code 0 - success
# Return code 1 - lexical analysis error
# Return code 2 - syntactic analysis error

declare -i num_name=0 # name of the file to be tested

echo
#echo "Test 1 - float numbers, good"
#test_one_file "./number_ok.tl" 0 "$test_type"

#echo "Test 2 - integer numbers, good"
#test_one_file "./integer_ok.tl" 0 "$test_type"

#echo "Test 3 - identifiers, good"
#test_one_file "./identif_ok.tl" 0 "$test_type"

#echo "Test 4 - strings, good"
#test_one_file "./string_ok.tl" 0 "$test_type"

echo "Test 5 - invalid tokens"
test_one_file "./tokens_invalid.tl" 1 "$test_type"

echo ""
echo "Finished." >> "$output_file"