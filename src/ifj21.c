/**
 * @file ifj21.c
 *
 * @author Skuratovich Aliaksandr <xskura01@vutbr.cz>
 * @author Lucie Svobodova <xsvobo1x@vutbr.cz>
 */

#include "errors.h"
#include "parser.h"
#include "progfile.h"
#include "list.h"
#include "code_generator.h"


int main() {
    pfile_t *pfile = NULL;

    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }

    Generator.initialise();

    if (!Parser.analyse(pfile)) {
        goto ret;
    }
    
    // Prints the list of instructions to stdout
    debug_msg("# ---------- Instructions List ----------\n");

    debug_msg("# ---------- startList ----------\n");
    Generator.print_instr_list(LIST_INSTR_START);

    debug_msg("# ---------- listFunctions ----------\n");
    Generator.print_instr_list(LIST_INSTR_FUNC);

    debug_msg("# ---------- mainList ----------\n");
    Generator.print_instr_list(LIST_INSTR_MAIN);

    ret:
    Generator.dtor();
    Pfile.dtor(pfile);
    return Errors.get_error();
}
