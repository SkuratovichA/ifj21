#!/bin/bash


## Program that compares our ifj21 output with teal output 

#sed '/require "ifj21"/d'


folder="without_errors"

RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'


temp_file=".tmp"
ifj21_code=".ifj21_code"
teal_output=".tmp_1"
ifj_output=".tmp_2"

all_filer=0

teal_kostra=`cat ifj21.tl`

touch .tmp
touch .tmp_1
touch .tmp_2
touch .ijf21_code

rm ERRORS
touch ERRORS


echo "Hello if program stop pres ctrl-c."

for file in ${folder}/*.tl;
do
	all_files=$((all_files+1))

	## Create teal program 
	var=$(echo "$teal_kostra")
	var1=$(sed '/require "ifj21"/d' $file)
	{ echo "$var"; echo "$var1"; } > "$temp_file"
	##
	
	
 	timeout 5 ../cmake-build-debug/ifj21 < $(echo "$file") 1> $(echo "$ifj21_code") 2>/dev/null
	exit_status=$?
	if [[ $exit_status -eq 124 ]]; then
		printf "$all_files: ${RED}FAILED${NC} $file TIMEOUT\n"
		continue
	fi
	
	timeout 5 tl run $(echo "$temp_file")  1>$(echo "$teal_output")
	exit_status=$?
	if [[ $exit_status -eq 124 ]]; then
		printf "$all_files: ${RED}FAILED${NC} $file TIMEOUT\n"
		continue
	fi
	timeout 5 ./ic21int .ifj21_code 1>$(echo "$ifj_output")  2>/dev/null
	if [[ $exit_status -eq 124 ]]; then
		printf "$all_files: ${RED}FAILED${NC} $file TIMEOUT\n"
		continue
	fi
	
	



#	if [[ $(wc  <".tmp_1") -ge 2500 ]];then
#		printf "$all_files: ${RED}FAILED${NC} $file ENDLESS LOOP\n"
#	fi
#	if [[ $(wc  <".tmp_2") -ge 2500 ]];then
#		printf "$all_files: ${RED}FAILED${NC} $file ENDLESS LOOP\n"
#	fi

	
	ret_val=$(cat $(echo "$ifj_output"))
	expected_err=$(cat $(echo "$teal_output"))

	if [ "a$ret_val" != "a$expected_err" ];  then
		err_files=$((err_files+1))
		printf "$all_files: ${RED}FAILED${NC} $file\n"
		printf "$all_files: ${RED}FAILED${NC} $file your = $ret_val, expected $expected_err\n" >> ERRORS
	else
		printf "$all_files: ${GREEN}SUCCESFUL${NC} $file\n"
	fi
done
echo "CHECK YOUR ERRORS IN ERRORS"


