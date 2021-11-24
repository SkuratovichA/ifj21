require "ifj21"

function func (id1 : integer, id2 : string, id3 : number) : integer, integer, string, number
    return 12, 12, "AAA", 90.5e1
end

function main ()
    local id1 : integer
    local id2 : integer
    local id3 : string
    local id4 : number

    id1, id2, id3, id4 = func(id1, id3)
end

main()
