-- test case 60.
-- Description : return types #8

-- Expected : '0'
require "ifj21" 
function is_beautiful(life : boolean) : boolean          
   return false                                          
end                                                      
function opposite(smth : boolean, a2 : boolean) : boolean
   if smth == a2 then                                    
       return false                                      
   else                                                  
       return true                                       
   end                                                   
end                                                      

