#include <stdio.h>
#include "errors.h"
#include "parser.h"

int main() {
    printf("Hello!\n");

    if (Parser.analyse()) {
        return Errors.get_error();
    }

    return 0;
}
