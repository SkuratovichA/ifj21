-- test case 24.
-- Description : more repuntil. No Until

-- Expected : '2'
require "ifj21" 

function me() :string                                            
    return "the whole project was written by a dog"            
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
                 until 2 + 2 = 5                                
             until false                                        
         until true and false                                   
                                                                
    return                                                      
end                                                             
main()                                                          

