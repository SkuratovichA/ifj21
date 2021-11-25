--require "ifj21"

function chuj(n : number, a : number) : number
	if a == 7 then 
        	return 0
  	end 
	return a * n + chuj(n, 7) 
end

write(chuj(0.48949, 5.5645646 ), "\n")
write(chuj(90000000000000000000000000000000000000, 4564.456), "\n")
write(chuj(220000000000000005665456666666666666666, 48997489.894981), "\n")

