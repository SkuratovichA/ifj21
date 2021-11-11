#include <stdio.h>
#include "errors.h"
#include "parser.h"
#include "progfile.h"
#include "symtable.h"
// moved definition of list_item_t to list.h
#include "list.h"
#include "code_generator.h"


int main() {
    pfile_t *pfile;
    int ret = 0;

    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }

    instrList = List.ctor();            // creating global list of instructions
    tmp_instr = Dynstring.ctor("");     // variable for currently generated instruction

    if (Parser.analyse(pfile)) {
        ret = Errors.get_error();
    }

    Generator.prog_start();             // test - remove later
    // Prints the list of instructions to stdout
    printf("# ---------- Instructions List ----------\n");
    for (list_item_t *tmp = instrList->head; tmp != NULL; tmp = tmp->next ) {
        printf("%s\n", Dynstring.c_str(tmp->data));
    }

    List.dtor(instrList, Dynstring.dtor); // use Dynstring.dtor or free?
    Dynstring.dtor(tmp_instr);

    Pfile.dtor(pfile);
    return ret;
}
