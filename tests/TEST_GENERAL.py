# IFJ21 testing script 
# Input files are in /test_input_valid & /test_input_invalid 
# Kuznik Jakub

# logs_and_input_files/input_files_for_each_server/
from datetime import date, timedelta
import time
import sys
import os
sys.path.insert(0, '.')

import glob
# Global variable that save on which lines are each json ends 
lines = [] 

import subprocess
from subprocess import call
from colorama import Fore, Style

test_num = 0


def test(input_file, expected_error_code):

    with open(input_file, 'r') as file:
        data = file.read().rstrip()


    result = subprocess.run(
        ["../cmake-build-debug/ifj21"], text=True, input=data
    ).returncode

    if result != expected_error_code:
        print(Fore.RED + "[" , test_num , "]" + "   Failed    [test:]" + input_file)
    else:
        print(Fore.GREEN + "[" , test_num , "]" + "   Passed    [test:]" + input_file)
    test = test_num+1
    



def main():
    #subprocess.run(args, *, stdin=None, input=None, stdout=None, stderr=None, capture_output=False, shell=False, cwd=None, timeout=None, check=False, encoding=None, errors=None, text=None, env=None, universal_newlines=None, **other_popen_kwargs)
    print(Fore.RESET)
    test('test_inputs_valid/0_vypocet_faktorialu_iterativne.tl', 1)
    test('test_inputs_invalid/1_lexical/1_tokens_invalid.tl', 0)
    print(Fore.RESET)

    return 0

if __name__ == '__main__':
    start = time.time()
    main()
    print("test time:", time.time() - start)
