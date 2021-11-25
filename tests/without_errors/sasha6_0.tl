-- test case 6.
-- Description : strings, builtin functions

-- Expected : '0'
-- Program 3: Prace s ěretzci a vestavenymi funkcemi
require "ifj21" 
function main()                                    
   local s1 : string =                             
             "\065\065\065\010ILOVE" 
   local s2 : string = s1 .. "MINECRAFT\n"      
   write(s2)                                       
                                                   
   local s1len : integer = #s2                     
   s1len = s1len - 4                               
   s1 = substr(s2, s1len, s1len + 4)               
   write(s1)                                       
   write(s2)                                       
   s1 = reads()                                    
end                                                
main()                                             

