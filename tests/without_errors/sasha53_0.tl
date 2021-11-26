-- test case 53.
-- Description : return types #1.

-- Expected : '0'
require "ifj21" 

global foo : function(    string,     string ) : string, number 
function foo (a : string, b : string) :  string, number         
    return "hello", 10.5 + 1.5                                
end                                                             
foo()                                                          

