require "ifj21"

function write_numbers(counter : integer)
    while (counter) > 0 do
        write(counter, "\n")
        counter = counter - 1
    end
end

function main()
    local counter : integer = readi()

    if counter == 0 then
        write("Error", " enter", " another", " number", ", because ", 0, " is wrong", "\n")
        return
    else
        write_numbers(counter)
    end

end

main()
