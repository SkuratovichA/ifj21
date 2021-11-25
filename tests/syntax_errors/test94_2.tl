-- test case 29.
-- Description : function semantic: no error.

-- Expected : '0'
require "ifj21" 

global foo : function(    "ddd",     string ) : string, number 
function foo (a : string, b : string) :  string, number         
    return "hello", 10                                        
end                                                             
function main()                                                 
   local a : string                                             
   local b : number                                             
   a, b = foo()                                                 
   write(a)                                                     
   write(b)                                                     
end                                                             
main()                                                          

