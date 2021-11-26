-- test case 68.
-- Description : while cycle.

-- Expected : '0'
require "ifj21" 
function main(iterations : number )                    
    local i : integer = 3                              
    while i ~= 0 do                                    
       local j : integer = 3                           
       while j ~= 0 do                                 
          local k : integer = 3                        
          while k ~= 0 do                              
              write("hello")                         
              k = k - 1                                
          end                                          
          j = j - 1                                    
       end                                             
       i = i - 2                                       
    end                                                
end                                                    
main(3)                                                

