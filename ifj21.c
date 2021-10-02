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
 */
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
