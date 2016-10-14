{-# LANGUAGE FlexibleInstances#-}
{-# LANGUAGE MultiParamTypeClasses #-}
module Interpreter
  (
    -- * Types
    Prog,

    -- * Functions
    evalRaw,
    evalAdt,
  ) where
-------------------------------------------------------------------------------
--------------------------------- The Expr ADT  -------------------------------
-------------------------------------------------------------------------------
data Expr = Add Expr Expr
          | Sub Expr Expr
          | Mult Expr Expr
          | Equal Expr Expr
          | Smaller Expr Expr
          | Symbol String
          | Value Int deriving (Show, Read)

-- [Optional] TODO Implement a parser for the Expr ADT.
--

-------------------------------------------------------------------------------
---------------------------------- The Prog ADT -------------------------------
-------------------------------------------------------------------------------
data Prog = Eq String Expr
          | Seq Prog Prog
          | If Expr Prog Prog
          | While Expr Prog
          | Return Expr deriving (Show, Read)

-- [Optional] TODO Implement a parser for the Prog ADT.
--

-- [Optional] TODO The *parse* function.  It receives a String - the program in
-- a "raw" format and it could return *Just* a program as an instance of the
-- *Prog* data type if no parsing errors are encountered, or Nothing if parsing
-- failed.
--
-- This is composed with evalAdt to yield the evalRaw function.
parse :: String -> Maybe Prog
parse = undefined

-------------------------------------------------------------------------------
-------------------------------- The Interpreter ------------------------------
-------------------------------------------------------------------------------

-- TODO The *evalAdt* function.  It receives the program as an instance of the
-- *Prog* data type and returns an instance of *Either String Int*; that is,
-- the result of interpreting the program.
--
-- The result of a correct program is always an Int.  This is a simplification
-- we make in order to ease the implementation.  However, we wrap this Int with
-- a *Either String* type constructor to handle errors.  The *Either* type
-- constructor is defined as:
--
-- data Either a b = Left a | Right b
--
-- and it is generally used for error handling.  That means that a value of
-- *Left a* - Left String in our case - wraps an error while a value of *Right
-- b* - Right Int in our case - wraps a correct result (notice that Right is a
-- synonym for "correct" in English).
-- 
-- For further information on Either, see the references in the statement of
-- the assignment.
--
type Dictionary = [([Char],Int)]

update :: ([Char], Int) -> Dictionary -> Dictionary
update (s,v) [] = [(s,v)]
update (s,v) ((a,b) : t) = if (s == a) then ((a,v) : t) else (a,b) : (update (s,v) t)


gol = []::Dictionary



valueOf :: Dictionary -> String -> Either String Int
valueOf [] str = Left "Uninitialized variable"
valueOf ((a,b):t) str = if (a == str) then (Right b) else (valueOf t str)


evalAdt :: Prog -> Either String Int
evalAdt p = let result = evalProg gol p
                r = if ((second result) == Left "Uninitialized variable") then (second result) else (if ((third result) == False) then (Left "Missing return") else (second result))
            in r


class Eval t where
    eval :: Dictionary -> t -> (Either String Int)

instance Eval Expr where
	eval dic (Value x) = Right x
	eval dic (Symbol x) = (valueOf dic x)
	eval dic (Add e1 e2) = (eval dic e1) `plus` (eval dic e2)
	eval dic (Sub e1 e2) = (eval dic e1) `minus` (eval dic e2)
	eval dic (Mult e1 e2) = (eval dic e1) `ori` (eval dic e2)
	eval dic (Smaller e1 e2) = (eval dic e1) `maimic` (eval dic e2)
	eval dic (Equal e1 e2) = (eval dic e1) `egalegal` (eval dic e2)

evalProg:: Dictionary -> Prog -> (Dictionary, Either String Int, Bool)
evalProg dic (Return e) = (dic, (eval dic e), True)
evalProg dic (Eq str e) = let esi = (eval dic e)
                              newDic = if (isNothing (fromRight esi)) then dic else (update (str, getMaybe(fromRight esi)) dic)
                          in (newDic, esi, False)

evalProg dic (Seq p1 p2) = let tuplu = evalProg dic p1
                               newDic = first tuplu
                               newState = second tuplu
                               newRetState = third tuplu
                               rez = if (newRetState == True) then (newDic, newState, newRetState) else (if (isNothing(fromRight newState) == True) then (dic, newState, newRetState) else (evalProg newDic p2)) 
                           in rez

evalProg dic (If e p1 p2) = let tuplu = (eval dic e)
                                rez = if (isNothing (fromRight tuplu)) then (dic, Left "Uninitialized variable", False)
                                      else (if (getMaybe (fromRight tuplu) == 1) then (evalProg dic p1) else (evalProg dic p2))
                            in rez


evalProg dic (While e p) = let rez = (eval dic e)
                               r = if (isNothing(fromRight rez) == True) then (dic, Left "Uninitialized variable", False)
                                    else ( if (getMaybe (fromRight rez) /= 0) then ( evalProg dic (Seq p (While e p))) else (dic, rez, False)  )
                           in r




plus :: (Either String Int) -> (Either String Int) -> (Either String Int)
plus (Left x) _ = (Left x)
plus _ (Left x) = (Left x)
plus (Right x) (Right y) = Right (x + y) 

minus :: (Either String Int) -> (Either String Int) -> (Either String Int)
minus (Left x) _ = (Left x)
minus _ (Left x) = (Left x)
minus (Right x) (Right y) = Right (x - y) 

ori :: (Either String Int) -> (Either String Int) -> (Either String Int)
ori (Left x) _ = (Left x)
ori _ (Left x) = (Left x)
ori (Right x) (Right y) = Right (x * y)

maimic :: (Either String Int) -> (Either String Int) -> (Either String Int)
maimic (Left x) _ = (Left x)
maimic _ (Left x) = (Left x)
maimic (Right x) (Right y) = if (x < y) then (Right 1) else (Right 0)  

egalegal :: (Either String Int) -> (Either String Int) -> (Either String Int)
egalegal (Left x) _ = (Left x)
egalegal _ (Left x) = (Left x)
egalegal (Right x) (Right y) = if (x == y) then (Right 1) else (Right 0)


getMaybe :: Maybe Int -> Int
getMaybe (Just x) = x

fromRight :: Either String Int -> Maybe Int
fromRight (Left _) = Nothing
fromRight (Right x) = Just x

isNothing         :: Maybe a -> Bool
isNothing Nothing = True
isNothing _       = False

first :: (a,b,c) -> a
first (a, _, _) = a

second ::(a,b,c) -> b
second (_, b, _) = b

third :: (a,b,c) -> c
third (_,_, c) = c



-- The *evalRaw* function is already implemented, but it relies on the *parse*
-- function which you have to implement.
--
-- Of couse, you can change this definition.  Only its name and type are
-- important.
evalRaw :: String -> Either String Int
evalRaw rawProg = case parse rawProg of
                    Just prog -> evalAdt prog
                    Nothing   -> Left "Syntax error"
