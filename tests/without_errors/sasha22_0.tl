-- test case 22.
-- Description : more repuntil. Without an error

-- Expected : '0'
require "ifj21" 

function me() :string                                            
    return "the whole project was written by a cat"            
end                                                              
function main()                                                  
 local you : string = "atata"                                 
     repeat                                                     
         repeat                                                 
             repeat                                             
                 repeat                                         
                     repeat                                     
                         repeat                                 
                             local not_true : string = me()     
                              write(not_true)                   
                         until false                            
                     until 1 < 0                                
                 until 2 + 2 == 5                                
             until false                                        
         until true and false                                   
     until false or false                                       
                                                                
    return                                                      
end                                                             
main()                                                          

