-- test case 58.
-- Description : return types #6. Typecasting nil in return.

-- Expected : '0'
require "ifj21" 

global foo : function(    string,     string ) : string, number 
function foo (a : string, b : string) :  string, number         
    return nil, nil                                             
end                                                             
foo()                                                           

