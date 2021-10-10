(* human: grammar rules in ascending order(from 1)
 * in .c  program comment write rule in EBNF form:
 * !rule <rule> -> productions
 * *)
open Str

let input_line_opt ic =
  try Some (input_line ic)
  with End_of_file -> None

 
let term = "[a-zA-Z_]+[0-9]*[a-zA-Z_]*"
let nonterm = "<" ^ term ^ ">"
let r_nonterm = regexp nonterm
let deriv = "->"

let rec merge list1 list2 =
    match list1, list2 with
        | [], _ -> list2
        | hd :: tl, _ -> hd :: merge tl list2

let read_lines ic =
  let start = ".*!rule" in
  let num = "\\([1-9]+[0-9]*\\|0\\)" in
  let numopt =  num ^ "?" in
  let numbendopt = "[.:]?" in
(*   let k_ws = " +" in (* kleene whitespaces *) *)
  let s_ws = "[ ]*" in (* star whitespaces *)
  let rall = ".*" in
  let idexp = "^" ^ start ^ s_ws ^ numopt ^ numbendopt ^ s_ws ^ nonterm ^ s_ws ^ deriv ^ rall ^ "$" in
  let replace_exp = "^" ^ start ^ s_ws ^ numopt ^ numbendopt ^ s_ws in  (* this must be replaced with an empty string *)
  let id_match x = string_match (regexp idexp) x 0 in
  let rec aux acc =
    match input_line_opt ic with
    | Some line ->
      if id_match line then
        let line1 = replace_first (regexp replace_exp) "" line in
        if String.contains line1 '|' then (* want to separate <rule> -> smth | smth2 into "<rule> -> smth" and "<rule> -> smth2" *)
          let rest = List.nth  (bounded_split (regexp (nonterm ^ s_ws ^ deriv ^ s_ws)) line1 1) 0 in
          (* get derived part*)
          let rest_splitted =
            if String.contains rest '|' then  Str.split (regexp (s_ws ^ "|" ^ s_ws)) rest 
            else [rest]
          in
          (* split into an array by token | or dont*)
          let head = Str.global_replace (regexp (s_ws ^ deriv ^ rall)) "" line1 in
          (* get head *)
          let headed_rest = List.map (fun x -> head ^ " -> " ^ x) rest_splitted in 
          (* create an array with separated rules *)
          aux (merge headed_rest acc)
        else
          aux (line1 :: acc)
      else
        aux acc
    | None ->
        (List.rev acc)
  in
  aux []
 
let lines_of_file filename =
  let ic = open_in filename in
  let lines = read_lines ic in
  close_in ic;
  (lines)

let get_filenames arr = 
  let acc = ref [] in
  for i = 1 to (Array.length arr) - 1 do
    acc := arr.(i) :: !acc
  done;
    !acc


let () =
  let lines = List.fold_left (fun acc x -> merge (lines_of_file x) acc) [] (get_filenames Sys.argv) in

  (* let lines = List.fold_left (fun x acc -> merge (lines_of_file x) acc) [] (get_filenames Sys.argv) in *)

  List.iteri (fun i x -> Printf.printf "%d: %s\n" (i + 1) x) (List.rev lines)





