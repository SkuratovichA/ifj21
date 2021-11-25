-- test case 54.
-- Description : return types #2 no returns.

-- Expected : '0'
require "ifj21" 

global foo : function(    string,     string ) : string, number 
function foo (a : string, b : string) :  string, number         
end                                                             
foo()                                                          

