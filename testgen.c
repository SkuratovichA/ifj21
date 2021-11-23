
#include <stdio.h>

#include "errors.h"
#include "parser.h"
#include "progfile.h"
#include "list.h"


#define NL "\n"
#define PROLOG "require \"ifj21\" \n"

#define TEST_CASE(number) \
do {                                                            \
    char *_filname;                                              \
    switch (retcode##number) {                                  \
        case_5 (ERROR_DEFINITION,                               \
                ERROR_TYPE_MISSMATCH,                           \
                ERROR_FUNCTION_SEMANTICS,                       \
                ERROR_SEMANTICS_TYPE_INCOMPATABLE,              \
                ERROR_SEMANTICS_OTHER                           \
                ):                                              \
            _filname = "../tests/semantic_errors/sasha" #number "_";       \
            break;                                                        \
        case ERROR_NOERROR:                                               \
            _filname = "../tests/without_errors/sasha" #number "_";        \
            break;                                                        \
        case ERROR_SYNTAX:                                                \
            _filname = "../tests/syntax_errors/sasha" #number "_";         \
            break;                                                        \
        default:                                                          \
            debug_msg_s("Undefined error code: %d\n", retcode##number);    \
            _filname = "sasha";                                            \
    }                                                                     \
    dynstring_t *filnam = Dynstring.ctor(_filname);                         \
    Dynstring.append(filnam, retcode ## number + '0');                     \
    Dynstring.cat(filnam, Dynstring.ctor(".tl"));                          \
    FILE *fil = fopen(Dynstring.c_str(filnam), "w");                        \
    assert(fil);                                                           \
    fprintf(fil, "-- test case %d.\n"                                      \
                "-- Description : %s\n\n"                                 \
                "-- Expected : '%d'\n",                                   \
                    number, description ## number, retcode ## number);    \
        fprintf(fil, "%s\n", Pfile.get_tape(pf ## number));                 \
        fclose(fil);                                                       \
    Pfile.dtor(pf ## number);                                              \
    debug_msg_s("file created: %s %s%c.tl\n",Dynstring.c_str(filnam), _filname, retcode##number + '0'); \
    Dynstring.dtor(filnam);             \
} while (0)


int main() {
    char *description1 = "prolog string";
    int retcode1 = ERROR_NOERROR;
    pfile_t *pf1 = Pfile.ctor(PROLOG);

    char *description2 = "lexical error";
    int retcode2 = ERROR_LEXICAL;
    pfile_t *pf2 = Pfile.ctor("1234.er" PROLOG);

    char *description3 = "Redeclaration error";
    int retcode3 = ERROR_DEFINITION;
    pfile_t *pf3 = Pfile.ctor(
            PROLOG
            "global foo : function()                                                                          "NL
            "global baz : function(string)                                                                    "NL
            "global bar : function(string, integer)                                                           "NL
            "global arst : function(string, integer, number, number, integer, string)                         "NL
            "global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string  "NL
            "global foo:function()       "NL
            " global baz:function(string)"NL
            " global bar:function(string, integer)"NL
            " global arst:function(string, integer, number, number, integer, string)"NL
            " global foo : function() : string\n"NL
            " global baz : function(string) : integer\n"NL
            " global bar : function(string, integer) : number\n"NL
            " global arst : function(string, integer, number) : number\n"NL
            " global foo : function() : string\n"NL
            " global baz : function(number) : integer, integer, integer, integer\n"NL
            " global bar : function(string, integer, number) : number\n"NL
            " global foo : function():string\n"NL
            " global baz : function(number):integer, integer, integer, integer\n"NL
            " global bar : function(string, integer, number):number\n"NL
            " global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"NL
            " global bar : function(string,integer,number):number, integer\n"NL
            " global bar : function(string,integer,number):number, number\n"NL
            " global bar : function(string,integer,number):number, string\n"NL
            " global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"NL
            " global foo : function()\n"NL
            " global foo : function()\n"NL
            " global foo : function()\n"NL
    );

    char *description4 = "no error, parsing definitions, declarations";
    int retcode4 = ERROR_NOERROR;
    pfile_t *pf4 = Pfile.ctor(
            PROLOG
            "global foo : function()"NL
            "function foo()         "NL
            "end                    "NL

            "global baz : function(string)"NL
            "function baz(str : string)   "NL
            "end                          "NL

            "global bar : function(string, integer)     "NL
            "function bar(str : string, int : integer)  "NL
            "end                                        "NL

            "global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n  "NL
            "function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) : string, string, string\n    "NL
            "end                                                                                                "NL

            " global arst : function(string,         integer,             number,       number,     integer, string)             "NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string) "NL
            "end                                                                                                                 "NL
            "foo() bar(\"hello\", 10) baz(\"hello\") arst(\"\", 10, 10.0, 10.10, 10, \"\") "NL
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa(\"arstioen\")                    "NL
    );

    char *description5 = "mutual recursion";
    int retcode5 = ERROR_NOERROR;
    pfile_t *pf5 = Pfile.ctor(
            PROLOG
            "global  foo : function (integer) : integer"NL
            "                                          "NL
            "function bar(param : integer) : integer   "NL
            "    if param <= 0 then                    "NL
            "        write(\"hello from bar!\\n\")     "NL
            "        return param                      "NL
            "    end                                   "NL
            "    return foo(param - 1)                 "NL
            "end                                       "NL
            "                                          "NL
            "function foo(param:integer ):integer      "NL
            "    if param <= 0 then                    "NL
            "        write(\"hello from foo!\\n\")     "NL
            "        return param                      "NL
            "    end                                   "NL
            "    return bar(param-1)                   "NL
            "end                                       "NL
    );

    char *description6 = "strings, builtin functions";
    int retcode6 = ERROR_NOERROR;
    pfile_t *pf6 = Pfile.ctor(
            "-- Program 3: Prace s ěretzci a vestavenymi funkcemi"NL
            PROLOG
            "function main()                                    "NL
            "   local s1 : string =                             "NL
            "             \"\\065\\065\\065\\010I\010LOVE\010\" "NL
            "   local s2 : string = s1 .. \"MINECRAFT\\n\"      "NL
            "   write(s2)                                       "NL
            "                                                   "NL
            "   local s1len : integer = #s2                     "NL
            "   s1len = s1len - 4                               "NL
            "   s1 = substr(s2, s1len, s1len + 4)               "NL
            "   write(s1)                                       "NL
            "   write(s2)                                       "NL
            "   s1 = reads()                                    "NL
            "end                                                "NL
            "main()                                             "NL
    );

    char *description7 = "nested while";
    int retcode7 = ERROR_NOERROR;
    pfile_t *pf7 = Pfile.ctor(
            PROLOG
            "function is_beautiful(life : boolean) : boolean"NL
            "   return false                                "NL
            "end                                            "NL

            "function opposite(smth : boolean, a2 : boolean) : boolean"NL
            "   if smth == a2 then return false else return true      "NL
            "end                                                      "NL
            "                                                         "NL
            "function main()                                           "NL
            "   local myself : string  = \"me\"                       "NL
            "   local love : bool                                     "NL
            "   local hate : bool                                     "NL
            "   local faith : bool                                    "NL
            "   local heresy : bool                                   "NL
            "   local death : bool                                    "NL
            "   local life : bool                                     "NL
            "   local indifference : bool                             "NL
            "   local ugliness : bool                                 "NL
            "   local art : bool                                      "NL
            "   while opposite(love, hate) == false and opposite(love, indifference) do                 "NL
            "       while opposite(art, ugliness) == false and opposite(art, indifference) do           "NL
            "           while " "opposite(faith, heresy) == false and opposite(faith, indifference) do  "NL
            "               while " "opposite(life, death) == false and opposite(life, indifference) do "NL
            "                   is_beautiful(life)                                                      "NL
            "               end                                                                         "NL
            "           end                                                                             "NL
            "       end                                                                                 "NL
            "   end                                                                                     "NL
            "end                                                                                        "NL
            "main()                                                                                     "NL
    );

    char *description8 = "elsif, if, NO ERRROR";
    int retcode8 = ERROR_NOERROR;
    pfile_t *pf8 = Pfile.ctor(
            PROLOG
            "function die()                               "NL
            "   write(\"I'm dead x_x\")                   "NL
            "end                                          "NL
            "                                             "NL
            "function must_not_be_reached()               "NL
            "   write(\"MUST NOT BE REACHED x_x\")        "NL
            "end                                          "NL
            "                                             "NL
            "                                             "NL
            "function main()                              "NL
            "    local suka: number = 12                  "NL
            "                                             "NL
            "    if suka > 10 then                        "NL
            "        write(\"suka (number)> 10\")         "NL
            "        local suka:string = \"AAAAAAAAA\"    "NL
            "        if #suka > 10 then                   "NL
            "            must_not_be_reached()            "NL
            "        elseif #suka < 10 then               "NL
            "            die()                            "NL
            "        else                                 "NL
            "            must_not_be_reached()            "NL
            "        end                                  "NL
            "    end                                      "NL
            "end                                          "NL
            "main()                                       "NL
            NL
    );

    char *description9 = "function die() has not been defined";
    int retcode9 = ERROR_DEFINITION;
    pfile_t *pf9 = Pfile.ctor(
            PROLOG
            "function fuck() :string                                 "NL
            "   return \"shit, one more memory leak in the project\" "NL
            "end                                                     "NL
            "                                                        "NL
            "global die :function ()                                 "NL
            "                                             "NL
            "function main()                              "NL
            "    local suka: number = 12                  "NL
            "                                             "NL
            "    if suka > 10 then                        "NL
            "        write(\"suka (number)> 10\")         "NL
            "        local suka:string = \"AAAAAAAAA\"    "NL
            "        if #suka > 10 then                   "NL
            "            must_not_be_reached()            "NL
            "        elseif #suka < 10 then               "NL
            "            die()                            "NL
            "        else                                 "NL
            "            must_not_be_reached()            "NL
            "        end                                  "NL
            "    end                                      "NL
            "end                                          "NL
            "main()                                       "NL
            NL
    );

    char *description10 = "repeat_until";
    int retcode10 = ERROR_NOERROR;
    pfile_t *pf10 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()"NL
            "    write(\"I'm about 2bee printed only once!\")    "NL
            "end                                                 "NL
            "                                                    "NL
            "function yours()                                    "NL
            "   local a : boolean = true                         "NL
            "   repeat                                           "NL
            "       to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()  "NL
            "       a = false                                    "NL
            "   until a == true                                  "NL
            "end                                                 "NL
            "yours()                                             "NL
    );

    char *description11 = "more repuntil, without an error";
    int retcode11 = ERROR_NOERROR;
    pfile_t *pf11 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee() : boolean "NL
            "    write(\"I'm about 2bee printed only once!\")               "NL
            "    return false                                               "NL
            "end                                                            "NL
            "                                                               "NL
            "function yours()                                               "NL
            "   local a : boolean = true                                    "NL
            "   repeat                                                      "NL
            "       a = to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()         "NL
            "   until a == true                                             "NL
            "end                                                            "NL
            "yours()                                                        "NL
    );

    char *description12 = "more repuntil, error with types";
    int retcode12 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf12 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee() : string "NL
            "    return \"I'm about 2bee printed only once!\"              "NL
            "end                                                           "NL
            "                                                              "NL
            "function yours()                                              "NL
            "   local a : boolean = true                                   "NL
            "   repeat                                                     "NL
            "       a = to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()        "NL
            "   until a == true                                            "NL
            "end                                                           "NL
            "yours()                                                       "NL
    );

    char *description13 = "more repuntil, multiple returns";
    int retcode13 = ERROR_SEMANTICS_OTHER; // ?? FIXME: which error is it?
    pfile_t *pf13 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee() : string "NL
            "    return \"I'm about 2bee printed only once!\"              "NL
            "    return false                                              "NL
            "end                                                           "NL
            "                                                              "NL
            "function yours()                                              "NL
            "   local a : boolean = true                                   "NL
            "   repeat                                                     "NL
            "       a = to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()        "NL
            "   until a == true                                            "NL
            "end                                                           "NL
            "yours()                                                       "NL
    );


    char *description14 = "for cycles. Single for cycle.";
    int retcode14 = ERROR_NOERROR;
    pfile_t *pf14 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )  "NL
            "    for i=0,i<number do              "NL
            "        write(\"hello, 10 times\")   "NL
            "    end                              "NL
            "end                                  "NL
            "main(10)                             "NL
    );

    char *description15 = "for cycles";
    int retcode15 = ERROR_NOERROR;
    pfile_t *pf15 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    for i=0,i<number do                  "NL
            "        for j=0,i<number do              "NL
            "            write(\"hello, 9 times\")    "NL
            "        end                              "NL
            "    end                                  "NL
            "end                                      "NL
            "main(3)                                  "NL
    );

    char *description16 = "empty function";
    int retcode16 = ERROR_NOERROR;
    pfile_t *pf16 = Pfile.ctor(
            PROLOG
            "function funnnn()end"
    );

    char *description17 = "stupid function";
    int retcode17 = ERROR_NOERROR;
    pfile_t *pf17 = Pfile.ctor(
            PROLOG
            "function AAAAAA()                 "NL
            "   write(\"AAAAAA() is stupid\")  "NL
            "end                               "NL
            "function main()                   "NL
            "   AAAAAA()                       "NL
            "end                               "NL
            "main()                            "NL

    );

    char *description18 = "expression error";
    int retcode18 = ERROR_SYNTAX;
    pfile_t *pf18 = Pfile.ctor(
            PROLOG
            "function main()  "NL
            "   +++()         "NL
            "end              "NL
            ""
    );

    char *description19 = "while, no error";
    int retcode19 = ERROR_NOERROR;
    pfile_t *pf19 = Pfile.ctor(
            PROLOG
            "function main()             "NL
            "    local a : integer = 10  "NL
            "    while a >= 0 do         "NL
            "        write (\"10 times\")"NL
            "        a = a - 1           "NL
            "    end                     "NL
            "end                         "NL
            "main()                      "NL
    );

    char *description20 = "nested whiles ";
    int retcode20 = ERROR_NOERROR;
    pfile_t *pf20 = Pfile.ctor(
            PROLOG
            "function main()                    "NL
            "    local a : integer = 2          "NL
            "    local b : integer = 2          "NL
            "    local c : integer = 2          "NL
            "    while a >= 0 do                "NL
            "        a = a-1                    "NL
            "        while b >= 0 do            "NL
            "            b = b-1                "NL
            "            while c >= 0 do        "NL
            "                c = c-1            "NL
            "                write (\"9 times\")"NL
            "            end                    "NL
            "        end                        "NL
            "    end                            "NL
            "end                                "NL
            "main()                             "NL
    );

    char *description21 = "function";
    int retcode21 = ERROR_NOERROR;
    pfile_t *pf21 = Pfile.ctor(
            PROLOG NL
            "function main()" NL
            "     local s1 : string = \"HELLO\" "NL
            "     local s2 : string = s1        "NL
            "     local I1 : integer = #s1      "NL
            "     local s1len : integer = #s2   "NL
            "end " NL NL
            "main()" NL
    );

    char *description22 = "more repuntil. Without an error";
    int retcode22 = ERROR_NOERROR;
    pfile_t *pf22 = Pfile.ctor(
            PROLOG NL
            "function me() :string                                            "NL
            "    return \"the whole project was written by a cat\"            "NL
            "end                                                              "NL

            "function main()                                                  "NL
            " local you : string = \"atata\"                                 "NL
            "     repeat                                                     "NL
            "         repeat                                                 "NL
            "             repeat                                             "NL
            "                 repeat                                         "NL
            "                     repeat                                     "NL
            "                         repeat                                 "NL
            "                             local not_true : string = me()     "NL
            "                              write(not_true)                   "NL
            "                         until false                            "NL
            "                     until 1 < 0                                "NL
            "                 until 2 + 2 = 5                                "NL
            "             until false                                        "NL
            "         until true and false                                   "NL
            "     until false or false                                       "NL
            "                                                                "NL
            "    return                                                      "NL
            "end                                                             "NL
            "main()                                                          "NL
    );

    char *description23 = "semantic: more than one declaration (error)";
    int retcode23 = ERROR_DEFINITION;
    pfile_t *pf23 = Pfile.ctor(
            PROLOG
            "function me() : string"
            "end "

            "function me() : integer"
            "end "

            "function me() : boolean"
            "end "
    );

    char *description24 = "more repuntil. No Until";
    int retcode24 = ERROR_SYNTAX;
    pfile_t *pf24 = Pfile.ctor(
            PROLOG NL
            "function me() :string                                            "NL
            "    return \"the whole project was written by a dog\"            "NL
            "end                                                              "NL

            "function main()                                                  "NL
            " local you : string = \"atata\"                                 "NL
            "     repeat                                                     "NL
            "         repeat                                                 "NL
            "             repeat                                             "NL
            "                 repeat                                         "NL
            "                     repeat                                     "NL
            "                         repeat                                 "NL
            "                             local not_true : string = me()     "NL
            "                              write(not_true)                   "NL
            "                         until false                            "NL
            "                     until 1 < 0                                "NL
            "                 until 2 + 2 = 5                                "NL
            "             until false                                        "NL
            "         until true and false                                   "NL
            "                                                                "NL
            "    return                                                      "NL
            "end                                                             "NL
            "main()                                                          "NL
    );

    char *description25 = "more repuntil. No Until";
    int retcode25 = ERROR_SYNTAX;
    pfile_t *pf25 = Pfile.ctor(
            PROLOG NL
            "function me() :string                                           "NL
            "    return \"the whole project was written by a cat \"          "NL
            "end                                                             "NL

            "function main()                                                  "NL
            " local you : string = \"atata\"                                 "NL
            "     repeat                                                     "NL
            "         repeat                                                 "NL
            "             repeat                                             "NL
            "                 repeat                                         "NL
            "                     repeat                                     "NL
            "                         repeat                                 "NL
            "                             local not_true : string = me()     "NL
            "                              write(not_true)                   "NL
            "                         until false                            "NL
            "    return                                                      "NL
            "end                                                             "NL
            "main()                                                          "NL
    );

    char *description26 = "bad returns";
    int retcode26 = ERROR_SYNTAX;
    pfile_t *pf26 = Pfile.ctor(
            PROLOG
            "function me() : write"NL
            "me()                 "NL
    );

    char *description27 = "unexpected token.";
    int retcode27 = ERROR_SYNTAX;
    pfile_t *pf27 = Pfile.ctor(
            PROLOG
            "function function : string"NL
            "end                       "NL
            "function()                "NL
    );

    char *description28 = "function semantic: declare builtin function(error)";
    int retcode28 = ERROR_DEFINITION;
    pfile_t *pf28 = Pfile.ctor(
            PROLOG
            "global readi : function()"NL
            "function me() : string   "NL
            "end                      "NL
            "me()                     "NL
    );

    char *description29 = "function semantic: no error.";
    int retcode29 = ERROR_NOERROR;
    pfile_t *pf29 = Pfile.ctor(
            PROLOG NL
            "global foo : function(    string,     string ) : string, number "NL
            "function foo (a : string, b : string) :  string, number         "NL
            "    return \"hello\", 10                                        "NL
            "end                                                             "NL
            "function main()                                                 "NL
            "   local a : string                                             "NL
            "   local b : number                                             "NL
            "   a, b = foo()                                                 "NL
            "   write(a)                                                     "NL
            "   write(b)                                                     "NL
            "end                                                             "NL
            "main()                                                          "NL
    );

    char *description30 = "function semantic: declaration without definiton (error)";
    int retcode30 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf30 = Pfile.ctor(
            PROLOG
            "global foo : function( string, string ) : string, number"
    );

    char *description31 = "function returns nil, no error.";
    int retcode31 = ERROR_NOERROR;
    pfile_t *pf31 = Pfile.ctor(
            PROLOG
            "global foo : function( nil, nil ) : nil"NL
            "function foo(a : nil, b : nil) : nil   "NL
            "end                                    "NL
            "foo()                                  "NL
    );

    char *description32 = "nil as a local variable.";
    int retcode32 = ERROR_NOERROR;
    pfile_t *pf32 = Pfile.ctor(
            PROLOG
            "function mein()                "NL
            "   local NULL : string  = nil  "NL
            "end                            "NL
            "mein()                         "NL
    );

    char *description33 = "Function semantics: nil in declaration, but not in definition";
    int retcode33 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf33 = Pfile.ctor(
            PROLOG
            "global foo : function( string, string ) : nil"NL
            "function foo () : string                     "NL
            "end                                          "NL
    );

    char *description34 = "Function semantics def before decl. Expr as an argument.";
    int retcode34 = ERROR_NOERROR;
    pfile_t *pf34 = Pfile.ctor(
            PROLOG
            "function foo( a : string, b : string ) : nil "NL
            "end                                          "NL

            "global foo : function(string, string ) : nil  "NL
            "foo(\"I \" .. \"LOVE\",  \"MINECRAFT\")  "NL
    );

    char *description35 = "syntax error";
    int retcode35 = ERROR_SYNTAX;
    pfile_t *pf35 = Pfile.ctor(
            PROLOG
            "function 123name ()"NL
            "end                "NL
    );

    char *description36 = "empty file";
    int retcode36 = ERROR_NOERROR;
    pfile_t *pf36 = Pfile.ctor(
            PROLOG
    );

    char *description37 = "Function semantics. Parameters have same names.";
    int retcode37 = ERROR_DEFINITION;
    pfile_t *pf37 = Pfile.ctor(
            PROLOG
            "function foo( a : number, a : string ) : nil "NL
            "end                                          "NL
            "foo()                                        "NL
    );

    char *description38 = "Complicated expr in function calling";
    int retcode38 = ERROR_NOERROR;
    pfile_t *pf38 = Pfile.ctor(
            PROLOG
            "functino three(): number return 3 end        "NL
            "function foo( a : number, b : number )       "NL
            "    write(a)                                 "NL
            "    write(b)                                 "NL
            "end                                          "NL
            "foo(1 + 2 + 3 + 4 * (2 + 3 + three()), 1)    "NL
    );

    char *description39 = "wrong number of aparameters in function calling";
    int retcode39 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf39 = Pfile.ctor(
            PROLOG
            "functino three(): number return 3 end        "NL
            "function foo( a : number, b : number )       "NL
            "    write(a)                                 "NL
            "    write(b)                                 "NL
            "end                                          "NL
            "foo(1 + 2 + 3 + 4 * (2 + 3 + three()))       "NL
    );

    char *description40 = "no expression after comma";
    int retcode40 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf40 = Pfile.ctor(
            PROLOG
            "functino three(): number return 3 end        "NL
            "function foo( a : number, b : number )       "NL
            "    write(a)                                 "NL
            "    write(b)                                 "NL
            "end                                          "NL
            "foo(1 + 2 + 3 + 4 * (2 + 3 + three()), )     "NL
    );

    char *description41 = "no expression after comma";
    int retcode41 = ERROR_SYNTAX;
    pfile_t *pf41 = Pfile.ctor(
            PROLOG
            "functino three(): number return 3 end        "NL
            "function foo( a : number, b : number )       "NL
            "    write(a)                                 "NL
            "    write(b)                                 "NL
            "end                                          "NL
            "foo(1 + 2 + 3 + 4 * (2 + 3 + three()), ((()((     "NL
    );

    char *description42 = "undefined function.";
    int retcode42 = ERROR_DEFINITION;
    pfile_t *pf42 = Pfile.ctor(
            PROLOG
            "function foo( r : string, a : string ) : nil"NL
            "    func42()                                "NL
            "end                                         "NL
    );


    char *description43 = "no error.";
    int retcode43 = ERROR_NOERROR;
    pfile_t *pf43 = Pfile.ctor(
            PROLOG
            "function foo( r : string, a : string ) : nil"NL
            "   write(\" \\xff \")                       "NL
            "end                                         "NL
    );

    char *description44 = "if else esleif no errors.";
    int retcode44 = ERROR_NOERROR;
    pfile_t *pf44 = Pfile.ctor(
            "require \"ifj21\"                              "NL
            "function foo( r : string, a : string ) : string"NL
            "    local A : integer = 10                     "NL
            "    local B : integer = 10                     "NL
            "    if A == B then                             "NL
            "        local hello : integer = 10             "NL
            "        if A == B then                         "NL
            "           local world: string = \"hello\"     "NL
            "            if A == B then                     "NL
            "               local newl : integer            "NL
            "            elseif A ~= B then                 "NL
            "            end                                "NL
            "        end                                    "NL
            "    else                                       "NL
            "       local arsoui : integer                  "NL
            "    end                                        "NL
            "end                                            "NL
    );

    char *description45 = "Syntax. No prolog string";
    int retcode45 = ERROR_SYNTAX;
    pfile_t *pf45 = Pfile.ctor(
            "function foo( r : string, a : string ) : string\n"
            "    local A : integer = 10                      "NL
            "    local B : integer = 10                      "NL
            "    if A == B then                              "NL
            "        local hello : integer = 10              "NL
            "        if A == B then                          "NL
            "            local world: string = \"hello\"     "NL
            "            if A == B then                      "NL
            "                local newl : integer            "NL
            "            elseif A ~= B then                  "NL
            "            end                                 "NL
            "        end                                     "NL
            "    else                                        "NL
            "        local arsoui : integer                  "NL
            "    end                                         "NL
            "end                                             "NL
    );

    char *description46 = "Syntax. wrong prolog string";
    int retcode46 = ERROR_SYNTAX;
    pfile_t *pf46 = Pfile.ctor(
            "require \"love\"" NL
    );

    char *description47 = "no error, parsing definitions, declarations";
    int retcode47 = ERROR_NOERROR;
    pfile_t *pf47 = Pfile.ctor(
            PROLOG
            "global baz : function(string)  "NL
            "function baz(str : string)     "NL
            "end                            "NL

            "global bar : function(string, integer)    "NL
            "function bar(str : string, int : integer) "NL
            "end                                       "NL NL

            " global                                                                      "NL
            "     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) :       "NL
            "                                                                      string,"NL
            "                                                                      string,"NL
            "                                                                      string "NL
            "function                                                                     "NL
            "     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) :           "NL
            "                                                                      string,"NL
            "                                                                      string,"NL
            "                                                                      string "NL
            "    write(\"hello there\")                                                   "NL
            "end "NL NL

            " global arst : function(string,         integer,             number,       number,     integer, string)" NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)" NL
            "end " NL
    );

    char *description48 = "no error, parsing definitions, declarations";
    int retcode48 = ERROR_NOERROR;
    pfile_t *pf48 = Pfile.ctor(
            PROLOG

            " global bar : function(string, integer)" NL
            "function bar(str : string, int : integer)" NL
            "end " NL

            " global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n" NL
            "function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) : string, string, string\n" NL
            "end " NL

            NL
            " global arst : function(string,         integer,             number,       number,     integer, string)" NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)" NL
            "end " NL
    );

    char *description49 = "no error, parsing definitions, declarations";
    int retcode49 = ERROR_NOERROR;
    pfile_t *pf49 = Pfile.ctor(
            PROLOG
            " global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n" NL
            "function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) : string, string, string\n" NL
            "end " NL

            NL
            " global arst : function(string,         integer,             number,       number,     integer, string)" NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)" NL
            "end " NL
    );

    char *description50 = "no error, parsing definitions, declarations";
    int retcode50 = ERROR_NOERROR;
    pfile_t *pf50 = Pfile.ctor(
            PROLOG
            NL
            " global arst : function(string,         integer,             number,       number,     integer, string)" NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)" NL
            "end " NL
    );

    char *description51 = "no error, parsing definitions, declarations";
    int retcode51 = ERROR_NOERROR;
    pfile_t *pf51 = Pfile.ctor(
            PROLOG
            "function foo()" NL
            "end " NL

            "function baz(str : string)" NL
            "end " NL

            " global bar : function(string, integer)" NL
            "function bar(str : string, int : integer)" NL
            "end " NL

            " global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n" NL
            "function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) : string, string, string\n" NL
            "end " NL

            NL
            " global arst : function(string,         integer,             number,       number,     integer, string)" NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)" NL
            "end " NL
    );

    char *description52 = "no error, parsing definitions, declarations";
    int retcode52 = ERROR_NOERROR;
    pfile_t *pf52 = Pfile.ctor(
            PROLOG
            " global bar : function(string, integer)" NL
            "function bar(str : string, int : integer)" NL
            "end " NL

            NL
            "global arst : function(string,         integer,             number,       number,     integer, string)" NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)" NL
            "end " NL
    );


    TEST_CASE(1);
    TEST_CASE(2);
    TEST_CASE(3);
    TEST_CASE(4);
    TEST_CASE(5);
    TEST_CASE(6);
    TEST_CASE(7);
    TEST_CASE(8);
    TEST_CASE(9);

    TEST_CASE(10);
    TEST_CASE(11);
    TEST_CASE(12);
    TEST_CASE(13);
    TEST_CASE(14);
    TEST_CASE(15);
    TEST_CASE(16);
    TEST_CASE(17);
    TEST_CASE(18);
    TEST_CASE(19);

    TEST_CASE(20);
    TEST_CASE(21);
    TEST_CASE(22);
    TEST_CASE(23);
    TEST_CASE(24);
    TEST_CASE(25);
    TEST_CASE(26);
    TEST_CASE(27);
    TEST_CASE(28);
    TEST_CASE(29);

    TEST_CASE(30);
    TEST_CASE(31);
    TEST_CASE(32);
    TEST_CASE(33);
    TEST_CASE(34);
    TEST_CASE(35);
    TEST_CASE(36);
    TEST_CASE(37);
    TEST_CASE(38);
    TEST_CASE(39);

    TEST_CASE(40);
    TEST_CASE(41);
    TEST_CASE(42);
    TEST_CASE(43);
    TEST_CASE(44);
    TEST_CASE(45);
    TEST_CASE(46);
    TEST_CASE(47);
    TEST_CASE(48);
    TEST_CASE(49);

    TEST_CASE(50);
    TEST_CASE(51);
    TEST_CASE(52);

    return 0;
}
