-- test case 34.
-- Description : Function semantics def before decl. Expr as an argument.

-- Expected : '0'
require "ifj21" 
function foo( a : string, b : string ) : nil 
end                                          
global foo : function(string, string ) : nil  
foo("I " .. "LOVE",  "MINECRAFT")  

