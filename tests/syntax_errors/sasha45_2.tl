-- test case 45.
-- Description : Syntax. No prolog string

-- Expected : '2'
function foo( r : string, a : string ) : string
    local A : integer = 10                      
    local B : integer = 10                      
    if A == B then                              
        local hello : integer = 10              
        if A == B then                          
            local world: string = "hello"     
            if A == B then                      
                local newl : integer            
            elseif A ~= B then                  
            end                                 
        end                                     
    else                                        
        local arsoui : integer                  
    end                                         
end                                             

