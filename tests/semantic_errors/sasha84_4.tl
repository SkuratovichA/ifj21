-- test case 84.
-- Description : for cycles, type mismatch in the condition.

-- Expected : '4'
require "ifj21" 
function main(iterations : number )      
    local a : integer = 10               
    for i=0, true, 20 do                 
    end                                  
end                                      
main("helloo")                         

