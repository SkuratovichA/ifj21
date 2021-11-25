require "ifj21"

function func () : integer, integer, string, number
    return 12, 12, 12, "aaa"
end

function main ()
    local id1 : integer
    local id2 : integer
    local id3 : string
    local id4 : number

    id1, id2, id3, id4 = func()
end

main()
