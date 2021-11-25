require "ifj21"

function chuj(n : integer, s : string) : integer
    local a : integer = 0
    while a < (n*n) do
	a = n + a * a 
    end
    return a
end

chuj(0, "a")
write(chuj(0, "a"), "\n")
write(chuj(100, "a"), "\n")
write(chuj(1000000, "a"), "\n")
write(chuj(10000000000000000, "a"), "\n")
write(chuj(90000000000000000000000000000000000000, "a"), "\n")
write(chuj(220000000000000005665456666666666666666, "a"), "\n")


