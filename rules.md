##PLAN FOR REWRITE##

1. Input
    - The input command can either be a file or "--repl" or "-r"
    - The first of which opens and runs a file
    - The latter two open the REPL environment
        > The REPL environment evaluates each line at a time and saves only the defined identifiers
2. Lexing
    - The input must first be cleansed
        > '\n', '\t', '\r\n' => SPACE
        > Duplicate SPACE characters => SPACE
        > All other special characters will be ignored
3. Parsing
    - Only convert structure
    - Do NOT create special expression types, except that which are of the basic char->expr (Lst, Num, Idr, Str...)
    - Check for floating point duplicate '.' characters
4. Types
    - Num, Str, Idr, Lst
    - Special: Lam, Nat, IfE, Def, For, Prg 
5. Representation in Memory
    - Normal types will take up one expression, and are converted directly by the parser
    - In general, special forms will be converted from multiple parsed expressions
        > With this, additions to the CallStack, and VisitStack will be made to express these changes
        > Their representation will be "reformatted" in such a way that they will be able to alter the control flow of the program to their needs
        > e.g. IfE ==> (if (cond) 1 0) ==> {__if__, 1, 0} (cond) + add 1-parameter function
        > e.g. Prg ==> (Progn (fn arg) (fn arg) (fn arg)) ==> add 3 1-parameter functions to CallStack + add each (fn arg) to the VisitStack (will need to use return_addr, no doubt)
        > e.g. Def ==> (define (lambda (...) (...))) ==> __define__ (lambda (...) (...)) + 1-paramter function to CallStack
    - Special forms have to change the structure of the program, because they are inconsistent with the behaviour of the program, inherently
        > That they need to withhold parts of the program on occasion
        > That they use special evaluation rules
        > That they usually are not partially-applicable
        > Because of these things, it would be infeasible and inefficient to not forcefully change the behaviour of evaluation
6. Evaluation
    - One file for combined evaluator
        > Its only purpose is to iterate through the tree using the VisitStack and call for the running of functinos using the CallStack 
        > Also has combined return-value handler, combined expression-type handler
    - One file for each of the two stacks
    - One file for each type of special form
        > Function for identifying, creating, and handling
    - One file for handling native functions
7. Testing
    - Test at least 10 cases in each:
        > Individual lexer
        > Integrated lexer
        > Individual copy
        > Integrated copy
        > Individual destruct
        > Integrated destruct
        > Individual parse
        > Integrated parse
        > Individual evaluator
        > Integrated evaluator
    - Create test tool for this
    - Test super-edge cases for lexer, parser, evaluator in integration testing

