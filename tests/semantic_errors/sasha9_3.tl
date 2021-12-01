-- test case 9.
-- Description : function die() has not been defined

-- Expected : '3'
require "ifj21" 
function fuck() :string                                 
   return "shit, one more memory leak in the project" 
end                                                     
                                                        
global die :function ()                                 
                                                        
function main()                                         
    local suka: number = 12                             
                                                        
    if suka > 10 then                                   
        write("suka (number)> 10")                    
        local suka:string = "AAAAAAAAA"               
        if #suka > 10 then                              
            must_not_be_reached()                       
        elseif #suka < 10 then                          
            die()                                       
        else                                            
            must_not_be_reached()                       
        end                                             
    end                                                 
end                                                     
main()                                                  


