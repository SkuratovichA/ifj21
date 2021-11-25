-- test case 11.
-- Description : more repuntil, without an error

-- Expected : '0'
require "ifj21" 
function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee() : 17 
    write("I'm about 2bee printed only once!")               
    return false                                               
end                                                            
                                                               
function yours()                                               
   local a : boolean = true                                    
   repeat                                                      
       a = to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()         
   until a == true                                             
end                                                            
yours()                                                        

