-- Zajisteni zakladni kompatibility IFJ21->Teal

function tointeger(f : number) : integer
  local _ : number
  local ret : integer
  if type(f) == "number" or type(f) == "integer" then 
    ret, _ = math.modf(f)
  end
  return ret
end

function readi() : integer
  local ret : string
  local _ : string 
  local rest : string
  ret, _ = io.read("*n")
  rest, _ = io.read("*l")   -- hack pro nacteni zbytku radku vcetne EOL
  if string.match(rest, "%g") ~= nil then  -- overeni zbytku radku
    return nil
  end
  return math.tointeger(ret)  -- nenulova desetinna cast zpusobi, ze vrati nil
end

function readn() : number
  local ret : string
  local _ : string 
  local rest : string  
  ret, _ = io.read("*n")
  rest, _ = io.read("*l")   -- hack pro nacteni zbytku radku vcetne EOL  
  if string.match(rest, "%g") ~= nil then  -- overeni zbytku radku
    return nil
  end
  return tonumber(ret)
end

function reads() : string
  local ret : string
  local _ : string 
  ret, _ = io.read("*l")
  return tostring(ret)
end

function write(... : string | integer | number | boolean)  -- podpora boolean pro bonusove rozsireni
  local va = { n = select("#", ...), ... }
  for i = 1, va.n do
    local x = va[i]
    --io.write("Debug: ", tostring(x), " is ", type(x), "\n")
    if type(x) == "number" then
      if (pcall(function () string.format("%i", x) end)) then  -- cislo lze reprezentovat celym cislem 
        if string.find(tostring(x),".",1,true) == nil then -- cislo je bez desetinne tecky
          io.write(string.format("%i", x))  -- vypis jako integer
        else
          io.write(string.format("%a", x)) -- vypis jako hexdecimalni float (integer s nulovou, ale neprazdnou desetinnou casti)
        end
      else 
        io.write(string.format("%a", x)) -- vypis jako hexdecimalni float
      end
    elseif type(x) == "integer" then  -- tato podminka pravdepodobne nikdy neuspeje
      io.write(string.format("%i", x))  -- vypis jako integer 
    else
      io.write(tostring(x))  -- vypis jako string nebo boolean
    end
  end
end

function substr(str : string, i : integer, j : integer) : string
  if str == nil or i == nil or j == nil then
    error("ERROR 8: Unexpected nil value in the parameters.")
  end
  if i <= 0 or i > #str and j <= 0 or j > #str then  -- kontrola mezi
    return ""
  end
  return string.sub(str, i, j)
end

function ord(s : string, i : integer) : integer
  if s == nil or i == nil then
    error("ERROR 8: Unexpected nil value in the parameters.")
  end
  if i <= 0 or i > #s then
    return nil
  end
  return string.byte(s, i)
end

function chr(i : integer) : string
  if i == nil then
    error("ERROR 8: Unexpected nil value in the parameter.")
  end 
  if i < 0 or i > 255 then
    return nil
  end
  return string.char(i)
end

-- Zde bude nasledovat program jazyka IFJ21

