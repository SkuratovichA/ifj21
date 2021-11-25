-- test case 7.
-- Description : nested while

-- Expected : '0'
require "ifj21" 
function is_beautiful(life : boolean) : boolean
   return false                                
end                                            
function opposite(smth : boolean, a2 : boolean) : boolean
   if smth == a2 then return false else return true end  
end                                                      
                                                         
function main()                                          
   local myself : string  = "me"                       
   local love : boolean                                    
   local hate : boolean                                    
   local faith : boolean                                   
   local heresy : boolean                                  
   local death : boolean                                   
   local life : boolean                                    
   local indifference : boolean                            
   local ugliness : boolean                                
   local art : boolean                                     
   while opposite(love, hate) == false and opposite(love, indifference) do                 
       while opposite(art, ugliness) == false and opposite(art, indifference) do           
           while opposite(faith, heresy) == false and opposite(faith, indifference) do  
               while opposite(life, death) == false and opposite(life, indifference) do 
                   is_beautiful(life)                                                      
               end                                                                         
           end                                                                             
       end                                                                                 
   end                                                                                     
end                                                                                        
main()                                                                                     

