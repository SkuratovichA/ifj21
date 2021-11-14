# IFJ21 testing script 
# Input files are in /test_input_valid & /test_input_invalid 
# Kuznik Jakub

from datetime import date, timedelta
import time
import sys
import os
sys.path.insert(0, '.')

import glob

import subprocess
from subprocess import call
from colorama import Fore, Style

test_num = 0


#return first two char from string 
def firstTwo(string):
    return string[:2]

# run all test from input folder 
# Expected error code should be same as input 
def test_error_codes(expected_error_code, input_folder, test_set):
    
    global test_num
    
    #open every file in folder 
    for filename in os.listdir(input_folder):
        with open(os.path.join(input_folder, filename), 'r') as f:
            #set file contain as input
            file_num = firstTwo(filename)
            data = f.read()
            
            test_num += 1
            
            #run ifj21 with data on stdin 
            result = subprocess.run(
                ["../cmake-build-debug/ifj21"], text=True, input=data
            ).returncode

            if result != expected_error_code:
                #[global num]   FAILED || PASED     EXPECTED  X   ACTUALL  X   TEST_SET   FILE_NUM
                print(Fore.RED + "[" ,test_num, "]" + "  Failed   " + " exp ret code: ",  expected_error_code , "Actuall:", result, test_set, "file_num",file_num)
            else:
                print(Fore.GREEN + "[" , test_num , "]" + "   Passed  "  + test_set)


def main():
    #subprocess.run(args, *, stdin=None, input=None, stdout=None, stderr=None, capture_output=False, shell=False, cwd=None, timeout=None, check=False, encoding=None, errors=None, text=None, env=None, universal_newlines=None, **other_popen_kwargs)
    print(Fore.RESET)

    #scanner test 
    test_error_codes(1, "test_inputs_invalid/1_lexical", "1_LEXICAL")
    #syntax test 
    test_error_codes(2, "test_inputs_invalid/2_syntax", "2_SYNTAX" )
    #definiton
    test_error_codes(3, "test_inputs_invalid/3_definition", "3_DEFINITION" )
    #type_missmatch
    test_error_codes(4, "test_inputs_invalid/4_type_missmatch", "4_TYPE_MISSMATCH" )
    #function_sematics
    test_error_codes(5, "test_inputs_invalid/5_function_sematics", "5_FUNCTION_SEMATICS" )
    #incompatable_sematics
    test_error_codes(6, "test_inputs_invalid/6_type_incompatable_sematic", "6_INCOMPATABLE_SEMATICS" )
    #sematic_other
    test_error_codes(7, "test_inputs_invalid/7_sematic_other", "7_SEMATIC_OTHER" )
    #runtime_nil
    test_error_codes(8, "test_inputs_invalid/8_runtime_nil", "8_RUNTIME_NIL" )
    #zero_divide
    test_error_codes(9, "test_inputs_invalid/9_zero_divide", "9_ZERO_DIVIDE" )
    
    #VALID PROGRAMS 
    test_error_codes(0, "test_inputs_valid", "0_VALID" )
    
    
    print(Fore.RESET)

    return 0

if __name__ == '__main__':
    start = time.time()
    main()
    print("test time:", time.time() - start)
