-- test case 72.
-- Description : while cycle. Break inside elseif statement

-- Expected : '0'
require "ifj21" 
function main(iterations : number )                    
    local i : integer = 3                              
    while i ~= 0 do                                    
        local j : integer = 3                          
        while j ~= 0 do                                
           local k : integer = 3                       
           while k ~= 0 do                             
               if k > 0 then                           
               elseif k > 2 then                       
                   break                               
               end                                     
               write("hello")                        
               k = k - 1                               
           end                                         
           break                                       
           j = j - 1                                   
        end                                            
        i = i - 2                                      
    end                                                
end                                                    
main(3)                                                

