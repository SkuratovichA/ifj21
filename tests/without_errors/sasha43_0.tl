-- test case 43.
-- Description : no error.

-- Expected : '0'
require "ifj21" 
function foo( r : string, a : string ) : nil
   write(" \xff ")                       
end                                         

