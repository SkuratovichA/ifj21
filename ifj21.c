#include <stdio.h>
#include "errors.h"
#include "parser.h"
#include "progfile.h"
#include "list.h"
#include "code_generator.h"


int main() {
    pfile_t *pfile;
    int ret = 0;

    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }

    // Initialise code generator lists of instructions
    // variable for currently generated instruction
    tmp_instr = Dynstring.ctor("");
    // initialize lists of instructions
    instructions.startList = List.ctor();
    instructions.instrListFunctions = List.ctor();
    instructions.mainList = List.ctor();
    instructions.in_loop = false;
    instructions.before_loop_start = NULL;
    instructions.outer_cond_id = 0;
    instructions.outer_loop_id = 0;

    instrList = instructions.startList;

    if (Parser.analyse(pfile)) {
        ret = Errors.get_error();
    }


    // Print instructions only when everything was ok
    if (ret == 0) {
        // Prints the list of instructions to stdout
        printf("# ---------- Instructions List ----------\n");
        INSTR_CHANGE_ACTIVE_LIST(instructions.startList);
        printf("# ---------- startList ----------\n");
        for (list_item_t *tmp = instrList->head; tmp != NULL; tmp = tmp->next) {
            printf("%s\n", Dynstring.c_str(tmp->data));
        }
        INSTR_CHANGE_ACTIVE_LIST(instructions.instrListFunctions);
        printf("# ---------- listFunctions ----------\n");
        for (list_item_t *tmp = instrList->head; tmp != NULL; tmp = tmp->next) {
            printf("%s\n", Dynstring.c_str(tmp->data));
        }
        INSTR_CHANGE_ACTIVE_LIST(instructions.mainList);
        printf("# ---------- mainList ----------\n");
        for (list_item_t *tmp = instrList->head; tmp != NULL; tmp = tmp->next) {
            printf("%s\n", Dynstring.c_str(tmp->data));
        }
    }

    List.dtor(instructions.startList, Dynstring.dtor); // use Dynstring.dtor or free?
    List.dtor(instructions.instrListFunctions, Dynstring.dtor); // use Dynstring.dtor or free?
    List.dtor(instructions.mainList, Dynstring.dtor); // use Dynstring.dtor or free?
    Dynstring.dtor(tmp_instr);

    Pfile.dtor(pfile);
    return ret;
}
