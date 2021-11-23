-- Program 1: Vypocet faktorialu (iterativne)
require "ifj21"

function main() -- uzivatelska funkce bez parametru
  local a : integer
  local vysl : integer = 0
  write("Zadejte cislo pro vypocet faktorialu\n")
  a = readi()
  if a == nil then
      write("a je nil\n") return
  else
  end
  if a < 0 then
    write("Faktorial nelze spocitat\n")
  else
    vysl = 1
    while a > 0 do
      vysl = vysl * a a = a - 1  -- dva prikazy
    end
    write("Vysledek je: ", vysl, "\n")
  end
end

main()  -- prikaz hlavniho tela programu