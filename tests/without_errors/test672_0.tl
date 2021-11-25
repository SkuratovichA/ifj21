require "ifj21"

function chuj(n : integer) : integer
    local a : integer = 0
    while a < (n*n) do
        a = n + a * a 
    end
    return a
end

function main ()
    local id : integer = 100000 

    id = chuj(id)         
    
    write(id, "\n")


    return
end


main()

