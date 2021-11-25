require "ifj21"

global func : function (integer, integer) : string

function main (num : number)
    local id : integer = tointeger(num)
    local str_var : string
    local nil_var : nil

    if str_var == nil_var then
        local len_str : integer = 10
        str_var = func(id, len_str)
    else end

    write(str_var, "\x0A")
    return
end

function func (a : integer, b : integer) : string
    local id_str : string = ""
    local counter : integer = 0

    while counter < b do
        id_str = id_str.."A"
        counter = counter + 1
    end

    write(a, "\010")
    return id_str
end

main(10.9)
main(12.8e-2)
