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

    Parser.analyse(pfile);

    // ERROR_NOERROR has value 0
    ret = Errors.get_error();

    // Print instructions only when everything was ok
    printf("\n# <<<<<<<<<< Return code: %d >>>>>>>>>>\n\n", ret);
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

    List.dtor(instructions.startList, (void (*)(void *)) (Dynstring.dtor)); // use Dynstring.dtor or free?
    List.dtor(instructions.instrListFunctions, (void (*)(void *)) Dynstring.dtor); // use Dynstring.dtor or free?
    List.dtor(instructions.mainList, (void (*)(void *)) Dynstring.dtor); // use Dynstring.dtor or free?
    Dynstring.dtor(tmp_instr);

    Pfile.dtor(pfile);
    return ret;
}
