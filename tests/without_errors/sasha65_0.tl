-- test case 65.
-- Description : break keyword in repeat-until

-- Expected : '0'
require "ifj21" 
function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()
    write("I'm about 2bee printed only once!")    
end                                                 
                                                    
function yours()                                    
   local a : boolean = true                         
   repeat                                           
       to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()  
                                                    
       a = false                                    
       break                                        
   until a == true                                  
end                                                 
yours()                                             

