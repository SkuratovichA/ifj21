(***human: grammar rules(parser) in ascending order for latex(nonterminals are bold) and everything else.
 * Rules start from 1.(first to last), but the first rule in latex == the last rule in the program.
 *
 * Latex comment generated, too. But you can comment those lines.(they are below). It's a trash, I know, but it works.

 ***Usage:
 * in .c  program comment write rule in EBNF form, e.g.
 * // some documentation. Im gonna get an oznuk.
 * // !rule <rule> -> derivations
 *
 **NOTE: !rule <rule> -> smth | <arstoien> will become
 *  <rule> -> smth
 *  <rule> -> <arstoien>
 *
 *
 *
 * P.S: yea, I know, the code is the peace of shit, but only god can judge me.(and disciplinarni komise)
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


  (* just a latex version *) 
  Printf.printf "\\begin{enumerate}\n"; (* because we want to create an automatic enumeration. Even if we dont need to... *) 
  let latexarrow = "\\rightarrow" in     (* just '->' if you want to output in terminal *) 
  let normarrow = "->" in
  let item = "\\item" in (* \\item  *)
  (* Iterate through every element in the list. *) 
  List.iter (fun x -> (Printf.printf "\t\\item $%s$\n" (* print \\item and a string itself, but... *) 
                        (global_replace (regexp " ") "\\ "              (* get rid of ws *)
                         (global_replace (regexp ">") "\\rangle{}$}"      (* same as before *)
                          (global_replace (regexp "<") "\\mbox{\\boldmath$\\langle{}"     (* latex doesn't like <, too *)
                           (global_replace (regexp "_") "\\_"           (* then kill undercores*)
                             (replace_first (regexp "->") latexarrow x) (* firstly get rid of the first arrow *)
                           )
                          )
                         )
                       )
                      ) 
            ) (List.rev lines); (* they are our lines. but prolog is below(suppose). *)
  Printf.printf "\\end{enumerate}\n";
  Printf.printf "\n\n\n";


  (* just a commandline vesion. (The easiest one. :^) *)
  List.iteri (fun i x -> Printf.fprintf stderr "%d: %s\n" (i + 1) x) (List.rev lines)







