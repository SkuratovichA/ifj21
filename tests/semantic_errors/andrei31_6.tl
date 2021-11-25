require "ifj21"

function main ()
    local id1 : string = "str"
    local id2 : string = "Str2"
    local id3 : string = "Str3"
    local id4 : string = "Str4"
    local id5 : string = "Str5"
    id1 = id2..id3..id4..id5..nil..id1
end

main()
