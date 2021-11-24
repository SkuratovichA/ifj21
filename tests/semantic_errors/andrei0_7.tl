require "ifj21"

function func() : integer, number, string
    local id1 : integer
    local id2 : number
    local id3 : string

    return id1, id2, id3
end

function main()
    local id1 : integer
    local id2 : number
    local id3 : string
    local id4 : string

    id1, id2 = func()
    id1, id2, id3, id4 = 89, 90, "s"
end

main()
