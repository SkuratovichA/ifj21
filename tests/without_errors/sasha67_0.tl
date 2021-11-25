-- test case 67.
-- Description : while cycle.

-- Expected : '0'
require "ifj21" 
function main(iterations : number )              
    local i : integer = 10                       
    while i ~= 0 do                              
       write("I'm gonna be printed 5 times\n")
       i = i - 2                                 
    end                                          
end                                              
main(3)                                          

