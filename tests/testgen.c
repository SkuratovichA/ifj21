
#include <stdio.h>

#include "../src/progfile.h"
#include <assert.h>


#define NL "\n"
#define PROLOG "require \"ifj21\" \n"

#define TEST_CASE(number)                                                    \
  do {                                                                       \
      char *_filname;                                                        \
      switch (retcode##number) {                                             \
          case    ERROR_DEFINITION:                                          \
          case    ERROR_TYPE_MISSMATCH:                                      \
          case    ERROR_FUNCTION_SEMANTICS:                                  \
          case    ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY:                    \
          case    ERROR_SEMANTICS_OTHER:                                     \
              _filname = "../tests/semantic_errors/sasha" #number "_";       \
              break;                                                         \
          case ERROR_NOERROR:                                                \
              _filname = "../tests/without_errors/sasha" #number "_";        \
              break;                                                         \
          case ERROR_SYNTAX:                                                 \
              _filname = "../tests/syntax_errors/sasha" #number "_";         \
              break;                                                         \
          case ERROR_LEXICAL:                                                \
              _filname = "../tests/lexical_errors/sasha" #number "_";        \
              break;                                                         \
          default:                                                           \
              debug_msg_s("Undefined error code: %d\n", retcode##number);    \
              _filname = "sasha";                                            \
      }                                                                      \
      dynstring_t *filnam = Dynstring.ctor(_filname);                        \
      Dynstring.append(filnam, retcode ## number + '0');                     \
      Dynstring.cat(filnam, Dynstring.ctor(".tl"));                          \
      FILE *fil = fopen(Dynstring.c_str(filnam), "w");                       \
      assert(fil);                                                           \
      fprintf(fil, "-- test case %d.\n"                                      \
                  "-- Description : %s\n\n"                                  \
                  "-- Expected : '%d'\n",                                    \
                      number, description ## number, retcode ## number);     \
          fprintf(fil, "%s\n", Pfile.get_tape(pf ## number));                \
          fclose(fil);                                                       \
      Pfile.dtor(pf ## number);                                              \
      debug_msg_s("file created: "                                           \
          "%s %s%c.tl\n",                                                    \
          Dynstring.c_str(filnam),                                           \
          _filname, retcode##number + '0');                                  \
      Dynstring.dtor(filnam);                                                \
  } while (0)

#define STRING_NOERROR(_num, _string)                            \
        char *description##_num = "good string";                 \
        int retcode##_num = ERROR_NOERROR;                       \
        pfile_t *pf##_num = Pfile.ctor(                          \
            PROLOG                                               \
            "function one()                             "NL      \
            "    local s : string =  \"\\x" _string "\" "NL      \
            "end                                        "NL      \
        );                                                       \

#define STRING_ERROR(_num, _string)                            \
        char *description##_num = "badstring";                 \
        int retcode##_num = ERROR_LEXICAL;                     \
        pfile_t *pf##_num = Pfile.ctor(                        \
            PROLOG                                             \
            "function one()                             "NL    \
            "    local s : string =      " _string "     "NL   \
            "end                                        "NL    \
        );                                                     \

#define GLOBAL_EXPRESSION(_num, _expr, _errcode)                                                    \
        char *description##_num = "global expression";                                              \
        int retcode##_num = _errcode;                                                               \
        pfile_t *pf##_num = Pfile.ctor(                                                             \
            PROLOG                                                                                  \
            "function a(_a : number ) : integer return 1 end "NL                                    \
            "function b(_a : number ) : integer return 1 end "NL                                    \
            "function c(_a : number ) : integer return 1 end "NL                                    \
            "function d(_a : number ) : integer return 1 end "NL                                    \
            "function e(_a : number ) : integer return 1 end "NL                                    \
            "function f(_a : number ) : integer return 1 end "NL                                    \
            "function aa(_a : number, _b : number ) : integer return 1 end "NL                      \
            "function bb(_a : number, _b : number ) : integer return 1 end "NL                      \
            "function cc(_a : number, _b : number) : integer return 1 end "NL                       \
            "function dd(_a : number, _b : number) : integer return 1 end "NL                       \
            "function ee(_a : number, _b : number) : integer return 1 end "NL                       \
            "function ff(_a : number, _b : number) : integer return 1 end "NL                       \
            "function aaa(_a : number, _b : number, _c : number ) : integer return 1 end "NL        \
            "function bbb(_a : number, _b : number, _c : number ) : integer return 1 end "NL        \
            "function ccc(_a : number, _b : number, _c : number ) : integer return 1 end "NL        \
            "function ddd(_a : number, _b : number, _c : number ) : integer return 1 end "NL        \
            "function eee(_a : number, _b : number, _c : number ) : integer return 1 end "NL        \
            "function fff(_a : number, _b : number, _c : number ) : integer return 1 end "NL        \
            _expr                                                                         NL        \
        );

#define GLOBAL_EXPRESSION_ERROR(_num, _expr) GLOBAL_EXPRESSION(_num, _expr, ERROR_SYNTAX)
#define GLOBAL_EXPRESSION_NOERROR(_num, _expr) GLOBAL_EXPRESSION(_num, _expr, ERROR_NOERROR)

int main() {

    system("rm -rf ../tests/*errors && echo \"Directories deleted.\"");
    system("mkdir ../tests/lexical_errors");
    system("mkdir ../tests/syntax_errors");
    system("mkdir ../tests/semantic_errors");
    system("mkdir ../tests/without_errors");

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
            "-- Program 3: Prace s ??retzci a vestavenymi funkcemi"NL
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
            "   if smth == a2 then return false else return true end  "NL
            "end                                                      "NL
            "                                                         "NL
            "function main()                                          "NL
            "   local myself : string  = \"me\"                       "NL
            "   local love : boolean                                    "NL
            "   local hate : boolean                                    "NL
            "   local faith : boolean                                   "NL
            "   local heresy : boolean                                  "NL
            "   local death : boolean                                   "NL
            "   local life : boolean                                    "NL
            "   local indifference : boolean                            "NL
            "   local ugliness : boolean                                "NL
            "   local art : boolean                                     "NL
            "   while opposite(love, hate) == false and opposite(love, indifference) do                 "NL
            "       while opposite(art, ugliness) == false and opposite(art, indifference) do           "NL
            "           while opposite(faith, heresy) == false and opposite(faith, indifference) do  "NL
            "               while opposite(life, death) == false and opposite(life, indifference) do "NL
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
            "                                                        "NL
            "function main()                                         "NL
            "    local suka: number = 12                             "NL
            "                                                        "NL
            "    if suka > 10 then                                   "NL
            "        write(\"suka (number)> 10\")                    "NL
            "        local suka:string = \"AAAAAAAAA\"               "NL
            "        if #suka > 10 then                              "NL
            "            must_not_be_reached()                       "NL
            "        elseif #suka < 10 then                          "NL
            "            die()                                       "NL
            "        else                                            "NL
            "            must_not_be_reached()                       "NL
            "        end                                             "NL
            "    end                                                 "NL
            "end                                                     "NL
            "main()                                                  "NL
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
    int retcode13 = ERROR_SYNTAX;
    pfile_t *pf13 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee() : string "NL
            "    return \"I'm about 2bee printed only once!\"              "NL
            "    return \"a\"                                              "NL
            "end                                                           "NL
            "                                                              "NL
            "function yours()                                              "NL
            "   local a : string                                           "NL
            "   repeat                                                     "NL
            "       a = to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()        "NL
            "   until a == \"a\"                                           "NL
            "end                                                           "NL
            "yours()                                                       "NL
    );


    char *description14 = "for cycles. Single for cycles with an error.";
    int retcode14 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf14 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )  "NL
            "    for i=0,i<iterations do          "NL
            "        write(\"hello, 10 times\")   "NL
            "    end                              "NL
            "end                                  "NL
            "main(10)                             "NL
    );

    char *description15 = "for cycles";
    int retcode15 = ERROR_NOERROR;
    pfile_t *pf15 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )  "NL
            "    for i=0,iterations do            "NL
            "        write(\"hello, 10 times\")   "NL
            "    end                              "NL
            "end                                  "NL
            "main(10)                             "NL
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
            "                 until 2 + 2 == 5                                "NL
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
            "function me() : string "NL
            "end                    "NL

            "function me() : integer"NL
            "end                    "NL

            "function me() : boolean"NL
            "end                     "
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
            "                 until 2 + 2 == 5                               "NL
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
            "function me() :string                                         "NL
            "    return \"the whole project was written by a cat \"        "NL
            "end                                                           "NL

            "function main()                                               "NL
            "   local you : string = \"atata\"                             "NL
            "   repeat                                                     "NL
            "       repeat                                                 "NL
            "           repeat                                             "NL
            "               repeat                                         "NL
            "                   repeat                                     "NL
            "                       repeat                                 "NL
            "                           local not_true : string = me()     "NL
            "                            write(not_true)                   "NL
            "                       until false                            "NL
            "                       return                                 "NL
            "end                                                           "NL
            "main()                                                        "NL
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
            "   a, b = foo(\"test\", \"text\")                               "NL
            "   write(a)                                                     "NL
            "   write(b)                                                     "NL
            "end                                                             "NL
            "main()                                                          "NL
    );

    char *description30 = "declaration without definiton (error)";
    int retcode30 = ERROR_DEFINITION;
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
            "foo(nil, nil)                          "NL
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
    int retcode33 = ERROR_DEFINITION;
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
            "function foo( a : string, b : string ) : nil   "NL
            "end                                            "NL

            "global foo : function(string, string ) : nil   "NL
            "foo(\"I \" .. \"LOVE\",  \"MINECRAFT\")        "NL
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
            "function three(): number return 3 end        "NL
            "function foo( a : number, b : number )       "NL
            "    write(a)                                 "NL
            "    write(b)                                 "NL
            "end                                          "NL
            "foo(1 + 2 + 3 + 4 * (2 + 3 + three()), 1)    "NL
    );

    char *description39 = "wrong number of paarameters in function calling";
    int retcode39 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf39 = Pfile.ctor(
            PROLOG
            "function three(): number return 3 end        "NL
            "function foo( a : number, b : number )       "NL
            "    write(a)                                 "NL
            "    write(b)                                 "NL
            "end                                          "NL
            "foo(1 + 2 + 3 + 4 * (2 + 3 + three()))       "NL
    );

    char *description40 = "no expression after comma";
    int retcode40 = ERROR_SYNTAX;
    pfile_t *pf40 = Pfile.ctor(
            PROLOG
            "function three(): number return 3 end        "NL
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
            "function three(): number return 3 end              "NL
            "function foo( a : number, b : number )             "NL
            "    write(a)                                       "NL
            "    write(b)                                       "NL
            "end                                                "NL
            "foo(1 + 2 + 3 + 4 * (2 + 3 + three()), ((()((      "NL
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
    int retcode46 = ERROR_SEMANTICS_OTHER;
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
            " global bar : function(string, integer)                                                                                "NL
            "function bar(str : string, int : integer)                                                                              "NL
            "end                                                                                                                    "NL
                                                                                                                                     NL
            "global arst : function(string,         integer,             number,       number,     integer, string)                 "NL
            "function "              "arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)    "NL
            "end " NL
    );

    char *description53 = "return types #1.";
    int retcode53 = ERROR_NOERROR;
    pfile_t *pf53 = Pfile.ctor(
            PROLOG NL
            "global foo : function(    string,     string ) : string, number "NL
            "function foo (a : string, b : string) :  string, number         "NL
            "    return \"hello\", 10.5 + 1.5                                "NL
            "end                                                             "NL
            "foo(\"test\", \"text\")                                         "NL
    );

    char *description54 = "return types #2 no returns.";
    int retcode54 = ERROR_NOERROR;
    pfile_t *pf54 = Pfile.ctor(
            PROLOG NL
            "global foo : function(    string,     string ) : string, number "NL
            "function foo (a : string, b : string) :  string, number         "NL
            "end                                                             "NL
            "foo(\"test\", \"text\")                                         "NL
    );

    char *description55 = "return types #3. Typecasting";
    int retcode55 = ERROR_NOERROR;
    pfile_t *pf55 = Pfile.ctor(
            PROLOG NL
            "global foo : function(    string,     string ) : string, number "NL
            "function foo (a : string, b : string) :  string, number         "NL
            "    return \"hello\", 10                                        "NL
            "end                                                             "NL
            "foo(\"test\", \"text\")                                         "NL
    );

    char *description56 = "return types #4. Exceed returns";
    int retcode56 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf56 = Pfile.ctor(
            PROLOG NL
            "global foo : function(    string,     string ) : string, number "NL
            "function foo (a : string, b : string) :  string, number         "NL
            "    return \"hello\", 10, 10                                    "NL
            "end                                                             "NL

            "foo()                                                           "NL
    );

    char *description57 = "return types #5. Wrong return types";
    int retcode57 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf57 = Pfile.ctor(
            PROLOG NL
            "global foo : function(    string,     string ) : string, number "NL
            "function foo (a : string, b : string) :  string, number         "NL
            "    return 10, 10                                               "NL
            "end                                                             "NL

            "foo()                                                           "NL
    );

    char *description58 = "return types #6. Typecasting nil in return.";
    int retcode58 = ERROR_NOERROR;
    pfile_t *pf58 = Pfile.ctor(
            PROLOG NL
            "global foo : function(    string,     string ) : string, number "NL
            "function foo (a : string, b : string) :  string, number         "NL
            "    return nil, nil                                             "NL
            "end                                                             "NL
            "foo(\"test\", \"text\")                                         "NL
    );

    char *description59 = "return types #7. Typecasting nil in return.";
    int retcode59 = ERROR_NOERROR;
    pfile_t *pf59 = Pfile.ctor(
            PROLOG NL
            "global foo : function(string,string ) :               "NL
            "                              string, number, boolean "NL
            "function foo (a : string, b : string) :               "NL
            "                              string, number, boolean "NL
            "    return nil, nil, true                             "NL
            "end                                                   "NL
            "foo(\"test\", \"text\")                               "NL
    );

    char *description60 = "return types #8";
    int retcode60 = ERROR_NOERROR;
    pfile_t *pf60 = Pfile.ctor(
            PROLOG
            "function is_beautiful(life : boolean) : boolean          "NL
            "   return false                                          "NL
            "end                                                      "NL
            "function opposite(smth : boolean, a2 : boolean) : boolean"NL
            "   if smth == a2 then                                    "NL
            "       return false                                      "NL
            "   else                                                  "NL
            "       return true                                       "NL
            "   end                                                   "NL
            "end                                                      "NL

    );

    char *description61 = "undef function";
    int retcode61 = ERROR_DEFINITION;
    pfile_t *pf61 = Pfile.ctor(
            PROLOG
            "function main(life : boolean) : boolean                  "NL
            "   return foo()                                          "NL
            "end                                                      "NL

    );

    char *description62 = "for cycles, type mismatched";
    int retcode62 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf62 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    for i=0,iterations,1 do              "NL
            "        for j=0,i<iterations,1 do        "NL
            "            write(\"hello, 9 times\")    "NL
            "        end                              "NL
            "    end                                  "NL
            "end                                      "NL
            "main(3)                                  "NL
    );

    char *description63 = "for cycles";
    int retcode63 = ERROR_NOERROR;
    pfile_t *pf63 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    for i=0,iterations,2 do              "NL
            "        for j=0,iterations,2 do          "NL
            "            write(\"hello, many times\") "NL
            "        end                              "NL
            "    end                                  "NL
            "end                                      "NL
            "main(4)                                  "NL
    );

    char *description64 = "for cycles";
    int retcode64 = ERROR_NOERROR;
    pfile_t *pf64 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    for i=0,iterations do                "NL
            "        for j=0,iterations do            "NL
            "            write(\"hello, 9 times\")    "NL
            "        end                              "NL
            "    end                                  "NL
            "end                                      "NL
            "main(3)                                  "NL
    );

    char *description65 = "break keyword in repeat-until";
    int retcode65 = ERROR_NOERROR;
    pfile_t *pf65 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()"NL
            "    write(\"I'm about 2bee printed only once!\")    "NL
            "end                                                 "NL
            "                                                    "NL
            "function yours()                                    "NL
            "   local a : boolean = true                         "NL
            "   repeat                                           "NL
            "       to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()  "NL
            "                                                    "NL
            "       a = false                                    "NL
            "       break                                        "NL
            "   until a == true                                  "NL
            "end                                                 "NL
            "yours()                                             "NL
    );

    char *description66 = "for cycles, break keyword";
    int retcode66 = ERROR_NOERROR;
    pfile_t *pf66 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    for i=0,iterations do                "NL
            "       write(\"I'm gonna be printed\\n\")"NL
            "        for j=0, iterations do           "NL
            "            break                        "NL
            "            write(\"hello, 9 times\")    "NL
            "        end                              "NL
            "    end                                  "NL
            "end                                      "NL
            "main(3)                                  "NL
    );

    char *description67 = "while cycle.";
    int retcode67 = ERROR_NOERROR;
    pfile_t *pf67 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )              "NL
            "    local i : integer = 10                       "NL
            "    while i ~= 0 do                              "NL
            "       write(\"I'm gonna be printed 5 times\\n\")"NL
            "       i = i - 2                                 "NL
            "    end                                          "NL
            "end                                              "NL
            "main(3)                                          "NL
    );

    char *description68 = "while cycle.";
    int retcode68 = ERROR_NOERROR;
    pfile_t *pf68 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "       local j : integer = 3                           "NL
            "       while j ~= 0 do                                 "NL
            "          local k : integer = 3                        "NL
            "          while k ~= 0 do                              "NL
            "              write(\"hello\")                         "NL
            "              k = k - 1                                "NL
            "          end                                          "NL
            "          j = j - 1                                    "NL
            "       end                                             "NL
            "       i = i - 2                                       "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description69 = "while cycle. Break inside while";
    int retcode69 = ERROR_NOERROR;
    pfile_t *pf69 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "        local j : integer = 3                          "NL
            "        while j ~= 0 do                                "NL
            "           local k : integer = 3                       "NL
            "           while k ~= 0 do                             "NL
            "               break                                   "NL
            "               write(\"hello\")                        "NL
            "               k = k - 1                               "NL
            "           end                                         "NL
            "           j = j - 1                                   "NL
            "        end                                            "NL
            "        i = i - 2                                      "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description70 = "while cycle. Break inside while, break after a break";
    int retcode70 = ERROR_NOERROR;
    pfile_t *pf70 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "        local j : integer = 3                          "NL
            "        while j ~= 0 do                                "NL
            "           local k : integer = 3                       "NL
            "           while k ~= 0 do                             "NL
            "               break                                   "NL
            "               write(\"hello\")                        "NL
            "               k = k - 1                               "NL
            "           end                                         "NL
            "           break                                       "NL
            "           j = j - 1                                   "NL
            "        end                                            "NL
            "        i = i - 2                                      "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description71 = "while cycle. Break inside if statement";
    int retcode71 = ERROR_NOERROR;
    pfile_t *pf71 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "        local j : integer = 3                          "NL
            "        while j ~= 0 do                                "NL
            "           local k : integer = 3                       "NL
            "           while k ~= 0 do                             "NL
            "               if k > 0 then break end                 "NL
            "               write(\"hello\")                        "NL
            "               k = k - 1                               "NL
            "           end                                         "NL
            "           break                                       "NL
            "           j = j - 1                                   "NL
            "        end                                            "NL
            "        i = i - 2                                      "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description72 = "while cycle. Break inside elseif statement";
    int retcode72 = ERROR_NOERROR;
    pfile_t *pf72 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "        local j : integer = 3                          "NL
            "        while j ~= 0 do                                "NL
            "           local k : integer = 3                       "NL
            "           while k ~= 0 do                             "NL
            "               if k > 0 then                           "NL
            "               elseif k > 2 then                       "NL
            "                   break                               "NL
            "               end                                     "NL
            "               write(\"hello\")                        "NL
            "               k = k - 1                               "NL
            "           end                                         "NL
            "           break                                       "NL
            "           j = j - 1                                   "NL
            "        end                                            "NL
            "        i = i - 2                                      "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description73 = "while cycle. Break inside elseif statement #2";
    int retcode73 = ERROR_NOERROR;
    pfile_t *pf73 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "        local j : integer = 3                          "NL
            "        while j ~= 0 do                                "NL
            "           local k : integer = 3                       "NL
            "           while k ~= 0 do                             "NL
            "               if k > 0 then                           "NL
            "               elseif k > 2 then                       "NL
            "                   break                               "NL
            "                   write (\"Something after break\")   "NL
            "               end                                     "NL
            "               write(\"hello\")                        "NL
            "               k = k - 1                               "NL
            "           end                                         "NL
            "           break                                       "NL
            "           j = j - 1                                   "NL
            "        end                                            "NL
            "        i = i - 2                                      "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description74 = "while cycle. Break inside elseif statement #2";
    int retcode74 = ERROR_NOERROR;
    pfile_t *pf74 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "        local j : integer = 3                          "NL
            "        while j ~= 0 do                                "NL
            "           local k : integer = 3                       "NL
            "           while k ~= 0 do                             "NL
            "               if k > 0 then                           "NL
            "               elseif k > 2 then                       "NL
            "               else                                    "NL
            "                   break                               "NL
            "               end                                     "NL
            "               write(\"hello\")                        "NL
            "               k = k - 1                               "NL
            "           end                                         "NL
            "           break                                       "NL
            "           j = j - 1                                   "NL
            "        end                                            "NL
            "        i = i - 2                                      "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description75 = "while cycle. Break inside elseif statement #2";
    int retcode75 = ERROR_NOERROR;
    pfile_t *pf75 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )                    "NL
            "    local i : integer = 3                              "NL
            "    while i ~= 0 do                                    "NL
            "        local j : integer = 3                          "NL
            "        while j ~= 0 do                                "NL
            "           local k : integer = 3                       "NL
            "           while k ~= 0 do                             "NL
            "               if k > 0 then                           "NL
            "               elseif k > 2 then                       "NL
            "               else                                    "NL
            "                   break                               "NL
            "                   write(\"Smth after break in else\") "NL
            "               end                                     "NL
            "               write(\"hello\")                        "NL
            "               k = k - 1                               "NL
            "           end                                         "NL
            "           break                                       "NL
            "           j = j - 1                                   "NL
            "        end                                            "NL
            "        i = i - 2                                      "NL
            "    end                                                "NL
            "end                                                    "NL
            "main(3)                                                "NL
    );

    char *description76 = "while cycle. Break cannot be without a cycle";
    int retcode76 = ERROR_SYNTAX;
    pfile_t *pf76 = Pfile.ctor(
            PROLOG
            "function main() "NL
            "   break        "NL
            "end             "NL
    );

    char *description77 = "while cycle. Break cannot be without a cycle #1";
    int retcode77 = ERROR_SYNTAX;
    pfile_t *pf77 = Pfile.ctor(
            PROLOG
            "function main()            "NL
            "   local a : integer = 10  "NL
            "   if a == 10 then         "NL
            "       break               "NL
            "   end                     "NL
            "end                        "NL
    );

    char *description78 = "while cycle. Break cannot be without a cycle #2";
    int retcode78 = ERROR_SYNTAX;
    pfile_t *pf78 = Pfile.ctor(
            PROLOG
            "function main()            "NL
            "   local a : integer = 10  "NL
            "   if a ~= 10 then         "NL
            "   elseif a == 10 then     "NL
            "       break               "NL
            "   end                     "NL
            "end                        "NL
    );

    char *description79 = "while cycle. Break cannot be without a cycle #3";
    int retcode79 = ERROR_SYNTAX;
    pfile_t *pf79 = Pfile.ctor(
            PROLOG
            "function main()            "NL
            "   local a : integer = 10  "NL
            "   if a ~= 10 then         "NL
            "   elseif a == 10 then     "NL
            "   else                    "NL
            "       break               "NL
            "   end                     "NL
            "end                        "NL
    );

    char *description80 = "for cycles, minus in the assignment";
    int retcode80 = ERROR_NOERROR;
    pfile_t *pf80 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    for i=0, -iterations, -1 do          "NL
            "       write(\"I'm gonna be printed\\n\")"NL
            "        for j=0, i-iterations do         "NL
            "            write(\"hello, n times\")    "NL
            "        end                              "NL
            "    end                                  "NL
            "end                                      "NL
            "main(3)                                  "NL
    );

    char *description81 = "minus with string";
    int retcode81 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf81 = Pfile.ctor(
            PROLOG
            "function main(iterations : string )      "NL
            "    for i=0, -iterations, -1 do          "NL
            "    end                                  "NL
            "end                                      "NL
            "main(\"helloo\")                         "NL
    );

    char *description82 = "for cycles, assignment in the condition.";
    int retcode82 = ERROR_SYNTAX;
    pfile_t *pf82 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    local b : number                     "NL
            "    for i=0, b = 10, -1 do               "NL
            "    end                                  "NL
            "end                                      "NL
            "main(\"helloo\")                         "NL
    );

    char *description83 = "for cycles, assignment in the assignment part.";
    int retcode83 = ERROR_SYNTAX;
    pfile_t *pf83 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    local a : integer = 10               "NL
            "    for i=0, 10, a = 20 do               "NL
            "    end                                  "NL
            "end                                      "NL
            "main(\"helloo\")                         "NL
    );

    char *description84 = "for cycles, type mismatch in the condition.";
    int retcode84 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf84 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    local a : integer = 10               "NL
            "    for i=0, true, 20 do                 "NL
            "    end                                  "NL
            "end                                      "NL
            "main(\"helloo\")                         "NL
    );

    char *description85 = "for cycles, type mismatch in the assignment.";
    int retcode85 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf85 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    local a : integer = 10               "NL
            "    for i=0, 20, true do                 "NL
            "    end                                  "NL
            "end                                      "NL
            "main(\"helloo\")                         "NL
    );

    char *description86 = "for cycles, no error.";
    int retcode86 = ERROR_NOERROR;
    pfile_t *pf86 = Pfile.ctor(
            PROLOG
            "function main(iterations : number )      "NL
            "    local a : integer = 10               "NL
            "    for a=0, 20, 2 do                    "NL
            "       break                             "NL
            "    end                                  "NL
            "end                                      "NL
            "main(#\"helloo\")                        "NL
    );

    char *description87 = "break inside repuntil";
    int retcode87 = ERROR_NOERROR;
    pfile_t *pf87 = Pfile.ctor(
            PROLOG NL
            "function me() :string                                           "NL
            "    return \"the whole project was written by a cat\"           "NL
            "end                                                             "NL

            "function main()                                                 "NL
            " local you : string = \"atata\"                                 "NL
            "     repeat                                                     "NL
            "         repeat                                                 "NL
            "             repeat                                             "NL
            "                 repeat                                         "NL
            "                     repeat                                     "NL
            "                         repeat                                 "NL
            "                             local not_true : string = me()     "NL
            "                             break                              "NL
            "                             write(not_true)                    "NL
            "                         until false                            "NL
            "                     until 1 < 0                                "NL
            "                 until 2 + 2 == 5                               "NL
            "             until false                                        "NL
            "         until true and false                                   "NL
            "     until false or false                                       "NL
            "                                                                "NL
            "    return                                                      "NL
            "end                                                             "NL
            "main()                                                          "NL
    );

    char *description88 = "break inside repuntil";
    int retcode88 = ERROR_NOERROR;
    pfile_t *pf88 = Pfile.ctor(
            PROLOG NL
            "function me() :string                                         "NL
            "    return \"the whole project was written by a cat\"         "NL
            "end                                                           "NL

            "function main()                                               "NL
            "   local you : string = \"atata\"                             "NL
            "   local truth : boolean  = true                              "NL
            "   repeat                                                     "NL
            "       repeat                                                 "NL
            "           repeat                                             "NL
            "               repeat                                         "NL
            "                   repeat                                     "NL
            "                       repeat                                 "NL
            "                           local not_true : string = me()     "NL
            "                           break                              "NL
            "                           write(not_true)                    "NL
            "                       until truth                            "NL
            "                       break                                  "NL
            "                   until truth                                "NL
            "                   break                                      "NL
            "               until truth                                    "NL
            "               break                                          "NL
            "           until truth                                        "NL
            "           break                                              "NL
            "       until truth                                            "NL
            "       break                                                  "NL
            "   until truth                                                "NL
            "                                                              "NL
            "   return                                                     "NL
            "end                                                           "NL
            "main()                                                        "NL
    );

    char *description89 = "lexical error";
    int retcode89 = ERROR_LEXICAL;
    pfile_t *pf89 = Pfile.ctor(
            PROLOG
            "function main()"NL
            "   ;13         "NL
            "end            "NL
    );

    char *description90 = "no errors";
    int retcode90 = ERROR_NOERROR;
    pfile_t *pf90 = Pfile.ctor(
            PROLOG
            "function main()             "NL
            "   local a : number = 90a=2 "NL
            "end                         "NL
    );


    char *description91 = "no errors, braced expressions";
    int retcode91 = ERROR_NOERROR;
    pfile_t *pf91 = Pfile.ctor(
            PROLOG
            "function write_numbers(counter : integer)"NL
            "   while (counter) > 0 do                "NL
            "        write(counter, \"\\n\")          "NL
            "    counter = counter - 1                "NL
            "    end                                  "NL
            "end                                      "NL
    );

    char *description92 = "LISP HAHA";
    int retcode92 = ERROR_NOERROR;
    pfile_t *pf92 = Pfile.ctor(
            PROLOG
            "function write_numbers(counter : integer)   "NL
            "   while (((((counter)))) > 0) do           "NL
            "        write(((((counter)))), \"\\n\")     "NL
            "    counter = (((((counter) - 1) + 2)) - 1) "NL
            "    end                                     "NL
            "end                                         "NL
    );

    char *description93 = "no errors, comments";
    int retcode93 = ERROR_NOERROR;
    pfile_t *pf93 = Pfile.ctor(

            "require \"ifj21\"                                                                            "NL
            "                                                                                             "NL
            "function write_numbers(counter : integer)                                                    "NL
            "    while (counter) > 0 do                                                                   "NL
            "        write(counter, \"\\n\")                                                              "NL
            "        counter = counter - 1                                                                "NL
            "    end                                                                                      "NL
            "end                                                                                          "NL
            "function main()                                                                              "NL
            "    local counter : integer = readi()                                                        "NL
            "                                                                                             "NL
            "    if counter == 0 then                                                                     "NL
            "        write(\"Error\", \" enter\",                                                         "NL
            "              \" another\", \" number\",                                                     "NL
            "              \", because \", 0, \" is wrong\",\"\\n\")                                      "NL
            "        return                                                                               "NL
            "    else                                                                                     "NL
            "        write_numbers(counter)                                                               "NL
            "    end                                                                                      "NL
            "                                                                                             "NL
            "end                                                                                          "NL
            "                                                                                             "NL
            "--[[                                                                                         "NL
            "function main()                                                                              "NL
            "    local counter : integer = readi()                                                        "NL
            "                                                                                             "NL
            "    if counter == 0 then                                                                     "NL
            "        write(\"Error\", \" enter\",                                                         "NL
            "              \" another\", \" number\",                                                     "NL
            "              \", because \", 0, \" is wrong\",\"\\n\")                                      "NL
            "        return                                                                               "NL
            "    else                                                                                     "NL
            "        write_numbers(counter)                                                               "NL
            "    end                                                                                      "NL
            "                                                                                             "NL
            "end                                                                                          "NL
            "]]                                                                                           "NL
            "                                                                                             "NL
            "-- main()                                                                                    "NL
            "                                                                                             "NL
    );

    char *description94 = "wrong parameters";
    int retcode94 = ERROR_DEFINITION;
    pfile_t *pf94 = Pfile.ctor(
            PROLOG
            "global name : function (integer, number, string,                   "NL
            "                        string, number, integer, integer, nil, nil,"NL
            "                        nil, number)                               "NL
            "function name    (a : integer, b : number, c : string, d : nil) end"NL
            "                                                                   "NL
            "function main()                                                    "NL
            "end                                                                "NL
            "                                                                   "NL
            "main()                                                             "NL
    );

    char *description95 = "condition with wrong types. Condition cannot be a string";
    int retcode95 = ERROR_NOERROR;
    pfile_t *pf95 = Pfile.ctor(
            PROLOG
            "function write_numbers(counter : string)   "NL
            "   while counter do                        "NL
            "        write(counter, \"\\n\")            "NL
            "    end                                    "NL
            "end                                        "NL
    );


    char *description96 = "condition with wrong types. Condition cannot be a number";
    int retcode96 = ERROR_NOERROR;
    pfile_t *pf96 = Pfile.ctor(
            PROLOG
            "function write_numbers(counter : string) "NL
            "   while #counter do                     "NL
            "        write(counter, \"\\n\")          "NL
            "    end                                  "NL
            "end                                      "NL
    );

    char *description97 = "condition with wrong types. Condition can be nil";
    int retcode97 = ERROR_NOERROR;
    pfile_t *pf97 = Pfile.ctor(
            PROLOG
            "function write_numbers(counter : nil)    "NL
            "   while counter do                      "NL
            "        write(counter, \"\\n\")          "NL
            "    end                                  "NL
            "end                                      "NL
    );

    char *description98 = "condition with wrong syntax. nil cannot have length";
    int retcode98 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf98 = Pfile.ctor(
            PROLOG
            "function write_numbers(counter : nil)    "NL
            "   while #counter do                     "NL
            "        write(counter, \"\\n\")          "NL
            "    end                                  "NL
            "end                                      "NL
    );

    char *description99 = "repeat_until";
    int retcode99 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf99 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()"NL
            "    write(\"I'm about 2bee printed only once!\")    "NL
            "end                                                 "NL
            "                                                    "NL
            "function yours()                                    "NL
            "   local a : string = \"arst\"                      "NL
            "   repeat                                           "NL
            "       to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()  "NL
            "       a = false                                    "NL
            "   until a                                          "NL
            "end                                                 "NL
            "yours()                                             "NL
    );

    char *description100 = "type errors in expressions.";
    int retcode100 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf100 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()"NL
            "    write(\"I'm about 2bee printed only once!\")    "NL
            "end                                                 "NL
            "                                                    "NL
            "function yours()                                    "NL
            "   local a : string = \"arst\"                      "NL
            "   repeat                                           "NL
            "       to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()  "NL
            "       a = false                                    "NL
            "   until (a + 10)                                   "NL
            "end                                                 "NL
            "yours()                                             "NL
    );

    char *description101 = "type errors in expressions.";
    int retcode101 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf101 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()"NL
            "    write(\"I'm about 2bee printed only once!\")    "NL
            "end                                                 "NL
            "                                                    "NL
            "function yours()                                    "NL
            "   local a : string = \"arst\"  + 10                "NL
            "   repeat                                           "NL
            "       to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()  "NL
            "       a = false                                    "NL
            "   until a                                          "NL
            "end                                                 "NL
            "yours()                                             "NL
    );

    char *description102 = "no error";
    int retcode102 = ERROR_NOERROR;
    pfile_t *pf102 = Pfile.ctor(
            PROLOG
            "function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()                                  "NL
            "    write(\"I'm about 2bee printed only once!\")                                      "NL
            "end                                                                                   "NL
            "                                                                                      "NL
            "function yours()                                                                      "NL
            "   local a : number = #(\"arst\" .. \"arsoi\" .. \"arsot\" .. (\"arst\")) + 10        "NL
            "   repeat                                                                             "NL
            "       to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()                                    "NL
            "       a = nil                                                                        "NL
            "   until a                                                                            "NL
            "end                                                                                   "NL
            "yours()                                                                               "NL
    );

    char *description103 = " no type errors in expressions.";
    int retcode103 = ERROR_NOERROR;
    pfile_t *pf103 = Pfile.ctor(
            PROLOG
            "function one(a : integer) : string                  "NL
            "end                                                 "NL
            "function two() : string                             "NL
            "end                                                 "NL
            "function three() : string                           "NL
            "end                                                 "NL
            "function four() : string                            "NL
            "end                                                 "NL
            "one(#(((two()) .. three()) .. four()))                "NL
    );


    char *description104 = "type errors in expressions.";
    int retcode104 = ERROR_NOERROR;
    pfile_t *pf104 = Pfile.ctor(
            PROLOG
            "function one(a : string) : string                   "NL
            "end                                                 "NL
            "function two() : string                             "NL
            "end                                                 "NL
            "function three() : string                           "NL
            "end                                                 "NL
            "function four() : string                            "NL
            "end                                                 "NL
            "one((((two()) .. three()) .. four()))                 "NL
    );

    char *description105 = "bad string.";
    int retcode105 = ERROR_LEXICAL;
    pfile_t *pf105 = Pfile.ctor(
            PROLOG
            "function one(a : integer) : string                  "NL
            "   local s : string = \"\\x00\"                     "NL
            "end                                                 "NL
    );


    char *description141 = "";
    int retcode141 = ERROR_NOERROR;
    pfile_t *pf141 = Pfile.ctor(
            PROLOG
            "global chuj: function(): string, integer          "NL
            "                                                  "NL
            "function main ()                                  "NL
            "    local id : integer                            "NL
            "    local str_var : string                        "NL
            "    local nil_var : nil                           "NL
            "                                                  "NL
            "    if str_var == nil_var then                    "NL
            "        local len_str : integer = 10              "NL
            "        id = len_str                              "NL
            "	if id == len_str then                          "NL
            "    	    write(\"\\n\")                         "NL
            "	else                                           "NL
            "        end                                       "NL
            "    else end                                      "NL
            "                                                  "NL
            "                                                  "NL
            "    local aaa : integer = 0                       "NL
            "    while str_var == nil do                       "NL
            "                                                  "NL
            "	if id == 20 then                               "NL
            "           str_var, aaa = chuj()                  "NL
            "        else                                      "NL
            "           id = id + 1                            "NL
            "        end                                       "NL
            "    end                                           "NL
            "    write(str_var, aaa, \"\\n\")                  "NL
            "                                                  "NL
            "                                                  "NL
            "    return                                        "NL
            "end                                               "NL
            "                                                  "NL
            "function chuj() : string, integer                 "NL
            "  return \"chuj\", 7                              "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
            "                                                  "NL
    );

    char *description142 = "";
    int retcode142 = ERROR_NOERROR;
    pfile_t *pf142 = Pfile.ctor(
            PROLOG
            "                                                  "NL
            "function chuj(n : integer) : integer              "NL
            "    local a : integer = 0                         "NL
            "    while a < (n*n) do                            "NL
            "        a = n + a * a                             "NL
            "    end                                           "NL
            "    return a                                      "NL
            "end                                               "NL
            "                                                  "NL
            "function main ()                                  "NL
            "    local id : integer = 100000                   "NL
            "                                                  "NL
            "    id = chuj(id)                                 "NL
            "                                                  "NL
            "    write(id, \"\\n\")                            "NL
            "                                                  "NL
            "                                                  "NL
            "    return                                        "NL
            "end                                               "NL
            "                                                  "NL
            "                                                  "NL
            "main()                                            "NL
            "                                                  "NL
    );

    char *description143 = "";
    int retcode143 = ERROR_NOERROR;
    pfile_t *pf143 = Pfile.ctor(
            PROLOG
            "                                                  "NL
            "function chuj(n : integer, s : string) : integer  "NL
            "    local a : integer = 0                         "NL
            "    if s == \"chch\" then                         "NL
            "	    while a < (n*n) do                         "NL
            "		a = n + a * a                              "NL
            "	    end                                        "NL
            "    else                                          "NL
            "        a = 7                                     "NL
            "    end                                           "NL
            "    return a                                      "NL
            "end                                               "NL
            "                                                  "NL
            "function main ()                                  "NL
            "    local id : integer = 100000                   "NL
            "                                                  "NL
            "    id = chuj(id, \"4\")                          "NL
            "                                                  "NL
            "    write(id, \"\\n\")                            "NL
            "                                                  "NL
            "                                                  "NL
            "    return                                        "NL
            "end                                               "NL
            "                                                  "NL
            "                                                  "NL
            "main()                                            "NL

    );

    char *description144 = "";
    int retcode144 = ERROR_NOERROR;
    pfile_t *pf144 = Pfile.ctor(
            PROLOG
            "                                                                       "NL
            "function chuj(n : integer, s : string) : integer                       "NL
            "    local a : integer = 0                                              "NL
            "    while a < (n*n) do                                                 "NL
            "	    a = n + a * a                                                   "NL
            "    end                                                                "NL
            "    return a                                                           "NL
            "end                                                                    "NL
            "                                                                       "NL
            "chuj(0, \"a\")                                                         "NL
            "write(chuj(0, \"a\"), \"\\n\")                                         "NL
            "write(chuj(100, \"a\"), \"\\n\")                                       "NL
            "write(chuj(1000000, \"a\"), \"\\n\")                                   "NL
            "write(chuj(10000000000000000, \"a\"), \"\\n\")                         "NL
            "write(chuj(90000000000000000000000000000000000000, \"a\"), \"\\n\")    "NL
            "write(chuj(220000000000000005665456666666666666666, \"a\"), \"\\n\")   "NL
            "                                                                       "NL
            "                                                                       "NL
    );

    char *description145 = "";
    int retcode145 = ERROR_NOERROR;
    pfile_t *pf145 = Pfile.ctor(
            PROLOG
            "function chuj(n : number, a : number) : number                                 "NL
            "	return a * n + n * a                                                        "NL
            "end                                                                            "NL
            "                                                                               "NL
            "write(chuj(0.48949, 5.5645646 ), \"\\n\")                                      "NL
            "write(chuj(90000000000000000000000000000000000000, 4564.456), \"\\n\")         "NL
            "write(chuj(220000000000000005665456666666666666666, 48997489.894981), \"\\n\") "NL
            "                                                                               "NL
    );

    char *description146 = "";
    int retcode146 = ERROR_NOERROR;
    pfile_t *pf146= Pfile.ctor(
            PROLOG
            "                                                                               "NL
            "function chuj(n : number, a : number) : number                                 "NL
            "	if a == 7 then                                                              "NL
            "        	return 0                                                            "NL
            "  	end                                                                         "NL
            "	return a * n + chuj(n, 7)                                                   "NL
            "end                                                                            "NL
            "                                                                               "NL
            "write(chuj(0.48949, 5.5645646 ), \"\\n\")                                      "NL
            "write(chuj(90000000000000000000000000000000000000, 4564.456), \"\\n\")         "NL
            "write(chuj(220000000000000005665456666666666666666, 48997489.894981), \"\\n\") "NL
    );

    char *description147 = "";
    int retcode147 = ERROR_NOERROR;
    pfile_t *pf147 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local i : integer = 69                        "NL
            "    local r : string = chr(i)                     "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL

    );

    char *description148 = "";
    int retcode148 = ERROR_NOERROR;
    pfile_t *pf148 = Pfile.ctor(
            PROLOG

            "function main()                                   "NL
            "    local j : string                              "NL
            "    local i : integer = 69                        "NL
            "    if i == 69 then                               "NL
            "      local i : number                            "NL
            "    elseif j == nil then                          "NL
            "      local z : string                            "NL
            "    elseif 0 then                                 "NL
            "      local b : number                            "NL
            "    end                                           "NL
            "    local k : integer                             "NL
            "    if 3 == 2 then                                "NL
            "      k = 2                                       "NL
            "    else                                          "NL
            "      k = 5                                       "NL
            "    end                                           "NL
            "    write(i, \"\\n\")                             "NL
            "                                                  "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL



    );

    char *description149 = "";
    int retcode149 = ERROR_NOERROR;
    pfile_t *pf149 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local i : integer                             "NL
            "                                                  "NL
            "    i = 0                                         "NL
            "    while i == 1 do                               "NL
            "      i = 1 + i                                   "NL
            "      while 3 == 2 do                             "NL
            "      	i = 1 + i                                  "NL
            "        local j : integer                         "NL
            "      end                                         "NL
            "    end                                           "NL
            "    while i == 1 do                               "NL
            "      i = 1 + i                                   "NL
            "      while 3 == 2 do                             "NL
            "      	i = 1 + i                                  "NL
            "        local j : integer                         "NL
            "      end                                         "NL
            "    end                                           "NL
            "    write(i,\"\\n\")                              "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL
    );
    char *description150 = "";
    int retcode150 = ERROR_NOERROR;
    pfile_t *pf150 = Pfile.ctor(
            PROLOG
            "function foo(n : integer) : integer               "NL
            "    local f : integer                             "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "    local i : integer = 69                        "NL
            "    foo(i)                                        "NL
            "    local r : string = chr(i)                     "NL
            "    write(r,\"\\n\")                              "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL


    );
    char *description151 = "";
    int retcode151 = ERROR_NOERROR;
    pfile_t *pf151 = Pfile.ctor(
            PROLOG
            "function foo(n : integer) : integer               "NL
            "    local f : integer                             "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "    local i : integer                             "NL
            "    repeat                                        "NL
            "      local i : integer                           "NL
            "    until 1 == 1                                  "NL
            "    repeat                                        "NL
            "      local i : integer                           "NL
            "    until 1 == 1                                  "NL
            "    write(i,\"\\n\")                              "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL
    );
    char *description152 = "";
    int retcode152 = ERROR_NOERROR;
    pfile_t *pf152 = Pfile.ctor(
            PROLOG
            "                                                  "NL
            "function main()                                   "NL
            "    local i : integer                             "NL
            "    write(\"69\\n\")                              "NL
            "    while 1 == 1 do                               "NL
            "      local j : integer                           "NL
            "      write(i,\"\\n\")                            "NL
            "      break                                       "NL
            "      while 3 == 2 do                             "NL
            "        local j : integer                         "NL
            "      break                                       "NL
            "        repeat                                    "NL
            "    write(i,\"\\n\")                              "NL
            "          local j : integer                       "NL
            "          if true then                            "NL
            "          write(i,\"\\n\")                        "NL
            "            local j : integer                     "NL
            "          elseif false then                       "NL
            "            write(i,\"\\n\")                      "NL
            "            local j : integer                     "NL
            "          elseif true then                        "NL
            "            local j : integer                     "NL
            "          else                                    "NL
            "            local j : integer                     "NL
            "      break                                       "NL
            "          end                                     "NL
            "          write(i,\"\\n\")                        "NL
            "      break                                       "NL
            "        until 1 == 1                              "NL
            "      break                                       "NL
            "      end                                         "NL
            "    end                                           "NL
            "    while 1 == 1 do                               "NL
            "      write(i,\"\\n\")                            "NL
            "      while 3 == 2 do                             "NL
            "        local j : integer                         "NL
            "      break                                       "NL
            "      end                                         "NL
            "      break                                       "NL
            "    end                                           "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL
    );
    char *description153 = "";
    int retcode153 = ERROR_NOERROR;
    pfile_t *pf153 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local a : integer = 1                         "NL
            "    local b : number = 3.0 + 4.2                  "NL
            "    local c : string = \"hell\"                   "NL
            "    local d : number = nil                        "NL
            "    d = a + b + 5                                 "NL
            "    write(d,\"\\n\")                              "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL
    );

    char *description154 = "";
    int retcode154 = ERROR_NOERROR;
    pfile_t *pf154 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "  if 1 == 1 then                                  "NL
            "    local i : integer = 1                         "NL
            "    write(i,\"\\n\")                              "NL
            "  end                                             "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description155 = "";
    int retcode155 = ERROR_NOERROR;
    pfile_t *pf155 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    while 1 == 1 do                               "NL
            "	write(\"helo\\n\")                             "NL
            "    for i = 0, 10, 1 do                           "NL
            "	write(\"helo\\n\")                             "NL
            "        for i = 0, 2, 2 do                        "NL
            "	write(\"helo\\n\")                             "NL
            "            if 1 == 6 then                        "NL
            "                local k : integer = 1             "NL
            "            elseif 1 == 2 then                    "NL
            "	write(\"helo\\n\")                             "NL
            "                local j : number                  "NL
            "            elseif 2 == 5 then                    "NL
            "	write(\"helo\\n\")                             "NL
            "                local c : string = \"heyo\"       "NL
            "            else                                  "NL
            "	write(\"helo\\n\")                             "NL
            "                local m : integer = 9             "NL
            "                if 1 == 1 then                    "NL
            "                    local k : integer = 3         "NL
            "                end                               "NL
            "                repeat                            "NL
            "	write(\"helo\\n\")                             "NL
            "                    local z : string = \"good\"   "NL
            "                    break                         "NL
            "                until 1 == 3                      "NL
            "            end                                   "NL
            "            break                                 "NL
            "        end                                       "NL
            "        break                                     "NL
            "    end                                           "NL
            "    break                                         "NL
            "    end                                           "NL
            "	write(\"helo\\n\")                             "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
            "                                                  "NL
    );

    char *description156 = "V??po??et faktori??lu iterativn??";
    int retcode156 = ERROR_NOERROR;
    pfile_t *pf156 = Pfile.ctor(
            PROLOG
            "function main() -- uzivatelska funkce bez parametru "NL
            "  local a : integer                                 "NL
            "  local vysl : integer = 0                          "NL
            "  write(\"Zadejte cislo pro vypocet faktorialu\\n\")"NL
            "  a = readi()                                       "NL
            "  if a == nil then                                  "NL
            "    write(\"a je nil\\n\") return                   "NL
            "  else                                              "NL
            "  end                                               "NL
            "  if a < 0 then                                     "NL
            "    write(\"Faktorial nelze spocitat\\n\")          "NL
            "  else                                              "NL
            "    vysl = 1                                        "NL
            "    while a > 0 do                                  "NL
            "      vysl = vysl * a a = a - 1  -- dva prikazy     "NL
            "    end                                             "NL
            "    write(\"Vysledek je: \", vysl, \"\\n\")         "NL
            "    end                                             "NL
            "  end                                               "NL
            "                                                    "NL
            "  main()  -- prikaz hlavniho tela programu          "NL
            "                                                    "NL

    );
    char *description157 = "";
    int retcode157 = ERROR_NOERROR;
    pfile_t *pf157 = Pfile.ctor(
            PROLOG
            "function factorial(n : integer) : integer                  "NL
            "  local n1 : integer = n - 1                               "NL
            "  if n < 2 then                                            "NL
            "    return 1                                               "NL
            "  else                                                     "NL
            "    local tmp : integer = factorial(n1)                    "NL
            "    return n * tmp                                         "NL
            "  end                                                      "NL
            "end                                                        "NL
            "                                                           "NL
            "function main()                                            "NL
            "  write(\"Zadejte cislo pro vypocet faktorialu: \")        "NL
            "  local a : integer = readi()                              "NL
            "  if a ~= nil then                                         "NL
            "		if a < 0 then                                       "NL
            "			write(\"Faktorial nejde spocitat!\", \"\\n\")   "NL
            "		else                                                "NL
            "			local vysl : integer = factorial(a)             "NL
            "			write(\"Vysledek je \", vysl, \"\\n\")          "NL
            "    end                                                    "NL
            "  else                                                     "NL
            "     write(\"Chyba pri nacitani celeho cisla!\\n\")        "NL
            "  end                                                      "NL
            "end                                                        "NL
            "                                                           "NL
            "main()                                                     "NL
    );


    char *description158 = "";
    int retcode158 = ERROR_NOERROR;
    pfile_t *pf158 = Pfile.ctor(
            PROLOG
            "function hlavni_program(year : integer)           "NL
            "  write(\"Hello from IFJ\", year, \"\\n\")        "NL
            "end                                               "NL
            "                                                  "NL
            "hlavni_program(21)                                "NL
            "hlavni_program(22) -- pozdrav z budoucnosti       "NL
    );

    char *description159 = "";
    int retcode159 = ERROR_NOERROR;
    pfile_t *pf159 = Pfile.ctor(
            PROLOG
            "function hlavni_program(year : integer)           "NL
            "  write(\"Hello from IFJ\", year, \"\\n\")        "NL
            "end                                               "NL
            "                                                  "NL
            "hlavni_program(21)                                "NL
    "hlavni_program(22) -- pozdrav z budoucnosti               "NL


    );

    char *description160 = "";
    int retcode160 = ERROR_NOERROR;
    pfile_t *pf160 = Pfile.ctor(
            PROLOG
            "function main ()                     "NL
            "   local id : integer                "NL
            "   local str_var : string            "NL
            "   local nil_var : nil               "NL
            "   if str_var == nil_var then        "NL
            "       local len_str : integer = 10  "NL
            "       id = len_str                  "NL
            "       if id == len_str then         "NL
            "           write(\"\\n\")            "NL
            "       else                          "NL
            "       end                           "NL
            "  else end                           "NL
            "return                               "NL
            "end                                  "NL
            "main()                               "NL

    );

    char *description161 = "";
    int retcode161 = ERROR_NOERROR;
    pfile_t *pf161 = Pfile.ctor(
            PROLOG
            "function main ()                   "NL
            "   local id : integer              "NL
            "   local str_var : string          "NL
            "   local nil_var : nil             "NL
            "   if str_var == nil_var then      "NL
            "   local len_str : integer = 10    "NL
            "   id = len_str                    "NL
            "               else end            "NL
            " return                            "NL
            "   end                             "NL
            " main()                            "NL
    );

    char *description162 = "";
    int retcode162 = ERROR_NOERROR;
    pfile_t *pf162 = Pfile.ctor(
            PROLOG
            "function main ()                               "NL
            "   local id : integer                          "NL
            "   local str_var : string                      "NL
            "   local nil_var : nil                         "NL
            "   if str_var == nil_var then                  "NL
            "       local len_str : integer = 10            "NL
            "       id = len_str                            "NL
            "       if id == len_str then                   "NL
            "           write(len_str, \"\\n\" , len_str)   "NL
            "       else                                    "NL
            "       end                                     "NL
            "   else end                                    "NL
            "   return                                      "NL
            "end                                            "NL
            "main()                                         "NL
    );

    char *description163 = "";
    int retcode163 = ERROR_NOERROR;
    pfile_t *pf163 = Pfile.ctor(
            PROLOG
            "function main ()                               "NL
            "   local id : integer                          "NL
            "   local str_var : string                      "NL
            "   local nil_var : nil                         "NL
            "   if str_var == nil_var then                  "NL
            "       local len_str : integer = 10            "NL
            "       id = len_str                            "NL
            "       if id == len_str then                   "NL
            "           write(len_str, \"\\n\" , len_str)   "NL
            "       else                                    "NL
            "       end                                     "NL
            "   else end                                    "NL
            "   return                                      "NL
            "end                                            "NL
            "main()                                         "NL
    );

    char *description164 = "";
    int retcode164 = ERROR_NOERROR;
    pfile_t *pf164 = Pfile.ctor(
            PROLOG
            "function chuj() : string                                   "NL
            "  return \"chuj\"                                          "NL
            "end                                                        "NL
            "                                                           "NL
            "                                                           "NL
            "function main ()                                           "NL
            "    local id : integer                                     "NL
            "    local str_var : string                                 "NL
            "    local nil_var : nil                                    "NL
            "                                                           "NL
            "    if str_var == nil_var then                             "NL
            "        local len_str : integer = 10                       "NL
            "        id = len_str                                       "NL
            "	if id == len_str then                                   "NL
            "    	    write(\"\\n\")                                  "NL
            "	else                                                    "NL
            "        end                                                "NL
            "    else end                                               "NL
            "                                                           "NL
            "                                                           "NL
            "    while str_var == nil do                                "NL
            "	if id == 20 then                                        "NL
            "           str_var = chuj()                                "NL
            "        else                                               "NL
            "           id = id + 1                                     "NL
            "        end                                                "NL
            "    end                                                    "NL
            "    write(str_var, \"\\n\")                                "NL
            "                                                           "NL
            "    return                                                 "NL
            "end                                                        "NL
            "                                                           "NL
            "main()                                                     "NL
            "                                                           "NL
            "                                                           "NL
    );

    char *description165 = "";
    int retcode165 = ERROR_NOERROR;
    pfile_t *pf165 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string   "NL
	        "       return 1, \"hi\"            "NL
            "end                                "NL
            "                                   "NL
            "function main()                    "NL
            "	local i : integer               "NL
            "	i = foo()                       "NL
            "	write(i)                        "NL
            "end                                "NL
            "                                   "NL
            "main()                             "NL
            "                                   "NL
    );

    char *description166 = "";
    int retcode166 = ERROR_NOERROR;
    pfile_t *pf166 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string  "NL
            "    return 1, \"hi\"              "NL
            "end                               "NL
            "                                  "NL
            "function main()                   "NL
            "	 local i : integer             "NL
            "    local s : string              "NL
            "    i, s = foo()                  "NL
            "	 write(i, s)                   "NL
            "end                               "NL
            "                                  "NL
            "main()                            "NL
            "                                  "NL
    );

    char *description167 = "";
    int retcode167 = ERROR_NOERROR;
    pfile_t *pf167 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                  "NL
            "	return 1, \"hi\"                               "NL
            "end                                               "NL
            "                                                  "NL
            "foo()                                             "NL
    );

    char *description168 = "";
    int retcode168 = ERROR_NOERROR;
    pfile_t *pf168 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                  "NL
            "	return 1, \"hi\"                               "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	local i : integer                              "NL
            "    local s : string = \"xd\"                     "NL
            "    local j : number                              "NL
            "                                                  "NL
            "    i, s, j = foo(), \"hello\", 2.0               "NL
            "	write(i, s, j)                                 "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description169 = "";
    int retcode169 = ERROR_NOERROR;
    pfile_t *pf169 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                  "NL
            "	return 1, \"hi\"                               "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	 local i : integer                             "NL
            "    local s : string = \"xd\"                     "NL
            "    local j : number                              "NL
            "                                                  "NL
            "    j, i, s = 1.1, foo()                          "NL
            " 	 write(i, s, j)                                "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description170 = "";
    int retcode170 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf170 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                  "NL
            "	return 1, \"hi\"                               "NL
            "end                                               "NL
            "                                                  "NL
            "function main() : integer                         "NL
            "	return foo()                                   "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description171 = "";
    int retcode171 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf171 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                  "NL
            "	return 1, \"hi\"                               "NL
            "end                                               "NL
            "                                                  "NL
            "function main() : integer, integer                "NL
            "	return 1, foo()                                "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description172 = "";
    int retcode172 = ERROR_NOERROR;
    pfile_t *pf172 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                  "NL
            "	return 1, \"hi\"                               "NL
            "end                                               "NL
            "                                                  "NL
            "function main() : integer, integer, string        "NL
            "	return 1, foo()                                "NL
            "end                                               "NL
            "                                                  "NL
            "function bar()                                    "NL
            "    write(main())                                 "NL
            "end                                               "NL
            "                                                  "NL
            "bar()                                             "NL
    );

    char *description173 = "";
    int retcode173 = ERROR_NOERROR;
    pfile_t *pf173 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                       "NL
            "	return 1, \"hi\"                                    "NL
            "end                                                    "NL
            "                                                       "NL
            "function main() : integer, integer, string             "NL
            "	return 1, foo()                                     "NL
            "end                                                    "NL
            "                                                       "NL
            "function buzz(i : integer, j : integer, s : string)    "NL
            "    write(i, j, s)                                     "NL
            "end                                                    "NL
            "                                                       "NL
            "function bar()                                         "NL
            "    buzz(main())                                       "NL
            "    write(main())                                      "NL
            "end                                                    "NL
            "                                                       "NL
            "bar()                                                  "NL
    );

    char *description174 = "";
    int retcode174 = ERROR_NOERROR;
    pfile_t *pf174 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string                  "NL
            "	return 1, \"hi\"                               "NL
            "end                                               "NL
            "                                                  "NL
            "function main() : integer, integer, string        "NL
            "	return 1, foo()                                "NL
            "end                                               "NL
            "                                                  "NL
            "function buzz(i : integer)                        "NL
            "    write(i)                                      "NL
            "end                                               "NL
            "                                                  "NL
            "function bar()                                    "NL
            "    buzz(1)                                       "NL
            "    write(main())                                 "NL
            "end                                               "NL
            "                                                  "NL
            "bar()                                             "NL
    );

    char *description175 = "";
    int retcode175 = ERROR_NOERROR;
    pfile_t *pf175 = Pfile.ctor(
            PROLOG
            "function main() : integer, integer, string        "NL
            "	return 1                                       "NL
            "end                                               "NL
            "                                                  "NL
            "function bar()                                    "NL
            "    local i : integer                             "NL
            "    local j : number                              "NL
            "                                                  "NL
            "    i, j = 2, main()                              "NL
            "    write(i, j)                                   "NL
            "end                                               "NL
            "                                                  "NL
            "bar()                                             "NL
    );

    char *description176 = "";
    int retcode176 = ERROR_NOERROR;
    pfile_t *pf176 = Pfile.ctor(
            PROLOG
            "function main() : integer, integer, string        "NL
            "	return 1, 3, \"hi\"                            "NL
            "end                                               "NL
            "                                                  "NL
            "function bar()                                    "NL
            "    local i : integer                             "NL
            "    local j : number                              "NL
            "    local k : integer                             "NL
            "                                                  "NL
            "    i, j, k = 2, main()                           "NL
            "    write(i, j, k)                                "NL
            "end                                               "NL
            "                                                  "NL
            "bar()                                             "NL
    );

    char *description177 = "";
    int retcode177 = ERROR_NOERROR;
    pfile_t *pf177 = Pfile.ctor(
            PROLOG
            "function foo(i : integer, n: number)              "NL
            "    if (i == n) then                              "NL
            "        write(\"i == n\")                         "NL
            "    elseif (i < n) then                           "NL
            "        write(\"i < n\")                          "NL
            "    else                                          "NL
            "        write(\"i > n\")                         "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "    foo(1, 2.4)                                   "NL
            "    foo(35, 34.5)                                 "NL
            "    foo(2, 2)                                     "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description178 = "";
    int retcode178 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf178 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local i : integer = 0                         "NL
            "    write(i)                                      "NL
            "    repeat                                        "NL
            "        local i : integer = 1                     "NL
            "        write(i)                                  "NL
            "        repeat                                    "NL
            "            local i : integer = 2                 "NL
            "            write(i)                              "NL
            "            while i == 1 - (1 * 2 - 1) do         "NL
            "                local i : integer = 3             "NL
            "                write(i)                          "NL
            "                break                             "NL
            "            end                                   "NL
            "        until true                                "NL
            "    until (false or true)                         "NL
            "    repeat                                        "NL
            "            while false do                        "NL
            "                local i : integer = 4             "NL
            "                write(i)                          "NL
            "            end                                   "NL
            "        local i : integer = 5                     "NL
            "        write(i)                                  "NL
            "    until (false or (true or false))              "NL
            "    while true do                                 "NL
            "        local i : integer = 6                     "NL
            "        while i - 2 * 3 and true do               "NL
            "            local i : integer = 7                 "NL
            "            write(i)                              "NL
            "            break                                 "NL
            "        end                                       "NL
            "        break                                     "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description179 = "";
    int retcode179 = ERROR_NOERROR;
    pfile_t *pf179 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local i : integer = 0                         "NL
            "    write(i)                                      "NL
            "    repeat                                        "NL
            "        local i : integer = 1                     "NL
            "        write(i)                                  "NL
            "    until (false or true)                         "NL
            "    write(i)                                      "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description180 = "";
    int retcode180 = ERROR_NOERROR;
    pfile_t *pf180 = Pfile.ctor(
            PROLOG
            "function main ()                                                                              "NL
            "    local i : number = (162.28 * 22 - (#(\"hello\" .. \" world\") // tointeger(23.42)) - 3)   "NL
            "    if i == (162.28 * 22 - #(\"hello\" .. \" world\") // tointeger(23.42) - 3) then           "NL
            "        write(\"success\")                                                                    "NL
            "    else                                                                                      "NL
            "        write(\"fail\")                                                                       "NL
            "    end                                                                                       "NL
            "end                                                                                           "NL
            "                                                                                              "NL
            "main()                                                                                        "NL
    );

    char *description181 = "";
    int retcode181 = ERROR_NOERROR;
    pfile_t *pf181 = Pfile.ctor(
            PROLOG
            "function foo(a : integer, b : string) : number, integer, string    "NL
            "    local k : integer = a + 3                                      "NL
            "    return 1, k, b                                                 "NL
            "end                                                                "NL
            "                                                                   "NL
            "function bar (ah: number, jo: integer, ne: string)                 "NL
            "    write(ah, jo, ne)                                                      "NL
            "end                                                                "NL
            "                                                                   "NL
            "function main()                                                    "NL
            "    local i : integer = 0                                          "NL
            "    i = 2 + 4 + 43                                                 "NL
            "    write(i)                                                       "NL
            "                                                                   "NL
            "    local x : number                                               "NL
            "    local y : integer                                              "NL
            "    local z : string                                               "NL
            "    x, y, z = foo(3, \"hey\")                                      "NL
            "    bar(x, y, z)                                                   "NL
            "end                                                                "NL
            "main()                                                             "NL
    );

    char *description182 = "";
    int retcode182 = ERROR_NOERROR;
    pfile_t *pf182 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local i : integer                             "NL
            "    while 1 == 1 do                               "NL
            "    for i = 0, 10, 1 do                           "NL
            "        for i = 0, 2, 2 do                        "NL
            "            if 1 == 6 then                        "NL
            "                i = 1                             "NL
            "            elseif 1 == 2 then                    "NL
            "                i = 2                             "NL
            "            elseif 2 == 5 then                    "NL
            "                i = 3                             "NL
            "            else                                  "NL
            "                i = 9                             "NL
            "                if 1 == 1 then                    "NL
            "                    i = 4                         "NL
            "                end                               "NL
            "                repeat                            "NL
            "                    i = 42                        "NL
            "                    break                         "NL
            "                until 1 == 3                      "NL
            "            end                                   "NL
            "            break                                 "NL
            "        end                                       "NL
            "        break                                     "NL
            "    end                                           "NL
            "    break                                         "NL
            "    end                                           "NL
            "    write(i)                                      "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description183 = "";
    int retcode183 = ERROR_NOERROR;
    pfile_t *pf183 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local i : integer = 1                         "NL
            "    for i = 0, 10, 2 do                           "NL
            "        i = 5                                     "NL
            "    end                                           "NL
            "    write(i)                                      "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description184 = "";
    int retcode184 = ERROR_NOERROR;
    pfile_t *pf184 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    for j = 0, 10, 2 do                           "NL
            "        write(j)                                  "NL
            "        j = 5                                     "NL
            "        write(j)                                  "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description185 = "";
    int retcode185 = ERROR_NOERROR;
    pfile_t *pf185 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    for j = 0, 10, 2 do                           "NL
            "        j = 5                                     "NL
            "        write(j)                                  "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description186 = "";
    int retcode186 = ERROR_NOERROR;
    pfile_t *pf186 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    for j = 0, 10, 2 do                           "NL
            "        write(j)                                  "NL
            "        j = 5                                     "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description187 = "";
    int retcode187 = ERROR_SYNTAX;
    pfile_t *pf187 = Pfile.ctor(
            PROLOG
            "function foo()                                    "NL
            "end                                               "NL
            "                                                  "NL
            "foo(())                                           "NL
    );

    char *description188 = "";
    int retcode188 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf188 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    repeat                                        "NL
            "        local i : integer = 1                     "NL
            "        write(i)                                  "NL
            "        repeat                                    "NL
            "            local i : integer = 2                 "NL
            "            write(i)                              "NL
            "            while i == 1 - (1 * 2 - 1) do         "NL
            "                local i : integer = 3             "NL
            "                write(i)                          "NL
            "                break                             "NL
            "            end                                   "NL
            "        until true                                "NL
            "    until (false or true)                         "NL
            "    repeat                                        "NL
            "            while false do                        "NL
            "                local i : integer = 4             "NL
            "                write(i)                          "NL
            "            end                                   "NL
            "        local i : integer = 5                     "NL
            "        write(i)                                  "NL
            "    until (false or (true or false))              "NL
            "    while true do                                 "NL
            "        local i : integer = 6                     "NL
            "        while i - 2 * 3 and true do               "NL
            "            local i : integer = 7                 "NL
            "            write(i)                              "NL
            "            break                                 "NL
            "        end                                       "NL
            "        break                                     "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description189 = "";
    int retcode189 = ERROR_NOERROR;
    pfile_t *pf189 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    for j = 0, 5 do                               "NL
            "        write(j)                                  "NL
            "        j = 5                                     "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description190 = "";
    int retcode190 = ERROR_NOERROR;
    pfile_t *pf190 = Pfile.ctor(
            PROLOG
            "function main(s : string) : integer               "NL
            "    local new_str : string = s .. s               "NL
            "    local s_len : integer = #s                    "NL
            "    local new_str_len : number = #new_str         "NL
            "                                                  "NL
            "    if (s_len == new_str_len) then                "NL
            "        write(\"nonono\")                         "NL
            "    elseif (s_len > new_str_len) then             "NL
            "        write(\"no chance\")                      "NL
            "    elseif (s_len ~= new_str_len) then            "NL
            "        write(\"not again\")                      "NL
            "    elseif s_len == new_str_len / 2 then          "NL
            "        write(\"finally correct\")                "NL
            "    else                                          "NL
            "        write(\"bad\")                            "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main(\"test\")                                    "NL
    );

    char *description191 = "";
    int retcode191 = ERROR_NOERROR;
    pfile_t *pf191 = Pfile.ctor(
            PROLOG
            "function main(s : string) : integer               "NL
            "    local new_str : string = s .. s               "NL
            "    local s_len : integer = #s                    "NL
            "    local new_str_len : number = #new_str         "NL
            "                                                  "NL
            "    if (s_len == new_str_len) then                "NL
            "        write(\"nonono\")                         "NL
            "    elseif (s_len > new_str_len) then             "NL
            "        write(\"no chance\")                      "NL
            "    elseif (s_len ~= new_str_len) then            "NL
            "        write(\"not again\")                      "NL
            "    elseif s_len == new_str_len / 2 then          "NL
            "        write(\"finally correct\")                "NL
            "    else                                          "NL
            "        write(\"bad\")                            "NL
            "    end                                           "NL
            "end                                               "NL
            "                                                  "NL
            "main(\"s\")                                       "NL

    );

    char *description192 = "";
    int retcode192 = ERROR_NOERROR;
    pfile_t *pf192 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local z : integer = 0                         "NL
            "    repeat                                        "NL
            "            while false do                        "NL
            "                local i : integer = 4             "NL
            "                write(i)                          "NL
            "            end                                   "NL
            "        local i : number = 5 / z                  "NL
            "        write(i)                                  "NL
            "    until (false or (true or false))              "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description193 = "";
    int retcode193 = ERROR_NOERROR;
    pfile_t *pf193 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local z : integer = 0                         "NL
            "    repeat                                        "NL
            "            while false do                        "NL
            "                local i : integer = 4             "NL
            "                write(i)                          "NL
            "            end                                   "NL
            "        local i : number = 5 / z                  "NL
            "        write(i)                                  "NL
            "    until (false or (true or false))              "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description194 = "";
    int retcode194 = ERROR_NOERROR;
    pfile_t *pf194 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local z : integer = 0                         "NL
            "    repeat                                        "NL
            "            while false do                        "NL
            "                local i : integer = 4             "NL
            "                write(i)                          "NL
            "            end                                   "NL
            "        local i : number = 5 // z                 "NL
            "        write(i)                                  "NL
            "    until (false or (true or false))              "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description195 = "";
    int retcode195 = ERROR_NOERROR;
    pfile_t *pf195 = Pfile.ctor(
            PROLOG
            "function main ()                                  "NL
            "    local exp : integer = 4                       "NL
            "    local base : integer = 2                      "NL
            "    local res : integer = 1                       "NL
            "    while exp ~= 0 do                             "NL
            "        res = res * base                          "NL
            "        exp = exp - 1                             "NL
            "    end                                           "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description196 = "";
    int retcode196 = ERROR_NOERROR;
    pfile_t *pf196 = Pfile.ctor(
            PROLOG
            "function foo(a : integer, b : string) : integer, integer, integer  "NL
            "    return 1, 2, 3                                                 "NL
            "end                                                                "NL
            "                                                                   "NL
            "function main()                                                    "NL
            "    local i : integer                                              "NL
            "    local j : integer                                              "NL
            "    local k : integer                                              "NL
            "    local l : integer                                              "NL
            "    i, j, k, l = 0, foo(1, \"hello\")                              "NL
            "    write(j)                                                       "NL
            "                                                                   "NL
            "end                                                                "NL
            "main()                                                             "NL
    );

    char *description197 = "";
    int retcode197 = ERROR_SEMANTICS_OTHER;
    pfile_t *pf197 = Pfile.ctor(
            PROLOG
            "function foo(a : integer, b : string) : integer, integer           "NL
            "    return 1, 2                                                    "NL
            "end                                                                "NL
            "                                                                   "NL
            "function main()                                                    "NL
            "    local i : integer                                              "NL
            "    local j : integer                                              "NL
            "    local k : integer                                              "NL
            "    local l : integer                                              "NL
            "    i, j, k, l = 0, foo(1, \"test\"), 3                            "NL
            "    write(j)                                                       "NL
            "                                                                   "NL
            "end                                                                "NL
            "main()                                                             "NL
    );

    char *description198 = "";
    int retcode198 = ERROR_NOERROR;
    pfile_t *pf198 = Pfile.ctor(
            PROLOG
            "function foo(a : integer, b : string) : integer, integer           "NL
            "    return 1, 2                                                    "NL
            "end                                                                "NL
            "                                                                   "NL
            "function main()                                                    "NL
            "    local i : integer                                              "NL
            "    local j : integer                                              "NL
            "    local k : integer                                              "NL
            "    local l : integer                                              "NL
            "    i, j, k, l = 0, 3, foo(1, \"hello\")                           "NL
            "    write(j)                                                       "NL
            "                                                                   "NL
            "end                                                                "NL
            "main()                                                             "NL
    );

    char *description199 = "";
    int retcode199 = ERROR_NOERROR;
    pfile_t *pf199 = Pfile.ctor(
            PROLOG
            "function main()                                                    "NL
            "    local l : integer = #(\"yes\")                                 "NL
            "end                                                                "NL
            "main()                                                             "NL
    );

    char *description200 = "";
    int retcode200 = ERROR_NOERROR;
    pfile_t *pf200 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string                            "NL
            "    local k : integer                             "NL
            "    local m : integer                             "NL
            "    local res : string = substr(str, k, m)        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
            "                                                  "NL
    );

    char *description201 = "";
    int retcode201 = ERROR_NOERROR;
    pfile_t *pf201 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"hey\"                  "NL
            "    local k : integer                             "NL
            "    local m : integer                             "NL
            "    local res : string = substr(str, k, m)        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description202 = "";
    int retcode202 = ERROR_NOERROR;
    pfile_t *pf202 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"hey\"                  "NL
            "    local k : integer                             "NL
            "    local m : integer                             "NL
            "    local res : string = substr(str, k, m)        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description203 = "";
    int retcode203 = ERROR_NOERROR;
    pfile_t *pf203 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"hey\"                  "NL
            "    local k : integer = 3                         "NL
            "    local m : integer = 8                         "NL
            "    local res : string = substr(str, k, m)        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description204 = "";
    int retcode204 = ERROR_NOERROR;
    pfile_t *pf204 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"loremipsumsthidk\"     "NL
            "    local k : integer = 3                         "NL
            "    local m : integer = 5                         "NL
            "    local res : string = substr(str, k, m)        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description205 = "";
    int retcode205 = ERROR_NOERROR;
    pfile_t *pf205 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"loremipsumsthidk\"     "NL
            "    local k : integer = 3                         "NL
            "    local m : integer = 3                         "NL
            "    local res : string = substr(str, k, m)        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description206 = "";
    int retcode206 = ERROR_NOERROR;
    pfile_t *pf206 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"loremipsumsthidk\"     "NL
            "    local k : integer = -3                        "NL
            "    local m : integer = -5                        "NL
            "    local res : string = substr(str, k, m)        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description207 = "";
    int retcode207 = ERROR_NOERROR;
    pfile_t *pf207 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"loremipsumsthidk\"     "NL
            "    local k : integer = -3                        "NL
            "    local res : integer = ord(str, k)             "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description208 = "";
    int retcode208 = ERROR_NOERROR;
    pfile_t *pf208 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"loremipsumsthidk\"     "NL
            "    local k : integer = 3                         "NL
            "    local res : integer = ord(str, k)             "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description209 = "";
    int retcode209 = ERROR_NOERROR;
    pfile_t *pf209 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"loremipsumsthidk\"     "NL
            "    local k : integer = 354                       "NL
            "    local res : integer = ord(str, k)             "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description210 = "";
    int retcode210 = ERROR_NOERROR;
    pfile_t *pf210 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"\"                     "NL
            "    local k : integer = 0                         "NL
            "    local res : integer = ord(str, k)             "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description211 = "";
    int retcode211 = ERROR_NOERROR;
    pfile_t *pf211 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string                            "NL
            "    local k : integer                             "NL
            "    local res : integer = ord(str, k)             "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description212 = "";
    int retcode212 = ERROR_NOERROR;
    pfile_t *pf212 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string                            "NL
            "    local k : integer = 3                         "NL
            "    local res : integer = ord(str, k)             "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description213 = "";
    int retcode213 = ERROR_NOERROR;
    pfile_t *pf213 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"hello\"                "NL
            "    local k : integer                             "NL
            "    local res : integer = ord(str, k)             "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description214 = "";
    int retcode214 = ERROR_NOERROR;
    pfile_t *pf214 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"hello\"                "NL
            "    local k : integer = 3                         "NL
            "    local res : number = ord(str, k)              "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description215 = "";
    int retcode215 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf215 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local str : string = \"hello\"                "NL
            "    local k : number = 3                          "NL
            "    local res : number = ord(str, k)              "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description216 = "";
    int retcode216 = ERROR_NOERROR;
    pfile_t *pf216 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local k : number                              "NL
            "    local res : string = chr(tointeger(k))        "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description217 = "";
    int retcode217 = ERROR_NOERROR;
    pfile_t *pf217 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local k : integer = -2                        "NL
            "    local res : string = chr(k)                   "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description218 = "";
    int retcode218 = ERROR_NOERROR;
    pfile_t *pf218 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local k : integer = 256                       "NL
            "    local res : string = chr(k)                   "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description219 = "";
    int retcode219 = ERROR_NOERROR;
    pfile_t *pf219 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local k : integer = 255                       "NL
            "    local res : string = chr(k)                   "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL

    );

    char *description220 = "";
    int retcode220 = ERROR_NOERROR;
    pfile_t *pf220 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local k : integer = 69                        "NL
            "    local res : string = chr(k)                   "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description221 = "";
    int retcode221 = ERROR_NOERROR;
    pfile_t *pf221 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local k : integer = 0                         "NL
            "    local res : string = chr(k)                   "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description222 = "";
    int retcode222 = ERROR_NOERROR;
    pfile_t *pf222 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local res : integer = tointeger(3.4)          "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description223 = "";
    int retcode223 = ERROR_NOERROR;
    pfile_t *pf223 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local k : integer                             "NL
            "    local res : integer = tointeger(k)            "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description224 = "";
    int retcode224 = ERROR_NOERROR;
    pfile_t *pf224 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local res : integer = tointeger(-02.28)       "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description225 = "";
    int retcode225 = ERROR_NOERROR;
    pfile_t *pf225 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local res : integer = readi()                 "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description226 = "";
    int retcode226 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf226 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local res : integer = readn()                 "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description227 = "";
    int retcode227 = ERROR_NOERROR;
    pfile_t *pf227 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local res : number = readn()                  "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description228 = "";
    int retcode228 = ERROR_NOERROR;
    pfile_t *pf228 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "    local res : string = reads()                  "NL
            "    write(res)                                    "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description229 = "";
    int retcode229 = ERROR_NOERROR;
    pfile_t *pf229 = Pfile.ctor(
            PROLOG
            "function main()                                          "NL
            "    local i : string                                     "NL
            "    i = write(6656,6%87,\"\\n\", \"escape\", \"\\010\")  "NL
            "    write(i)                                             "NL
            "end                                                      "NL
            "main()                                                   "NL
    );

    char *description230 = "";
    int retcode230 = ERROR_NOERROR;
    pfile_t *pf230 = Pfile.ctor(
            PROLOG
            "function modulo(a: integer, b: integer) : integer "NL
            "  local i : integer = a - (a // b) * b            "NL
            "  if (i == (a%b)) then                            "NL
            "    write(\"success\\n\")                         "NL
            "  else                                            "NL
            "    write(\"fail\\n\")                            "NL
            "  end                                             "NL
            "  return i                                        "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "  local mod : number = modulo(42, 23)             "NL
            "  write(mod, \"\\n\")                             "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description231 = "";
    int retcode231 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf231 = Pfile.ctor(
            PROLOG
            "function power(a: number, b: integer) : number    "NL
            "  local i : number = a ^ b                        "NL
            "  return i                                        "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "  local res : number = power(2.0)                 "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description232 = "";
    int retcode232 = ERROR_DEFINITION;
    pfile_t *pf232 = Pfile.ctor(
            PROLOG
            "function main()                                    "NL
            "	local i : integer = 1                           "NL
            "	local j : integer = 2 - 1 * 0 + 1               "NL
            "	if i == j then                                  "NL
            "		local i : string                            "NL
            "	else                                            "NL
            "	end                                             "NL
            "                                                   "NL
            "	local i : number = 3.0                          "NL
            "end                                                "NL
            "                                                   "NL
            "main()                                             "NL
    );

    char *description233 = "";
    int retcode233 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf233 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = \"hello\"                  "NL
            "end                                               "NL
    );

    char *description234 = "";
    int retcode234 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf234 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = 9.9                        "NL
            "end                                               "NL
    );

    char *description235 = "";
    int retcode235 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf235 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = 9.9 + 1.1                  "NL
            "end                                               "NL
    );

    char *description236 = "";
    int retcode236 = ERROR_DEFINITION;
    pfile_t *pf236 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = 9                          "NL
            "	i = 2 + j                                      "NL
            "end                                               "NL
    );

    char *description237 = "";
    int retcode237 = ERROR_DEFINITION;
    pfile_t *pf237 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = 9                          "NL
            "	while true do                                  "NL
            "		local j : integer = 8                      "NL
            "	end                                            "NL
            "	write(i, j)                                    "NL
            "end                                               "NL
    );

    char *description238 = "";
    int retcode238 = ERROR_SYNTAX;
    pfile_t *pf238 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = 9                          "NL
            "	global j : integer                             "NL
            "	write(i, j)                                    "NL
            "end                                               "NL
    );

    char *description239 = "";
    int retcode239 = ERROR_SYNTAX;
    pfile_t *pf239 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = 9                          "NL
            "	else                                           "NL
            "		i = 4                                      "NL
            "	end                                            "NL
            "	write(i)                                       "NL
            "end                                                "NL
    );

    char *description240 = "";
    int retcode240 = ERROR_DEFINITION;
    pfile_t *pf240 = Pfile.ctor(
            PROLOG
            "function abcd()                                   "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	local abcd : integer = 9                       "NL
            "	write(abcd)                                    "NL
            "end                                               "NL
    );

    char *description241 = "";
    int retcode241 = ERROR_DEFINITION;
    pfile_t *pf241 = Pfile.ctor(
            PROLOG
            "function foo()                                    "NL
            "end                                               "NL
            "                                                  "NL
            "function bar()                                    "NL
            "end                                               "NL
            "                                                  "NL
            "function foo()                                    "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "end                                               "NL
    );

    char *description242 = "";
    int retcode242 = ERROR_DEFINITION;
    pfile_t *pf242 = Pfile.ctor(
            PROLOG
            "function foo()                                    "NL
            "	main()                                         "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	write(\"ok\")                                  "NL
            "end                                               "NL
    );

    char *description243 = "";
    int retcode243 = ERROR_DEFINITION;
    pfile_t *pf243 = Pfile.ctor(
            PROLOG
            "global foo : function(string) : integer           "NL
            "                                                  "NL
            "function foo(s: string) : integer                 "NL
            "	return #s                                      "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	write(\"ok\")                                  "NL
            "	foo(\"text\")                                  "NL
            "end                                               "NL
            "                                                  "NL
            "global foo : function(string) : integer           "NL
    );

    char *description244 = "function is declared but not defined";
    int retcode244 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf244 = Pfile.ctor(
            PROLOG
            "global foo : function(string) : integer           "NL
            "                                                  "NL
            "function main()                                   "NL
            "	foo()                                          "NL
            "end                                               "NL
    );

    char *description245 = "";
    int retcode245 = ERROR_DEFINITION;
    pfile_t *pf245 = Pfile.ctor(
            PROLOG
            "global foo : function(string) : integer           "NL
            "                                                  "NL
            "function main()                                   "NL
            "end                                               "NL
    );

    char *description246 = "";
    int retcode246 = ERROR_DEFINITION;
    pfile_t *pf246 = Pfile.ctor(
            PROLOG
            "global foo : function(string) : integer           "NL
            "                                                  "NL
            "function foo() : integer                          "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	foo()                                          "NL
            "end                                               "NL
    );

    char *description247 = "";
    int retcode247 = ERROR_DEFINITION;
    pfile_t *pf247 = Pfile.ctor(
            PROLOG
            "global foo : function(string) : integer           "NL
            "                                                  "NL
            "function foo(s: string) : number                  "NL
            "end                                               "NL
    );

    char *description248 = "";
    int retcode248 = ERROR_DEFINITION;
    pfile_t *pf248 = Pfile.ctor(
            PROLOG
            "global foo : function(string) : integer           "NL
            "                                                  "NL
            "function foo(s: string, i: integer) : integer     "NL
            "end                                               "NL
    );

    char *description249 = "";
    int retcode249 = ERROR_DEFINITION;
    pfile_t *pf249 = Pfile.ctor(
            PROLOG
            "global foo : function(string) : integer           "NL
            "                                                  "NL
            "function main()                                   "NL
            "	foo(\"text\")                                  "NL
            "end                                               "NL
            "                                                  "NL
            "function foo(s: string) : integer, number         "NL
            "	return #s                                      "NL
            "end                                               "NL

    );

    char *description250 = "";
    int retcode250 = ERROR_DEFINITION;
    pfile_t *pf250 = Pfile.ctor(
            PROLOG
            "function substr(a: string) : string               "NL
            "	return a                                       "NL
            "end                                               "NL
    );

    char *description251 = "";
    int retcode251 = ERROR_SYNTAX;
    pfile_t *pf251 = Pfile.ctor(
            PROLOG
            "function foo(a: string) : string                  "NL
            "	local i : number                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "end                                               "NL
    );

    char *description252 = "";
    int retcode252 = ERROR_SYNTAX;
    pfile_t *pf252 = Pfile.ctor(
            PROLOG
            "function foo(a: string) : string                  "NL
            "	local i : number                               "NL
            "                                                  "NL
            "	function main()                                "NL
            "		write(\"bad\")                             "NL
            "	end                                            "NL
            "end                                               "NL
    );

    char *description253 = "";
    int retcode253 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf253 = Pfile.ctor(
            PROLOG
            "function foo(a: string) : string, integer         "NL
            "	local i : number                               "NL
            "	return i                                       "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	print(\"bad\")                                 "NL
            "end                                               "NL
    );

    char *description254 = "";
    int retcode254 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf254 = Pfile.ctor(
            PROLOG
            "function foo(a: string) : string, integer         "NL
            "	local i : number                               "NL
            "	return i                                       "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	local i : integer = foo(\"a\")                 "NL
            "end                                               "NL
    );

    char *description255 = "";
    int retcode255 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf255 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer = \"hello\"                  "NL
            "end                                               "NL
    );

    char *description256 = "";
    int retcode256 = ERROR_TYPE_MISSMATCH;
    pfile_t *pf256 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	local j : integer                              "NL
            "	local k : integer                              "NL
            "	local l : integer                              "NL
            "	local m : integer                              "NL
            "	i, j, k, l, m = 0, 1, 2, 3, 4.5                "NL
            "end                                               "NL
    );

    char *description257 = "";
    int retcode257 = ERROR_SEMANTICS_OTHER;
    pfile_t *pf257 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	local j : integer                              "NL
            "	local k : integer                              "NL
            "	local l : integer                              "NL
            "	local m : integer                              "NL
            "	i, j, k, l, m = 0, 1, 2, 3                     "NL
            "end                                               "NL
    );

    char *description258 = "";
    int retcode258 = ERROR_NOERROR;
    pfile_t *pf258 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	local j : integer                              "NL
            "	local k : integer                              "NL
            "	i, j, k = 0, 1, 2, 3                           "NL
            "end                                               "NL
    );

    char *description259 = "";
    int retcode259 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf259 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	while 1 == \"string\" do                       "NL
            "	end                                            "NL
            "end                                               "NL
    );

    char *description260 = "";
    int retcode260 = ERROR_SYNTAX;
    pfile_t *pf260 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	while 1 == 2.4                                 "NL
            "	end                                            "NL
            "end                                               "NL
    );

    char *description261 = "correct usage of not operator";
    int retcode261 = ERROR_NOERROR;
    pfile_t *pf261 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local a : integer = 5                          "NL
            "   if not(2 < a) then                             "NL
            "       write(\"something wrong\\n\")              "NL
            "   else                                           "NL
            "       write(\"correct\")                         "NL
            "	end                                            "NL
            "end                                               "NL
    );

    char *description262 = "unrecognised token in precedence analyze";
    int retcode262 = ERROR_LEXICAL;
    pfile_t *pf262 = Pfile.ctor(
            PROLOG
            "function main()              "NL
            "   local a : integer = 1 @ 2 "NL
            "end                          "NL
    );

    char *description263 = "semantics error with not operator";
    int retcode263 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf263 = Pfile.ctor(
            PROLOG
            "function main()                       "NL
            "   local a : boolean = not \"string\" "NL
            "end                                   "NL
    );

    char *description264 = "precedence error in expression";
    int retcode264 = ERROR_SYNTAX;
    pfile_t *pf264 = Pfile.ctor(
            PROLOG
            "function main()             "NL
            "   local a : integer = 1 2  "NL
            "end                         "NL
    );

    char *description265 = "call a function without any params";
    int retcode265 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf265 = Pfile.ctor(
            PROLOG
            "function foo() : integer, string         "NL
            "   return 22, \"test\"                   "NL
            "end                                      "NL
            "                                         "NL
            "function main(a : integer, b : string)   "NL
            "   write(a)                              "NL
            "end                                      "NL
            "                                         "NL
            "main(foo(1, 2, 3))                       "NL
    );

    char *description266 = "precedence analyse error in return stmt";
    int retcode266 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf266 = Pfile.ctor(
            PROLOG
            "function foo() : integer, number, integer     "NL
            "   local bar : number = 25.6                  "NL
            "   return 1, bar, # # \"text\"                "NL
            "end                                           "NL
    );

    char *description267 = "undefined variable in multiple assignment";
    int retcode267 = ERROR_DEFINITION;
    pfile_t *pf267 = Pfile.ctor(
            PROLOG
            "function foo(a : boolean) : string, integer   "NL
            "   if a then                                  "NL
            "      return \"true\", 112                    "NL
            "   end                                        "NL
            "                                              "NL
            "   return \"false\", -112                     "NL
            "end                                           "NL
            "                                              "NL
            "function main()                               "NL
            "   local b : string                           "NL
            "   local c : integer                          "NL
            "   a, b, c = 2.0, foo()                       "NL
            "end                                           "NL
            "                                              "NL
            "main()                                        "NL
    );

    char *description268 = "wrong expression in multiple assignment #1";
    int retcode268 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf268 = Pfile.ctor(
            PROLOG
            "function main()                                "NL
            "   local a : integer                           "NL
            "   local b : integer                           "NL
            "   a, b = 6.0 // 2.0, 2                        "NL
            "end                                            "NL
    );

    char *description269 = "wrong expression in multiple assignment #2";
    int retcode269 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf269 = Pfile.ctor(
            PROLOG
            "function main()                                "NL
            "   local a : boolean                           "NL
            "   local b : integer                           "NL
            "   local c : string                            "NL
            "   a, b, c = true, 2, not \"text\"             "NL
            "end                                            "NL
    );

    char *description270 = "call undefined function in global scope";
    int retcode270 = ERROR_DEFINITION;
    pfile_t *pf270 = Pfile.ctor(
            PROLOG
            "foo(1, 2, 3)"NL
    );

    char *description271 = "call undefined function inside other function";
    int retcode271 = ERROR_DEFINITION;
    pfile_t *pf271 = Pfile.ctor(
            PROLOG
            "function main()                     "NL
            "   foo(1, 2, 3)                     "NL
            "end                                 "NL
    );

    char *description272 = "multiple unary minuses";
    int retcode272 = ERROR_NOERROR;
    pfile_t *pf272 = Pfile.ctor(
            PROLOG
            "function main() : integer                         "NL
            "   local a : integer = 1 + - - - - - 3            "NL
            "   return a                                       "NL
            "end                                               "NL
            "write(main())                                     "NL
    );

    char *description273 = "expression with function which does not return anything";
    int retcode273 = ERROR_NOERROR;
    pfile_t *pf273 = Pfile.ctor(
            PROLOG
            "function foo()                           "NL
            "   write(\"hello\")                      "NL
            "end                                      "NL
            "                                         "NL
            "function main()                          "NL
            "   local a : boolean = foo() == nil      "NL
            "   if a then                             "NL
            "       write(\"yes\")                    "NL
            "   else                                  "NL
            "       write(\"no\")                     "NL
            "   end                                   "NL
            "end                                      "NL
            "main()                                   "NL
    );

    char *description274 = "strcat with wrong types";
    int retcode274 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf274 = Pfile.ctor(
            PROLOG
            "function main()                       "NL
            "   local str : string = true .. 123   "NL
            "end                                   "NL
    );

    char *description275 = "boolean subtraction";
    int retcode275 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf275 = Pfile.ctor(
            PROLOG
            "function main()                       "NL
            "   local a : boolean = true - false   "NL
            "end                                   "NL
    );

    char *description276 = "wrong if statement";
    int retcode276 = ERROR_SYNTAX;
    pfile_t *pf276 = Pfile.ctor(
            PROLOG
            "function main()                       "NL
            "   local a : boolean                  "NL
            "   if 1 == 2 then                     "NL
            "       a = true                       "NL
            "end                                   "NL
    );

    char *description277 = "unrecognized variable type";
    int retcode277 = ERROR_SYNTAX;
    pfile_t *pf277 = Pfile.ctor(
            PROLOG
            "function main()                       "NL
            "   local b : float = 12.337           "NL
            "end                                   "NL
    );

    char *description278 = "wrong type of parameter in function definition #1";
    int retcode278 = ERROR_SYNTAX;
    pfile_t *pf278 = Pfile.ctor(
            PROLOG
            "function main(a : float)              "NL
            "   write(\"i parsed type float :/\")  "NL
            "end                                   "NL
    );

    char *description279 = "wrong type of parameter in function definition #2";
    int retcode279 = ERROR_SYNTAX;
    pfile_t *pf279 = Pfile.ctor(
            PROLOG
            "function main(a : number, b : float)              "NL
            "   write(\"i parsed type float :/\")              "NL
            "end                                               "NL
    );

    char *description280 = "wrong parameters in function definition";
    int retcode280 = ERROR_SYNTAX;
    pfile_t *pf280 = Pfile.ctor(
            PROLOG
            "function main(a : number, b : number,)            "NL
            "   local b : integer = nil                        "NL
            "end                                               "NL
    );

    char *description281 = "unrecognized symbol in function statement";
    int retcode281 = ERROR_LEXICAL;
    pfile_t *pf281 = Pfile.ctor(
            PROLOG
            "function main()                       "NL
            "   @                                  "NL
            "end                                   "NL
    );

    char *description282 = "error in function declaration parameters #1";
    int retcode282 = ERROR_SYNTAX;
    pfile_t *pf282 = Pfile.ctor(
            PROLOG
            "global foo : function(float, string)"NL
    );

    char *description283 = "error in function declaration parameters #2";
    int retcode283 = ERROR_SYNTAX;
    pfile_t *pf283 = Pfile.ctor(
            PROLOG
            "global foo : function(number, integer, float)"NL
    );

    char *description284 = "error in function declaration rets #1";
    int retcode284 = ERROR_SYNTAX;
    pfile_t *pf284 = Pfile.ctor(
            PROLOG
            "global foo : function() : float, string"NL
    );

    char *description285 = "error in function declaration rets #2";
    int retcode285 = ERROR_SYNTAX;
    pfile_t *pf285 = Pfile.ctor(
            PROLOG
            "global foo : function() : integer, number, float"NL
    );

    char *description286 = "function declaration wrong rets";
    int retcode286 = ERROR_SYNTAX;
    pfile_t *pf286 = Pfile.ctor(
            PROLOG
            "global foo : function() : integer,"NL
    );

    char *description287 = "function declaration wrong params";
    int retcode287 = ERROR_SYNTAX;
    pfile_t *pf287 = Pfile.ctor(
            PROLOG
            "global foo : function(string, number,)"NL
    );

    char *description288 = "equality of incompatible types";
    int retcode288 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf288 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	if true == \"a\" then                          "NL
            "	else                                           "NL
            "	end                                            "NL
            "end                                               "NL
    );

    char *description289 = "equality of boolean and nil";
    int retcode289 = ERROR_NOERROR;
    pfile_t *pf289 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	if true == nil then                            "NL
            "		write(\"false\")                           "NL
            "	else                                           "NL
            "		write(\"true\")                            "NL
            "	end                                            "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description290 = "more else";
    int retcode290 = ERROR_SYNTAX;
    pfile_t *pf290 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	if true == nil then                            "NL
            "		write(\"false\")                           "NL
            "	else                                           "NL
            "		write(\"true\")                            "NL
            "	else                                           "NL
            "		write(\"ba                                 "NL
            "	end                                            "NL
            "end                                               "NL
    );

    char *description291 = "more if";
    int retcode291 = ERROR_SYNTAX;
    pfile_t *pf291 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	if true == nil then                            "NL
            "		i = 0                                      "NL
            "	if false == nil then                           "NL
            "		i = 1                                      "NL
            "	else                                           "NL
            "		i = 2                                      "NL
            "	end                                            "NL
            "	write(i)                                       "NL
            "end                                               "NL
    );

    char *description292 = "nested if";
    int retcode292 = ERROR_NOERROR;
    pfile_t *pf292 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	if true == nil then                            "NL
            "		i = 0                                      "NL
            "	if false == nil then                           "NL
            "		i = 1                                      "NL
            "	else                                           "NL
            "		i = 2                                      "NL
            "	end                                            "NL
            "	end                                            "NL
            "	write(i)                                       "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description293 = "elseif without if";
    int retcode293 = ERROR_SYNTAX;
    pfile_t *pf293 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	elseif 1 + 4 + 334 - 3 then                    "NL
            "		i = 10                                     "NL
            "	if true == nil then                            "NL
            "		i = 0                                      "NL
            "	if false == nil then                           "NL
            "		i = 1                                      "NL
            "	else                                           "NL
            "		i = 2                                      "NL
            "	end                                            "NL
            "	end                                            "NL
            "	print(i)                                       "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description294 = "undeclared variable in loop";
    int retcode294 = ERROR_DEFINITION;
    pfile_t *pf294 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	while i <= 10 do                               "NL
            "		local i : integer                          "NL
            "		print(i)                                   "NL
            "	end                                            "NL
            "end                                               "NL
    );

    char *description295 = "wrong number of params in function call";
    int retcode295 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf295 = Pfile.ctor(
            PROLOG
            "function bar (a: string, b: integer, c : number, d : string)   "NL
            "end                                                            "NL
            "                                                               "NL
            "function main()                                                "NL
            "	bar(\"hello\", 1, 4, \"world\", 2)                          "NL
            "end                                                            "NL
    );

    char *description296 = "wrong number of params in function call";
    int retcode296 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf296 = Pfile.ctor(
            PROLOG
            "function bar (a: string, b: integer, c : number, d : string)   "NL
            "end                                                            "NL
            "                                                               "NL
            "function main()                                                "NL
            "	bar(\"hello\", 1, 4)                                        "NL
            "end                                                            "NL
    );

    char *description297 = "wrong param types in function call";
    int retcode297 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf297 = Pfile.ctor(
            PROLOG
            "function bar (a: string, b: integer, d : string)  "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	bar(\"hello\", 1.4, \"s\")                     "NL
            "end                                               "NL
    );

    char *description298 = "wrong param types in function call";
    int retcode298 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf298 = Pfile.ctor(
            PROLOG
            "function bar (a: string, b: integer, c : number)  "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	bar(\"hello\", 1.4, 4)                         "NL
            "end                                               "NL
    );

    char *description299 = "fewer ids than return values";
    int retcode299 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf299 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number       "NL
            "	return 1, 2                                    "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	local j : number                               "NL
            "	local k : string                               "NL
            "	i, j, k = bar(4)                               "NL
            "end                                               "NL
    );

    char *description300 = "more ids than return values";
    int retcode300 = ERROR_NOERROR;
    pfile_t *pf300 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, integer   "NL
            "	return 1, 2, 3                                      "NL
            "end                                                    "NL
            "                                                       "NL
            "function main()                                        "NL
            "	local i : integer                                   "NL
            "	local j : number                                    "NL
            "	i, j = bar(4)                                       "NL
            "	write(i, j)                                         "NL
            "end                                                    "NL
            "                                                       "NL
            "main()                                                 "NL
    );

    char *description301 = "wrong type of id in assignment";
    int retcode301 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf301 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, integer  "NL
            "	return 1, 2, 3                                     "NL
            "end                                                   "NL
            "                                                      "NL
            "function main()                                       "NL
            "	local i : integer                                  "NL
            "	local j : string                                   "NL
            "	i, j = bar(4)                                      "NL
            "end                                                   "NL
    );

    char *description302 = "wrong id type in assignment";
    int retcode302 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf302 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, integer   "NL
            "	return                                              "NL
            "end                                                    "NL
            "                                                       "NL
            "function main()                                        "NL
            "	local i : integer                                   "NL
            "	local j : string                                    "NL
            "	i, j = bar(4)                                       "NL
            "end                                                    "NL
    );

    char *description303 = "";
    int retcode303 = ERROR_NOERROR;
    pfile_t *pf303 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, integer   "NL
            "	write(a)                                            "NL
            "	return a                                            "NL
            "end                                                    "NL
            "                                                       "NL
            "function main()                                        "NL
            "	local i : integer                                   "NL
            "	local j : number                                    "NL
            "	i, j = bar(i)                                       "NL
            "	write(i)                                            "NL
            "end                                                    "NL
            "                                                       "NL
            "main()                                                 "NL
    );

    char *description304 = "wrong number of params in main scope";
    int retcode304 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf304 = Pfile.ctor(
            PROLOG
            "function bar(i : integer) : integer, number       "NL
            "   return i, 2.0                                  "NL
            "end                                               "NL
            "                                                  "NL
            "function main()                                   "NL
            "	local i : integer                              "NL
            "	local j : number                               "NL
            "	i, j = bar(i)                                  "NL
            "	write(i)                                       "NL
            "end                                               "NL
            "                                                  "NL
            "main(8)                                           "NL
    );

    char *description305 = "redeclaration of variable in params";
    int retcode305 = ERROR_DEFINITION;
    pfile_t *pf305 = Pfile.ctor(
            PROLOG
            "function main(i : integer)                        "NL
            "	local i : integer                              "NL
            "	local j : number                               "NL
            "	i, j = bar(i)                                  "NL
            "	write(i)                                       "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description306 = "wrong number of params in main scope";
    int retcode306 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf306 = Pfile.ctor(
            PROLOG
            "function bar(i : integer) : integer, number       "NL
            "   return i, 2.0                                  "NL
            "end                                               "NL
            "                                                  "NL
            "function main(i : integer)                        "NL
            "	local j : number                               "NL
            "	i, j = bar(i)                                  "NL
            "	write(i)                                       "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description307 = "wrong number of values in return";
    int retcode307 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf307 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, integer   "NL
            "	return a, 3, 4, 5                                   "NL
            "end                                                    "NL
            "                                                       "NL
            "function main(i : integer)                             "NL
            "	local j : number                                    "NL
            "	i, j = bar(i)                                       "NL
            "	write(i)                                            "NL
            "end                                                    "NL
    );

    char *description308 = "fewer values in return";
    int retcode308 = ERROR_NOERROR;
    pfile_t *pf308 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, number "NL
            "	write(a)                                         "NL
            "	return a, 3                                      "NL
            "end                                                 "NL
            "                                                    "NL
            "function main(i : integer)                          "NL
            "	local j : number                                 "NL
            "	i, j = bar(i)                                    "NL
            "	write(i)                                         "NL
            "end                                                 "NL
    );

    char *description309 = "expecting nil error 8";
    int retcode309 = ERROR_NOERROR;
    pfile_t *pf309 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, number "NL
            "	while a <= 3 do                                  "NL
            "		write(a)                                     "NL
            "		if (a == 2) then                             "NL
            "			return a                                 "NL
            "		end                                          "NL
            "	end                                              "NL
            "	return a                                         "NL
            "end                                                 "NL
            "                                                    "NL
            "function main(i : integer)                          "NL
            "	local j : number                                 "NL
            "	i, j = bar(i)                                    "NL
            "	write(i)                                         "NL
            "end                                                 "NL
            "                                                    "NL
            "main(nil)                                           "NL
    );

    char *description310 = "";
    int retcode310 = ERROR_NOERROR;
    pfile_t *pf310 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, number "NL
            "	while a <= 3 do                                  "NL
            "		write(a)                                     "NL
            "		if (a == 2) then                             "NL
            "			return a                                 "NL
            "		end                                          "NL
            "	end                                              "NL
            "	return a                                         "NL
            "end                                                 "NL
            "                                                    "NL
            "function main(i : integer)                          "NL
            "	local j : number                                 "NL
            "	i, j = bar(i)                                    "NL
            "	write(i)                                         "NL
            "end                                                 "NL
            "                                                    "NL
            "main(4)                                             "NL
    );

    char *description311 = "variable declaration in main scope";
    int retcode311 = ERROR_SYNTAX;
    pfile_t *pf311 = Pfile.ctor(
            PROLOG
            "function bar(a : integer) : integer, number       "NL
            "   write(a)                                       "NL
            "end                                               "NL
            "function main(i : integer)                        "NL
            "	local j : number                               "NL
            "	i, j = bar(i)                                  "NL
            "end                                               "NL
            "                                                  "NL
            "local i : integer                                 "NL
            "main(i)                                           "NL
    );

    char *description312 = "global variable declaration in main scope";
    int retcode312 = ERROR_SYNTAX;
    pfile_t *pf312 = Pfile.ctor(
            PROLOG
            "function bar(i : integer) : integer, number       "NL
            "   return i, 2.0                                  "NL
            "end                                               "NL
            "                                                  "NL
            "function main(i : integer)                        "NL
            "	local j : number                               "NL
            "	i, j = bar(i)                                  "NL
            "	write(i)                                       "NL
            "end                                               "NL
            "                                                  "NL
            "global i : integer                                "NL
            "main(i)                                           "NL
    );

    char *description313 = "comparing boolean and integer";
    int retcode313 = ERROR_EXPRESSIONS_TYPE_INCOMPATIBILITY;
    pfile_t *pf313 = Pfile.ctor(
            PROLOG
            "function bar (a: integer) : integer, number, number "NL
            "	while a <= 3 do                                  "NL
            "		write(a)                                     "NL
            "		if (false == 2) then                         "NL
            "			return a                                 "NL
            "		end                                          "NL
            "	end                                              "NL
            "	return a                                         "NL
            "end                                                 "NL
            "                                                    "NL
            "function main(i : integer)                          "NL
            "	local j : number                                 "NL
            "	i, j = bar(i)                                    "NL
            "	write(i)                                         "NL
            "end                                                 "NL
            "                                                    "NL
            "main(2)                                             "NL
    );

    char *description314 = "built-in function call wrong returns";
    int retcode314 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf314 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local s : string                               "NL
            "	s = readn()                                    "NL
            "end                                               "NL
    );

    char *description315 = "built-in function call nil error expected (8)";
    int retcode315 = ERROR_NOERROR;
    pfile_t *pf315 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : number                               "NL
            "	i = tointeger(i)                               "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description316 = "built-in function call";
    int retcode316 = ERROR_NOERROR;
    pfile_t *pf316 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : number                               "NL
            "	i = tointeger(4)                               "NL
            "	write(i)                                       "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description317 = "built-in function call wrong params";
    int retcode317 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf317 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local i : number                               "NL
            "	i = ord(\"hello\", i)                          "NL
            "	write(i)                                       "NL
            "end                                               "NL
    );

    char *description318 = "fuilt-in function call wrong params";
    int retcode318 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf318 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local s : string                               "NL
            "	local i : string                               "NL
            "	s = chr(i)                                     "NL
            "	write(i)                                       "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description319 = "built-in function call";
    int retcode319 = ERROR_NOERROR;
    pfile_t *pf319 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "   local s : string = \"bumblebee\"               "NL
            "	while (s > \"\") do                              "NL
            "		s = substr(s, 1, #s - 1)                   "NL
            "		write(s)                                   "NL
            "	end                                            "NL
            "	write(s)                                       "NL
            "end                                               "NL
            "main()                                            "NL
    );

    char *description320 = "built-in function call expecting error 8";
    int retcode320 = ERROR_NOERROR;
    pfile_t *pf320 = Pfile.ctor(
            PROLOG
            "function main()                                   "NL
            "	local s : string                               "NL
            "	while (s > \"\") do                              "NL
            "		s = substr(s, 1, #s - 1)                   "NL
            "		write(s)                                   "NL
            "	end                                            "NL
            "	write(s)                                       "NL
            "end                                               "NL
            "                                                  "NL
            "main()                                            "NL
    );

    char *description321 = "more ids than return values in assignment";
    int retcode321 = ERROR_FUNCTION_SEMANTICS;
    pfile_t *pf321 = Pfile.ctor(
            PROLOG
            "function bar(a : integer) : integer               "NL
            "   write(a)                                       "NL
            "end                                               "NL
            "function main(i : integer)                        "NL
            "	local j : number                               "NL
            "	i, j = bar(i)                                  "NL
            "end                                               "NL
            "main(1)                                           "NL
    );

    STRING_NOERROR(107, "02");
    STRING_NOERROR(108, "0A");
    STRING_NOERROR(109, "aa");
    STRING_NOERROR(110, "bb");

    STRING_NOERROR(111, "cc");
    STRING_NOERROR(112, "Aa");
    STRING_NOERROR(113, "Ab");
    STRING_NOERROR(114, "ff");
    STRING_NOERROR(115, "Ff");
    STRING_NOERROR(116, "FF");
    STRING_NOERROR(117, "F0");
    STRING_NOERROR(118, "0f");
    STRING_NOERROR(119, "0F");

    STRING_NOERROR(120, "0A");
    STRING_NOERROR(121, "A0");
    STRING_NOERROR(122, "F0");
    STRING_NOERROR(123, "F2");
    STRING_NOERROR(124, "DD");
    STRING_NOERROR(125, "FD");
    STRING_ERROR(126, "\\x00");
    STRING_ERROR(127, "\\0x22");
    STRING_ERROR(128, "\\000");
    STRING_ERROR(129, "\269");

    STRING_ERROR(130, "\256");
    STRING_ERROR(131, "\\a");
    STRING_ERROR(132, "\\x");
    STRING_ERROR(133, "\\f");
    STRING_ERROR(134, "\\r");
    STRING_ERROR(135, "\\0x");
    STRING_ERROR(136, "\\N");

    GLOBAL_EXPRESSION_ERROR(137, "aa(a(a(1)) + b(a(1)), c(a(1)) + d(a(1))");
    GLOBAL_EXPRESSION_ERROR(138, "aa(   a(a(1)) + b(a(1)), c(a(1)) + d(,a(1))   )");
    GLOBAL_EXPRESSION_ERROR(139, "aa(   a(a(1)) + b(a(1)), c(a(1)) + d(a(1),)   )");

    GLOBAL_EXPRESSION_NOERROR(140, "aaa(    a(1) + b(2), c(c(c(c(c(2))))) + dd( 1 + 2, 3 * (4 + a((2))) ), 2    )");

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
    TEST_CASE(53);
    TEST_CASE(54);
    TEST_CASE(55);
    TEST_CASE(56);
    TEST_CASE(57);
    TEST_CASE(58);
    TEST_CASE(59);

    TEST_CASE(60);
    TEST_CASE(61);
    TEST_CASE(62);
    TEST_CASE(63);
    TEST_CASE(64);
    TEST_CASE(65);
    TEST_CASE(66);
    TEST_CASE(67);
    TEST_CASE(68);
    TEST_CASE(69);

    TEST_CASE(70);
    TEST_CASE(71);
    TEST_CASE(72);
    TEST_CASE(73);
    TEST_CASE(74);
    TEST_CASE(75);
    TEST_CASE(76);
    TEST_CASE(77);
    TEST_CASE(78);
    TEST_CASE(79);

    TEST_CASE(80);
    TEST_CASE(81);
    TEST_CASE(82);
    TEST_CASE(83);
    TEST_CASE(84);
    TEST_CASE(85);
    TEST_CASE(86);
    TEST_CASE(87);
    TEST_CASE(88);
    TEST_CASE(89);

    TEST_CASE(90);
    TEST_CASE(91);
    TEST_CASE(92);
    TEST_CASE(93);
    TEST_CASE(94);
    TEST_CASE(95);
    TEST_CASE(96);
    TEST_CASE(97);
    TEST_CASE(98);
    TEST_CASE(99);


    TEST_CASE(101);
    TEST_CASE(102);
    TEST_CASE(103);
    TEST_CASE(104);
    TEST_CASE(105);
    //TEST_CASE(106);
    TEST_CASE(107);
    TEST_CASE(108);
    TEST_CASE(109);

    TEST_CASE(110);
    TEST_CASE(111);
    TEST_CASE(112);
    TEST_CASE(113);
    TEST_CASE(114);
    TEST_CASE(115);
    TEST_CASE(116);
    TEST_CASE(117);
    TEST_CASE(118);
    TEST_CASE(119);

    TEST_CASE(120);
    TEST_CASE(121);
    TEST_CASE(122);
    TEST_CASE(123);
    TEST_CASE(124);
    TEST_CASE(125);
    TEST_CASE(126);
    TEST_CASE(127);
    TEST_CASE(128);
    TEST_CASE(129);

    TEST_CASE(130);
    TEST_CASE(131);
    TEST_CASE(132);
    TEST_CASE(133);
    TEST_CASE(134);
    TEST_CASE(135);
    TEST_CASE(136);
    TEST_CASE(137);
    TEST_CASE(138);
    TEST_CASE(139);
    TEST_CASE(140);


    TEST_CASE(141);
    TEST_CASE(142);
    TEST_CASE(143);
    TEST_CASE(144);
    TEST_CASE(145);
    TEST_CASE(146);
    TEST_CASE(147);
    TEST_CASE(148);
    TEST_CASE(149);
    TEST_CASE(150);
    TEST_CASE(151);
    TEST_CASE(152);
    TEST_CASE(153);
    TEST_CASE(154);
    TEST_CASE(155);
    TEST_CASE(156);
    TEST_CASE(157);
    TEST_CASE(158);
    TEST_CASE(159);
    TEST_CASE(160);
    TEST_CASE(161);
    TEST_CASE(162);
    TEST_CASE(163);
    TEST_CASE(164);
    TEST_CASE(165);
    TEST_CASE(166);
    TEST_CASE(167);
    TEST_CASE(168);
    TEST_CASE(169);
    TEST_CASE(170);
    TEST_CASE(171);
    TEST_CASE(172);
    TEST_CASE(173);
    TEST_CASE(174);
    TEST_CASE(175);
    TEST_CASE(176);
    TEST_CASE(177);
    TEST_CASE(178);
    TEST_CASE(179);
    TEST_CASE(180);
    TEST_CASE(181);
    TEST_CASE(182);
    TEST_CASE(183);
    TEST_CASE(184);
    TEST_CASE(185);
    TEST_CASE(186);
    TEST_CASE(187);
    TEST_CASE(188);
    TEST_CASE(189);
    TEST_CASE(190);
    TEST_CASE(191);
    TEST_CASE(192);
    TEST_CASE(193);
    TEST_CASE(194);
    TEST_CASE(195);
    TEST_CASE(196);
    TEST_CASE(197);
    TEST_CASE(198);
    TEST_CASE(199);
    TEST_CASE(200);
    TEST_CASE(201);
    TEST_CASE(202);
    TEST_CASE(203);
    TEST_CASE(204);
    TEST_CASE(205);
    TEST_CASE(206);
    TEST_CASE(207);
    TEST_CASE(208);
    TEST_CASE(209);
    TEST_CASE(210);
    TEST_CASE(211);
    TEST_CASE(212);
    TEST_CASE(213);
    TEST_CASE(214);
    TEST_CASE(215);
    TEST_CASE(216);
    TEST_CASE(217);
    TEST_CASE(218);
    TEST_CASE(219);
    TEST_CASE(220);
    TEST_CASE(221);
    TEST_CASE(222);
    TEST_CASE(223);
    TEST_CASE(224);
    TEST_CASE(225);
    TEST_CASE(226);
    TEST_CASE(227);
    TEST_CASE(228);
    TEST_CASE(229);
    TEST_CASE(230);
    TEST_CASE(231);
    TEST_CASE(232);
    TEST_CASE(233);
    TEST_CASE(234);
    TEST_CASE(235);
    TEST_CASE(236);
    TEST_CASE(237);
    TEST_CASE(238);
    TEST_CASE(239);
    TEST_CASE(240);
    TEST_CASE(241);
    TEST_CASE(242);
    TEST_CASE(243);
    TEST_CASE(244);
    TEST_CASE(245);
    TEST_CASE(246);
    TEST_CASE(247);
    TEST_CASE(248);
    TEST_CASE(249);
    TEST_CASE(250);
    TEST_CASE(251);
    TEST_CASE(252);
    TEST_CASE(253);
    TEST_CASE(254);
    TEST_CASE(255);
    TEST_CASE(256);
    TEST_CASE(257);
    TEST_CASE(258);
    TEST_CASE(259);
    TEST_CASE(260);
    TEST_CASE(261);
    TEST_CASE(262);
    TEST_CASE(263);
    TEST_CASE(264);
    TEST_CASE(265);
    TEST_CASE(266);
    TEST_CASE(267);
    TEST_CASE(268);
    TEST_CASE(269);
    TEST_CASE(270);
    TEST_CASE(271);
    TEST_CASE(272);
    TEST_CASE(273);
    TEST_CASE(274);
    TEST_CASE(275);
    TEST_CASE(276);
    TEST_CASE(277);
    TEST_CASE(278);
    TEST_CASE(279);
    TEST_CASE(280);
    TEST_CASE(281);
    TEST_CASE(282);
    TEST_CASE(283);
    TEST_CASE(284);
    TEST_CASE(285);
    TEST_CASE(286);
    TEST_CASE(287);
    TEST_CASE(288);
    TEST_CASE(289);
    TEST_CASE(290);
    TEST_CASE(291);
    TEST_CASE(292);
    TEST_CASE(293);
    TEST_CASE(294);
    TEST_CASE(295);
    TEST_CASE(296);
    TEST_CASE(297);
    TEST_CASE(298);
    TEST_CASE(299);
    TEST_CASE(300);
    TEST_CASE(301);
    TEST_CASE(302);
    TEST_CASE(303);
    TEST_CASE(304);
    TEST_CASE(305);
    TEST_CASE(306);
    TEST_CASE(307);
    TEST_CASE(308);
    TEST_CASE(309);
    TEST_CASE(310);
    TEST_CASE(311);
    TEST_CASE(312);
    TEST_CASE(313);
    TEST_CASE(314);
    TEST_CASE(315);
    TEST_CASE(316);
    TEST_CASE(317);
    TEST_CASE(318);
    TEST_CASE(319);
    TEST_CASE(320);
    TEST_CASE(321);

    return 0;
}
