-- test case 38.
-- Description : Complicated expr in function calling

-- Expected : '0'
require "ifj21" 
function three(): number return 3 end        
function foo( a : number, b : number )       
    write(a)                                 
    write(b)                                 
end                                          
foo(1 + 2 + 3 + 4 * (2 + 3 + three()), 1)    

