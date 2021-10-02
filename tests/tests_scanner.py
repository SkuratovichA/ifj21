# ##########################################
# Project name: IFJ - projekt
# File: tests_scanner.py
# Date: 01. 10. 2021
# Last change: 01. 10. 2021
# Team number: 101
# Authors:  Aliaksandr Skuratovich
#           Evgeny Torbin
#           Lucie Svobodová
#           Jakub Kuzník
# ##########################################

##
#  A set of tests that test out the functionality of the scanner.
#
#  @package tests_scanner
#  @file tests_scanner.py
#  @brief A set of tests that test out the functionality of the scanner.
#
#  @author Aliaksandr Skuratovich
#  @author Evgeny Torbin
#  @author Lucie Svobodová
#  @author Jakub Kuzník
import importlib
import unittest
import cffi


# the code for the function load was copied from a youtube video
# "Alexander Steffen - Writing unit tests for C code in Python"
def load(filename):
    # load source code
    source = open(filename + '.c').read()
    includes = open(filename + '.h').read()

    # pass source code to CFFI
    ffibuilder = cffi.FFI()
    ffibuilder.cdef(includes)
    ffibuilder.set_source(filename + '_', source)
    ffibuilder.compile()

    # import and return resulting module
    module = importlib.import_module(filename + '_')
    return module.lib


class Scanner(unittest.TestCase):#
    def setUp(self):
        self.module = load('scanner')

    def test_tokens(self):
        self.assertEqual(self.module.scanner("Hello"), "Hello")
