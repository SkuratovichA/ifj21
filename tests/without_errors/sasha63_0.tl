-- test case 63.
-- Description : for cycles

-- Expected : '0'
require "ifj21" 
function main(iterations : number )      
    for i=0,iterations,2 do              
        for j=0,iterations,2 do        
            write("hello, many times") 
        end                              
    end                                  
end                                      
main(4)                                  

