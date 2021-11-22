-- test case 28.
-- Description : Function semantics definition before declaration

-- Expected : '3'
require "ifj21" 

 function                foo( a : string, b : string ) : nil
 end 
 global foo : function(string, string ) : nil 

