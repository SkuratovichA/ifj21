#include <stdio.h>
#include "errors.h"
#include "parser.h"
#include "progfile.h"

int main() {
    pfile_t *pfile;

    if (DEBUG) {
        // tests
        pfile_t *pf1 = Pfile.ctor("require \"ifj21\"\n");
        Parser.analyse(pf1);
        debug_msg_s("Tests are working!\n");
        Pfile.free(pf1);
    }

    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }


    if (Parser.analyse(pfile)) {
        return Errors.get_error();
    }


    return 0;
}
