-- test case 12.
-- Description : more repuntil, error with types

-- Expected : '4'
require "ifj21" 
function to_be_a_bee_but_bi_bee_and_maybe_be_a_bee() : string 
    return "I'm about 2bee printed only once!"              
end                                                           
                                                              
function yours()                                              
   local a : boolean = true                                   
   repeat                                                     
       a = to_be_a_bee_but_bi_bee_and_maybe_be_a_bee()        
   until a == true                                            
end                                                           
yours()                                                       

