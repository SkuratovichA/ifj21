-- test case 66.
-- Description : for cycles, break keyword

-- Expected : '0'
require "ifj21" 
function main(iterations : number )      
    for i=0,iterations do                
       write("I'm gonna be printed\n")
        for j=0, iterations do          
            break                        
            write("hello, 9 times")    
        end                              
    end                                  
end                                      
main(3)                                  

