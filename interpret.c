#include <stdio.h>
#include "errors.h"


int main() {
    printf("Hello!\n");
    Errors.return_error(42);
    return 0;
}
