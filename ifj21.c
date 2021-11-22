/**
 * @file ifj21.c
 *
 * @brief main() program.
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 */

#include "errors.h"
#include "parser.h"
#include "progfile.h"
#include "list.h"
#include "code_generator.h"


int main() {
    pfile_t *pfile;
    int ret = 0;


//    if (!(pfile = Pfile.getfile_stdin())) {
//        return Errors.get_error();
//    }
    pfile = Pfile.ctor(""
                       "-- test case 4.\n"
                       "-- Description : no error, parsing definitions, declarations\n"
                       "\n"
                       "-- Expected : '0'\n"
                       "require \"ifj21\" \n"
                       " global foo : function()\n"
                       " global baz : function(string)\n"
                       " global bar : function(string, integer)\n"
                       " global aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa : function(string) : string, string, string\n"
                       "\n"
                       "\n"
                       " function foo()\n"
                       " end \n"
                       " function baz(str : string)\n"
                       " end \n"
                       "\n"
                       " function bar(str : string, int : integer)\n"
                       " end \n"
                       "\n"
                       " global arst : function(string,         integer,             number,       number,     integer, string)\n"
                       " function arst(str : string, ddd : integer, nummm : number, aaa : number, ii: integer, suka :string)\n"
                       " end \n"
                       "\n"
                       " function aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa (str : string) : string, string, string\n"
                       "\n"
                       " end \n"
    );


    Generator.initialise();

    if (!Parser.analyse(pfile)) {
        return Errors.get_error();
    }


    // ERROR_NOERROR has value 0
    ret = Errors.get_error();

    // Print instructions only when everything was ok
    printf("\n# <<<<<<<<<< Return code: %d >>>>>>>>>>\n\n", ret);
    if (ret == 0) {
        // Prints the list of instructions to stdout
        printf("# ---------- Instructions List ----------\n");
        list_item_t *tmp;

        printf("# ---------- startList ----------\n");
        INSTR_CHANGE_ACTIVE_LIST(instructions.startList);
        tmp = instrList ? instrList->head : NULL;
        size_t x = (size_t) ((void *) tmp->data);
        if (!x) {
            return 0;
        }
        while (tmp != NULL) {
            printf("[]\n");
            debug_msg_s("%p\n", (void *) tmp->data);
            printf("%s\n", Dynstring.c_str((dynstring_t *) tmp->data));
            tmp = tmp->next;
        }

        printf("# ---------- listFunctions ----------\n");
        INSTR_CHANGE_ACTIVE_LIST(instructions.instrListFunctions);
        tmp = instrList ? instrList->head : NULL;
        x = (size_t) ((void *) tmp->data);
        if (!x) { return 0; }
        while (tmp != NULL) {
            printf("[]\n");
            printf("%s\n", Dynstring.c_str(tmp->data));
            tmp = tmp->next;
        }

        printf("# ---------- mainList ----------\n");
        INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
        tmp = instrList ? instrList->head : NULL;
        while (tmp != NULL) {
            printf("%s\n", Dynstring.c_str(tmp->data));
            tmp = tmp->next;
        }
    }

    Generator.dtor();

    Pfile.dtor(pfile);
    return ret;
}
