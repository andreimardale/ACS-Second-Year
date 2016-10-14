valnum(X):- char_type(X, alnum), char_type(X, ascii).
vother(X):- member(X, [';','<','+','-','*','(',')','{','}']).
validc(X):- valnum(X) ; vother(X) ;  X == '='.

lparseq(['='|L],'==',L).
lparseq([X|L],'=',[X|L]):-dif(X,'=').
lparseq([],'=',[]).

lparsealn([X|L],L2,R,L3):- valnum(X), lparsealn(L, [X|L2], R, L3).
lparsealn([X|L],L2,R,[X|L]):- \+valnum(X), reverse(L2, L3), atom_chars(R, L3).
lparsealn([],L2,R,[]):- reverse(L2, L3), atom_chars(R, L3).

lparse2(['='|L],L2,L3):- lparseq(L,R,L4), lparse2(L4,[R|L2],L3).
lparse2([X|L],L2,L3):- valnum(X),lparsealn(L,[X],R,L4), lparse2(L4,[R|L2],L3).
lparse2([X|L],L2,L3):- vother(X), lparse2(L,[X|L2],L3).
lparse2([X|L],L2,L3):- \+validc(X), lparse2(L,L2,L3).
lparse2([],L2,L3):- reverse(L2,L3).

lparse(S, L):- atom_chars(S, L2), lparse2(L2,[],L),!.


/*
 *
 */

un_pack(X, X) :- integer(X), !.
un_pack(X, R) :- atom_number(X,R), !.
un_pack(X, X).  


computeMult([X,;], L, R, []) :- append(L, [X,;], Ln ), compute(Ln, [] ,R, []), !.
computeMult([X,;| Remain], L, R, Remain) :- append(L, [X | [;| Remain]], Ln ), compute(Ln,[] ,R,  Remain), !.
computeMult([X,')'], L, R,  []) :- append(L, [X,')'], Ln ), compute(Ln, [] ,R,  []), !.
computeMult([X,')'| Remain], L, R,  Remain) :- append(L, [X | [')'| Remain]], Ln ), compute(Ln,[] ,R,  Remain), !.
computeMult([H1| [*| [H2|T]  ] ], L, Rez,  Remain):-  computeMult([mult(H1,H2)|T], L, Rez,  Remain), !.
computeMult([H1| [+| [H2|T]  ] ], L, Rez,  Remain) :- append(L, [H1, +], Ln) ,computeMult([H2|T], Ln , Rez,  Remain), !.
computeMult([H1| [-| [H2|T]  ] ], L, Rez,  Remain) :- append(L, [H1, -], Ln) ,computeMult([H2|T], Ln , Rez,  Remain), !.
computeMult([H1| [<| [H2|T]  ] ], L, Rez,  Remain) :- append(L, [H1, <], Ln) ,computeMult([H2|T], Ln , Rez,  Remain), !.
computeMult([H1| [==| [H2|T]  ] ], L, Rez,  Remain) :- append(L, [H1, ==], Ln) ,computeMult([H2|T], Ln , Rez,  Remain), !.
computeMult(_, _, e, _).



compute([X,;], L, R  ,[] ) :- append(L, [X], Ln), parseSmaller(Ln, [] ,R,  []), !.
compute([X,;| Remain], L, R,  Remain) :- append(L, [X | [;| Remain]], Ln ), parseSmaller(Ln, [] ,R,  Remain), !.
compute([X,')'], L, R  ,[] ) :- append(L, [X], Ln), parseSmaller(Ln, [] ,R,  []), !.
compute([X,')'| Remain], L, R,  Remain) :- append(L, [X | [')'| Remain]], Ln ), parseSmaller(Ln, [] ,R,  Remain), !.
compute([H1| [-| [H2|T]  ] ], L, Rez,  Remain) :- 	compute([sub(H1,H2)|T], L, Rez,  Remain), !.
compute([H1| [+| [H2|T]  ] ], L, Rez,  Remain) :- compute([add(H1,H2)|T], L, Rez,  Remain), !.
compute([H1| [<| [H2|T]  ] ], L, Rez,  Remain) :- append(L, [H1, <], Ln) ,compute([H2|T], Ln , Rez,  Remain), !.
compute([H1| [==| [H2|T]  ] ], L, Rez,  Remain) :- append(L, [H1, ==], Ln) ,compute([H2|T], Ln , Rez,  Remain).



parseSmaller([X,;], L ,R,  [] ) :- append(L, [X], Ln), parseEqual(Ln, R,  []), !.
parseSmaller([X,; | Remain ], L , R,  Remain ) :- append(L, [X | [;| Remain]], Ln ), parseEqual(Ln, R,  Remain), !.
parseSmaller([X,')'], L ,R,  [] ) :- append(L, [X], Ln), parseEqual(Ln, R,  []), !.
parseSmaller([X,')' | Remain ], L , R,  Remain ) :- append(L, [X | [')'| Remain]], Ln ), parseEqual(Ln, R,  Remain), !.
parseSmaller([H1| [<| [H2|T]  ] ], L, Rez,  Remain) :- parseSmaller([less(H1,H2)|T], L, Rez,  Remain), !.
parseSmaller([H1| [==| [H2|T]  ] ], L, Rez,  Remain) :- append(L, [H1, ==], Ln) ,parseSmaller([H2|T], Ln , Rez,  Remain).



parseEqual([X, ;], X, [] ) :- !.
parseEqual([X,;| Remain], X,  Remain) :- !.
parseEqual([X, ')'], X,  [] ) :- !.
parseEqual([X, ')'| Remain], X,  Remain) :- !.
parseEqual([H1| [==| [H2|T]  ] ], L,  Remain) :- parseEqual([eqeq(H1,H2)|T], L,  Remain), !.


parseExpr(L, O , Remain) :- computeMult(L, [], O, Remain).


% genereaza un element din lista input si bonus, il si sterge!
mem(E,[E|T], T) :- !.
mem(E, [H|T], [H|NT]) :- mem(E,T, NT).

%insereaza si updateaza o pereche (Variabila, Valoare)
put_value(Name, Value, L, [[Name,Value]|L]) :- not(member([Name, _], L)), !.
put_value(Name, Value, L, [[Name,Value]|Lp]) :- mem([Name, _], L, Lp).

% obtine valoarea pentru o anumita cheie
get_value(X, _, X) :- integer(X),!.
get_value(X, _, R) :- atom_number(X, R) , !.
get_value(_, [], e) :- !.
get_value(Name, [[K,V]|_], V) :- Name = K,!.
get_value(Name, [[K,_]|T], Value) :- Name \= K, get_value(Name, T, Value). 


is_string(H) :- atom_chars(H, [H1|_]), char_type(H1,alpha).

checkLetter([]).
checkLetter([H|T]) :- char_type(H,alpha), checkLetter(T).

is_string2(H) :- atom_chars(H, L), checkLetter(L), H \= 'if', H \= 'while',H \= 'then',H \= 'else', H \= 'return'.

parse_alpha([H|T], H, T) :- is_string(H).

parse_alpha2([H|T], H, T) :- is_string2(H).

parse_int([H|T], H, T) :- atom_number(H, _).

parse_char(C,[C|T], C, T).

parse_bracket(S,O,R) :- parse_char('(', S, O, R).
parse_open_Curly_bracket(S,O,R) :- parse_char('{', S, O, R).

parse_closed_Curly_bracket( S,O,R) :- parse_char('}', S, O, R).



star(P,S,[R|Rest], Sr) :- call(P,S,R,Sp), star(P,Sp,Rest,Sr),!.
star(_, S, [], S).

plus(P, S, [R|Rest], Sr) :- call(P,S,R,Sp), star(P,Sp,Rest,Sr).

parse_var(S,O,Rst) :- plus(parse_alpha, S, L, Rst), atomic_list_concat(L,O).

parse_variable(S,O,Rst) :- plus(parse_alpha2, S, L, Rst), atomic_list_concat(L,O).

parse_val(S,O,Rst) :- plus(parse_int, S, L, Rst), atomic_list_concat(L,O).

parse_tok(S,O,R) :- (parse_var(S, O, R), !; parse_val(S, O, R)).


parse_equal(S, equal(V,Rez) ,Remain) :- parse_variable(S,V,R1), parse_char('=', R1, _, R2), parseExpr(R2, Rez, Remain).

parse_return([return | L], return(V) , Remain) :- parseExpr(L, V, Remain).											 

parse_semicolon(S, semicln , Remain) :- parse_char(';', S, _, Remain).

parse_if(S, if(Cond,Then,Else), Remain) :- 	parse_tok(S , I , R1), I = 'if' ,parse_bracket(R1, _ , R2),
										   	parseExpr(R2,  Cond, R3), 
										  	parse_tok(R3, _, R4),
										  	parse_open_Curly_bracket(R4, _, R5),
										   	parse_program(R5, Then, R6), 
										   	parse_closed_Curly_bracket(R6, _, R7), 
										   	parse_tok(R7, _, R8),
										   	parse_open_Curly_bracket(R8, _, R9),
											parse_program(R9, Else, R10), Else \= e,
											parse_closed_Curly_bracket(R10, _, Remain), !.


parse_while(S, while(Cond, Body), Remain) :- parse_tok(S , W , R1), W = 'while' ,parse_bracket(R1, _ , R2),
											 parseExpr(R2,  Cond, R3),
										     parse_open_Curly_bracket(R3, _, R4),
										   	 parse_program(R4, Body, R5),
										  	 parse_closed_Curly_bracket(R5, _, Remain).


parse_program(S,  secv(Rez1, Rez2), Remain) :- parse_equal(S,  Rez1, Remain1), parse_program(Remain1,  Rez2, Remain), !.
parse_program(S,  Rez1, Remain) :- parse_equal(S,  Rez1, Remain), !.

parse_program(S,  secv(Rez1, Rez2), Remain) :- parse_return(S,  Rez1, Remain1), parse_program(Remain1,  Rez2, Remain), !.
parse_program(S,  Rez1, Remain) :- parse_return(S,  Rez1, Remain), !.

parse_program(S,  secv(Rez1, Rez2), Remain) :- parse_semicolon(S, Rez1, Remain1), parse_program(Remain1,  Rez2, Remain), !.
parse_program(S,  Rez1, Remain) :- parse_semicolon(S, Rez1, Remain), !.

parse_program(S,  secv(Rez1, Rez2), Remain) :- parse_if(S,  Rez1, Remain1), parse_program(Remain1,  Rez2, Remain), !. 
parse_program(S,  Rez1, Remain) :- parse_if(S,  Rez1, Remain), !.

parse_program(S,  secv(Rez1, Rez2), Remain) :- parse_while(S, Rez1, Remain1), parse_program(Remain1, Rez2, Remain), !.
parse_program(S,  Rez1, Remain) :- parse_while(S, Rez1, Remain), !.



eval_expr(add(A,B), Ctxt, R) :- eval_expr(A, Ctxt, Va),	eval_expr(B, Ctxt, Vb), (Va \= e, Vb \= e)  ,R is Va + Vb, !.
eval_expr(add(A,B), Ctxt, R) :- eval_expr(A, Ctxt, _),	eval_expr(B, Ctxt, _), R  = e, !.

eval_expr(sub(A,B), Ctxt, R) :- eval_expr(A, Ctxt, Va),	eval_expr(B, Ctxt, Vb), (Va \= e, Vb \= e) ,R is Va - Vb, !.
eval_expr(sub(A,B), Ctxt, R) :- eval_expr(A, Ctxt, _),	eval_expr(B, Ctxt, _) ,R = e, !.


eval_expr(mult(A,B), Ctxt, R) :- eval_expr(A, Ctxt, Va), eval_expr(B, Ctxt, Vb), (Va \= e, Vb \= e) ,R is Va * Vb, !.
eval_expr(mult(A,B), Ctxt, R) :- eval_expr(A, Ctxt, _), eval_expr(B, Ctxt, _), R = e, !.

eval_expr(less(A,B), Ctxt, R) :- eval_expr(A, Ctxt, Va), eval_expr(B, Ctxt, Vb), (Va \= e, Vb \= e) , Va < Vb, R is 1, !.
eval_expr(less(A,B), Ctxt, R) :- eval_expr(A, Ctxt, Va), eval_expr(B, Ctxt, Vb), (Va \= e, Vb \= e) ,Va >= Vb, R is 0, !.
eval_expr(less(A,B), Ctxt, R) :- eval_expr(A, Ctxt, _), eval_expr(B, Ctxt, _), R = e, !.

eval_expr(eqeq(A,B), Ctxt, R) :- eval_expr(A, Ctxt, Va), eval_expr(B, Ctxt, Vb), (Va \= e, Vb \= e) ,Va == Vb ,R is 1, !.
eval_expr(eqeq(A,B), Ctxt, R) :- eval_expr(A, Ctxt, Va), eval_expr(B, Ctxt, Vb), (Va \= e, Vb \= e) ,Va \= Vb ,R is 0, !.
eval_expr(eqeq(A,B), Ctxt, R) :- eval_expr(A, Ctxt, _), eval_expr(B, Ctxt, _), R = e, !.

eval_expr(A, Ctxt, R) :- get_value(A, Ctxt, R), !.

eval_prog(_,_,_,R,found, _) :- nonvar(R) ,!.
eval_prog(_,_,_,R,_, e) :- nonvar(R) ,!.

eval_prog(secv(A, B), Ctxt, Ctxt2, R, F, N) :- eval_prog(A, Ctxt, Ctxt1, R, _, N), eval_prog(B, Ctxt1, Ctxt2, R, F, N), !. 

eval_prog(return(A), Ctxt, Ctxt ,R, found, e) :- eval_expr(A, Ctxt, R), R = e, !.
eval_prog(return(A), Ctxt, Ctxt ,R, found, _) :- eval_expr(A, Ctxt, R), !.

eval_prog(equal(_, B), Ctxt, _, _, _, e) :- eval_expr(B, Ctxt, Vb), Vb = e, !.
eval_prog(equal(A, B), Ctxt, Ctxt1, _, _, _) :- eval_expr(B, Ctxt, Vb), put_value(A, Vb, Ctxt, Ctxt1), !.


eval_prog(semicln, Ctxt, Ctxt, _, _, _) :- !.

eval_prog(if(Cond,_,_), Ctxt, _, _, _, e) :- eval_expr(Cond, Ctxt, V), V = e, !.
eval_prog(if(Cond,Then,_), Ctxt, Ctxt1, R, F, N) :- eval_expr(Cond, Ctxt, V), V \= 0, eval_prog(Then, Ctxt, Ctxt1, R, F, N), !.
eval_prog(if(Cond,_,Else), Ctxt, Ctxt1, R, F, N) :- eval_expr(Cond, Ctxt, V), V = 0, eval_prog(Else, Ctxt, Ctxt1, R, F, N), !.

eval_prog(while(Cond, _), Ctxt0, _, _,_, e) :- eval_expr(Cond, Ctxt0, V), V = e, !.
eval_prog(while(Cond, _), Ctxt0, Ctxt, _,_, _) :- eval_expr(Cond, Ctxt0, V), V = 0, Ctxt = Ctxt0, !.
eval_prog(while(Cond, Body), Ctxt0, Ctxt, R, F, N) :- eval_expr(Cond, Ctxt0, V), V \= 0,
												 eval_prog(Body, Ctxt0, Ctxt1, R, _, N),
												 eval_prog(while(Cond,Body), Ctxt1, Ctxt, R, F, N), !.




parseProgr(L, e) :- parse_program(L, _, R), R \= [], !.
parseProgr(L, O) :- parse_program(L, O, _), !.



%parseInputAux(L, Rfinal) :- parse_program(L, Rfinal, _), !.

parseInputAux(L, O) :- parseProgr(L, O), O = e, !.
parseInputAux(L, Rez) :- parseProgr(L, O), O \= e ,eval_prog(O, [],_ , Rez, F, N), nonvar(F) ,var(N), !.
parseInputAux(L, e) :- parseProgr(L, O), O \= e ,eval_prog(O, [],_ , _, _, N), nonvar(N), !.

parseInputAux(L, x) :- parseProgr(L, O), O \= e ,eval_prog(O, [], _ ,_ , F, _), var(F) ,!.

parseInput(F,R):-read_file_to_string(F,S,[]), lparse(S,L), parseInputAux(L,R), !.

