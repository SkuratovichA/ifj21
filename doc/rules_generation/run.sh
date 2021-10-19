#! /bin/bash

#(***human: grammar rules(parser) in ascending order for latex(nonterminals are bold ) and everything else.
# *Rules startfrom 1.(first tolast),
# but the first rule in latex == the last rule in the program.
# *
# * Latex comment generated, too. But you can comment those lines .(they are below). It's a trash, I know, but it works.
#
# ***Usage:
# * in .c program comment write rule in EBNF form, e.g.
# * // some documentation. Im gonna get an oznuk.
# * // !rule <rule> -> derivations
# *
# **NOTE: !rule <rule> -> smth | <arstoien> will become
# *  <rule> -> smth
# *  <rule> -> <arstoien>
# *
# *
# *
# * P.S: yea, I know, the code is the peace of shit, but only god can judge me.(and disciplinarni komise)
# * *)
#

# commandline argument: first the path to parser.c fil.e
ocamlc str.cma rules_generator.ml && ./a.out $1|| echo "gl, you need to install ocaml first"
#                                            ^ this.

