require "ifj21"
function program()
  local y : integer = 20
  if 1 == 1 then
    y = 10
    write(y)
  else
    write(y)
  end
  write(y) -- vypise 10
  local i : integer = 1
  while i <= 10 do
    local i : integer = i + 5
    i = i + 1
    write(i)
    -- nekonecny cyklus (7...)
    write("Insert non-empty input for quiting, otherwise next iteration.\n")
    local inp : string = reads()
    if #inp ~= 0 then return else end  -- extra moznost prerusit nekonecny cyklus
  end
end
program()
