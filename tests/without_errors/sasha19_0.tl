-- test case 19.
-- Description : while, no error

-- Expected : '0'
require "ifj21" 
function main()             
    local a : integer = 10  
    while a >= 0 do         
        write ("10 times")
        a = a - 1           
    end                     
end                         
main()                      

