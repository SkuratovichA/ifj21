-- test case 59.
-- Description : return types #7. Typecasting nil in return.

-- Expected : '0'
require "ifj21" 

global foo : function(    string,     string ) : string, number 
function foo (a : string, b : string) :  string, number, boolean
    return nil, nil, true                                       
end                                                             
foo()                                                           

