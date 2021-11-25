require "ifj21"

function main ()
    local str : string = "H"
    local a : integer

    if a == nil then
        str = str.."e"
        if a == nil then
            str = str.."l"
            if a == nil then
                str = str.."l"

                local str1 : string = str
                if a ~= nil then
                else
                    local str : string = str
                    str = str.."o"
                    str1 = str
                end

                str = str1.."\x0A"
            else end
        else end
    else end

    local len : number = #str
    write("Len of str = ", len, "str contains ", str)
end

main()
