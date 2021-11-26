require "ifj21"


function main ()
    local id : integer 
    local str_var : string
    local nil_var : nil

    if str_var == nil_var then
        local len_str : integer = 10
        id = len_str
	if id == len_str then
    	    write("\n")
	else
        end
    else end

    
    local aaa : integer = 0
    while str_var == nil do
        
	if id == 20 then
           str_var, aaa = chuj()         
        else
           id = id + 1
        end    
    end
    write(str_var, aaa, "\n")


    return
end

function chuj() : string, integer
  return "chuj", 7
end

main()

