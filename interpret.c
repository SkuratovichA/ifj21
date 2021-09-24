//
// Created by xskura01 on 23.09.2021.
//

#include <stdio.h>
#include "errors.h"
#include "progfile.h"
#include "scanner.h"
#include "progfile.h"

int main() {
    printf("Hello!\n");

    progfile_t *pfile;

    token_t token;
    pfile = Scanner.initialize();
    if (!pfile) {
        return -1;
    }

    while ((token = Scanner.get_curr_token()).type != TOKEN_EOFILE) {
        debug_msg("%s", Scanner.to_string(Scanner.get_next_token(pfile).type));
        if (token.type == TOKEN_EOL) {
            debug_msg("\n");
        }
    }


    Progfile.free(pfile);

    Errors.return_error(42);
    return 0;
}
