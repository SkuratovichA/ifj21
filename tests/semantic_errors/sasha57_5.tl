-- test case 57.
-- Description : return types #5. Wrong return types

-- Expected : '5'
require "ifj21" 

global foo : function(    string,     string ) : string, number 
function foo (a : string, b : string) :  string, number         
    return 10, 10                                               
end                                                             
foo()                                                           

