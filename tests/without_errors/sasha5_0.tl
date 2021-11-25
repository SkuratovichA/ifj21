-- test case 5.
-- Description : mutual recursion

-- Expected : '0'
require "ifj21" 
global  foo : function (integer) : integer
                                          
function bar(param : integer) : integer   
    if param <= 0 then                    
        write("hello from bar!\n")     
        return param                      
    end                                   
    return foo(param - 1)                 
end                                       
                                          
function foo(param:integer ):integer      
    if param <= 0 then                    
        write("hello from foo!\n")     
        return param                      
    end                                   
    return bar(param-1)                   
end                                       

