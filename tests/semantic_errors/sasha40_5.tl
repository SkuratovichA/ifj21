-- test case 40.
-- Description : no expression after comma

-- Expected : '5'
require "ifj21" 
functino three(): number return 3 end        
function foo( a : number, b : number )       
    write(a)                                 
    write(b)                                 
end                                          
foo(1 + 2 + 3 + 4 * (2 + 3 + three()), )     

