-- test case 8.
-- Description : elsif, if, NO ERRROR

-- Expected : '0'
require "ifj21" 
function die()                               
   write("I'm dead x_x")                   
end                                          
                                             
function must_not_be_reached()               
   write("MUST NOT BE REACHED x_x")        
end                                          
                                             
                                             
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


