#include <stdio.h>
#include "errors.h"
#include "parser.h"
#include "progfile.h"

int main() {
    printf("Hello!\n");

    pfile_t *pfile;

    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }

    if (Parser.analyse(pfile)) {
        return Errors.get_error();
    }

    return 0;
}
