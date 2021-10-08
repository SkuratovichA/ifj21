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

test_type="parser"
output_file="failed_parser.txt"

rm "failed_parser.txt"

. ./tests_lib.sh

echo "Starting parser tests..." >> "$output_file"

# Return code 0 - success
# Return code 1 - lexical analysis error
# Return code 2 - syntactic analysis error

declare -i num_name=0 # name of the file to be tested

echo
echo "Test 1 - invalid expressions"
test_one_file "./expressions_invalid.tl" 2 "$test_type"

echo ""
echo "Finished." >> "$output_file"