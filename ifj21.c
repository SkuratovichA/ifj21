#include <stdio.h>
#include "errors.h"
#include "parser.h"
#include "progfile.h"
#include "tests/tests.h"

#define PARSERTEST 1

int main() {
    pfile_t *pfile;

    if (PARSERTEST) {
        // tests
        pfile_t *pf1 = Pfile.ctor("require \"ifj21\"\n");
        TEST_EXPECT(Parser.analyse(pf1), true, "First test.");
        Pfile.free(pf1);
        // tests
        pfile_t *pf2 = Pfile.ctor("1234.er require \"ifj21\"\n");
        TEST_EXPECT(Parser.analyse(pf2), false, "First test.");
        Pfile.free(pf2);
    }

    //=====
    if (!(pfile = Pfile.getfile_stdin())) {
        return Errors.get_error();
    }

    if (Parser.analyse(pfile)) {
        return Errors.get_error();
    }


    return 0;
}
