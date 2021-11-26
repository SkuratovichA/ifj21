-- test case 20.
-- Description : nested whiles 

-- Expected : '0'
require "ifj21" 
function main()                    
    local a : integer = 2          
    local b : integer = 2          
    local c : integer = 2          
    while a >= 0 do                
        a = a-1                    
        while b >= 0 do            
            b = b-1                
            while c >= 0 do        
                c = c-1            
                write ("9 times")
            end                    
        end                        
    end                            
end                                
main()                             

