-- test case 64.
-- Description : for cycles

-- Expected : '0'
require "ifj21" 
function main(iterations : number )      
    for i=0,iterations do                
        for j=0,i<iterations do          
            write("hello, 9 times")    
        end                              
    end                                  
end                                      
main(3)                                  

