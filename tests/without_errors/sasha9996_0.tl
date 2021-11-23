-- Hello World example in IFJ21
-- run it on Merlin by: tl run hello.tl

require "ifj21"

function hlavni_program(year : integer)
  write("Hello from IFJ", year, "\n")
end

hlavni_program(21)
hlavni_program(22) -- pozdrav z budoucnosti
