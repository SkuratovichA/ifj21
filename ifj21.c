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
#include "progfile.h"
#include "parser.h"

int main() {
    printf("Hello!\n");
    Parser.hello_from_parser();

    if (Parser.analyse()) {
        return Errors.get_error();
    }


    Errors.return_error(42);
    return 0;
}
