-- Program 3: Prace s retezci a vestavenymi funkcemi
require "ifj21"
function main()
  local s1 : string = "Toto je nejaky text"
  local s2 : string = s1 .. ", ktery jeste trochu obohatime"
  write(s1, "\010", s2, "\n")

  local s1len : integer = #s1
  local s1len4 : integer = #s1 - 3 
  s1 = substr(s2, s1len4, s1len) 
  write("4 znaky od ", s1len4, ". znaku v \"", s2, "\":", s1, "\n")

  s1 = substr(s2, 16, 19) -- vysledny retezec v s1 bude mit 19-16+1 znaku
  s1len4 = #s1
  write(s1len4, " znaky od ", 16, ". znaku v \"", s2, "\":", s1, "\n")  
end
main()
