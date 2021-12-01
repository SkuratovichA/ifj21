require "ifj21"

function chuj(n : integer, s : string) : integer
    local a : integer = 0
    if s == "chch" then
	    while a < (n*n) do
		a = n + a * a 
	    end
    else
        a = 7
    end 
    return a
end

function main ()
    local id : integer = 100000 

    id = chuj(id, "4")         
    
    write(id, "\n")


    return
end


main()

