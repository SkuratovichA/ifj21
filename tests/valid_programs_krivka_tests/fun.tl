require "ifj21"

global g : function (integer) : integer  -- deklarace funkce

function f(x : integer) : integer
  if x < 10then return x-1
  else
    x = x - 1
    write("calling g with ", x, "\n")
    local res : integer = g(x)
    return res
  end
end

function g(x : integer) : integer
  if x > 0 then
    write("calling f with ", x, "\n")
    local x : integer = f(x)  -- pøekryji parametr x, ale až po zavolání f(x) [Teal varuje!]
    return x
  else return 200 end  -- proè musí být pøed end bílý znak?
end

function main()
  local res : integer = g(10)
  write("res: ", res, "\n")
end main()  -- main() schovane za klicovym slovem end je ve skutecnosti volani teto uzivatelske funkce
