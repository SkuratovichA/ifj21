/********************************************
 * Project name: IFJ - projekt
 * File: interpret.c
 * Date: 23. 09. 2021
 * Last change: 23. 09. 2021
 * Team: TODO
 * Authors:  Aliaksandr Skuratovich
 *           Evgeny Torbin
 *           Lucie Svobodová
 *           Jakub Kuzník
 *******************************************/
/**
 *
 *
 *  @package interpret
 *  @file interpret.c
 *  @brief Main file - ifj21 compiler.
 *
 *
 *
 *  @author Aliaksandr Skuratovich
 *  @author Evgeny Torbin
 *  @author Lucie Svobodová
 *  @author Jakub Kuzník
 */
#include <stdio.h>
#include "errors.h"
#include "progfile.h"
#include "scanner.h"
#include "bin_tree.h"
#include "progfile.h"
int tree_testing();
int main() {

    printf("Hello!\n");

    progfile_t *pfile;

    token_t token;
    pfile = Scanner.initialize();
    if (!pfile) {
        return -1;
    }

    while ((token = Scanner.get_curr_token()).type != TOKEN_EOFILE) {
        debug_msg("%s\n", Scanner.to_string(Scanner.get_next_token(pfile).type));
        if (token.type == TOKEN_EOL) {
            debug_msg("\n");
        }
    }


    Progfile.free(pfile);
    Errors.return_error(42);
    return 0;
}

