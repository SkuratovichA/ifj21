#include <stdio.h>
#include "errors.h"
#include "parser.h"
#include "progfile.h"
#include "symtable.h"

int main() {
    pfile_t *pfile;
    int ret = 0;

    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }

    if (Parser.analyse(pfile)) {
        ret = Errors.get_error();
    }

    Pfile.dtor(pfile);

    Symt.st_dtor(symbol_tab);

    return ret;
}
