-- test case 56.
-- Description : return types #4. Exceed returns

-- Expected : '5'
require "ifj21" 

global foo : function(    string,     string ) : string, number 
function foo (a : string, b : string) :  string, number         
    return "hello", 10, 10                                    
end                                                             
foo()                                                           

