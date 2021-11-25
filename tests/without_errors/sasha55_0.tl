-- test case 55.
-- Description : return types #3. Typecasting

-- Expected : '0'
require "ifj21" 

global foo : function(    string,     string ) : string, number 
function foo (a : string, b : string) :  string, number         
    return "hello", 10                                        
end                                                             
foo()                                                          

