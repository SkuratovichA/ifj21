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

    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }

    Generator.initialise();

    if (!Parser.analyse(pfile)) {
        return Errors.get_error();
    }

    ret = Errors.get_error();

    // Print instructions only when everything was ok
    debug_msg("\n# <<<<<<<<<< Return code: %d >>>>>>>>>>\n\n", ret);
    if (ret == 0) {
        // Prints the list of instructions to stdout
        debug_msg("# ---------- Instructions List ----------\n");

        debug_msg("# ---------- startList ----------\n");
        Generator.print_instr_list(LIST_INSTR_START);

        debug_msg("# ---------- listFunctions ----------\n");
        Generator.print_instr_list(LIST_INSTR_FUNC);

        debug_msg("# ---------- mainList ----------\n");
        Generator.print_instr_list(LIST_INSTR_MAIN);
    }

    Generator.dtor();

    Pfile.dtor(pfile);
    return ret;
}
