#!/bin/bash

# writes Passed/Failed to stdout
fn_test_status() {
	case $1 in
		$2)
			echo -n -e "\x1b[32mPassed\x1b[0m (lexical error)"
		;;
		*)
			echo -n -e "\x1b[31mFailed\x1b[0m"
			echo
		;;
	esac
}

# runs the program and checks the exit code
# fn_test_run test_expression return_code_expected
fn_test_run() {
      status_expected=$2
			status=$?

			../cmake-build-debug/ifj21 <$1 >/dev/null 2>/dev/null

			if [ $status -ne $status_expected ]; then
			  echo "" >> failed.txt
				echo -e "\nExit status: " "$status" ", expected " "$status_expected" " when testing the file " "$1" >> failed.txt
				cat < "$1" >> failed.txt
      else
        rm "$1"
			fi
			echo "Test number ""$num_name"":""$TEST"  `fn_test_status $status $status_expected`
}

# reads one file and tests every word
# test_one_file file_to_test return_code_expected
test_one_file() {
  # insert require header
  req="require \"ifj21\"\n"
  stat_expected=$2

  # Read line by line

  #declare -i num_name=0 # name of the file to be tested

  while read -ra line;
  do
      # read one word
      for word in "${line[@]}";
      do
          #echo "$word";
          # concat the word with required header
          expr="${req}$word"
          file_name="${num_name}.tl"
          echo -e "$expr" > "$file_name"
          # test the program with one expression
          #echo $file_name
          fn_test_run "$file_name" "$stat_expected"
          #fn_test_run "$expr" "$stat_expected"   # could be used in the case the compiler can be run: ./run < "expression"
          num_name=$(( num_name + 1 ))
      done;
  done < $1
}