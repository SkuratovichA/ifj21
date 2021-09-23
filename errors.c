//
// Created by suka on 23.09.2021.
//

#include "errors.h"
#include <stdio.h>

static int Return_error(int errcode) {
    char *errmsg = "Undefined error code!";
    switch (errcode) {
        case ERROR_LEXICAL:
//             1 - chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému).
            errmsg = "Lexical error";
            break;
        case ERROR_SYNTAX:
//            • 2 - chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu).
            errmsg = "Syntax error";
            break;
        case ERROR_UNDEF_FUN_OR_VAR:
            // todo: does it want to add more error messages? i.e undefined function, undefined variable ...
        // case ERROR_UNDEFINED_FUNCTION:
//            • 3 - sémantická chyba v programu – nedefinovaná funkce/proměnná, pokus o redefinici proměnné, atp.
            errmsg = "Undefined function/variable!";
            break;
        case ERROR_TYPE_MISSMATCH:
 //           • 4 - sémantická chyba v příkazu přiřazení (typová nekompatibilita).
            errmsg = "Type missmatch!";
            break;
        case ERROR_SEMANTICS_NUMBER_PARAMETERS:
  //          • 5 - sémantická chyba v programu – špatný počet/typ parametrů či návratových hodnot u volání funkce či návratu z funkce.
            errmsg = "Wrong number of parameters/return values!";
            break;
        case ERROR_SEMANTICS_TYPE_INCOMPATABLE:
   //         • 6 - sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech.
            errmsg = "Type incompatibility error!";
            break;
        case ERROR_SEMANTICS_OTHER:
    //        • 7 - ostatní sémantické chyby.
            errmsg = "Semantic error!";
            break;
        case ERROR_RUNTIME_NIL:
     //       • 8 - běhová chyba při práci s neočekávanou hodnotou nil.
            errmsg = "nil assignment error!";
            break;
        case ERROR_RUNTIME_DIV_BY_ZERO:
      //      • 9 - běhová chyba celočíselného dělení nulovou konstantou.
            errmsg = "Division by zero error";
            break;
        case ERROR_INTERNAL:
       //     • 99 - interní chyba překladače tj. neovlivněná vstupním programem (např. chyba alokace paměti, atd.).
            errmsg = "Internal error";
            break;
        default:
            break;
    }
    fprintf(stderr, "ERROR: %s\n", errmsg);
    return errcode;
}

const struct error Errors= {
        .return_error = Return_error
};
