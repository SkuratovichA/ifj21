-- test case 37.
-- Description : Function semantics. Parameters have same names.

-- Expected : '3'
require "ifj21" 
function foo( a : number, a : string ) : nil 
end                                          
foo()                                        

