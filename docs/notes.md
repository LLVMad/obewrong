## 1. Lexer

### 1.1 DFA table:

Legend:
- left-most column - current state
- header - incoming input from source text
- cell - in a form of: `<next state> { | output }`, so we either change states or change states and return some output

|                     | A-z (char)             | 0-9 (num)              | whitespace                        | :                                                  | =   | (                                 | )                                 | ,                                 | .                                 | [                                 | ]                                 | "                  | >                 |
| ------------------- | ---------------------- | ---------------------- | --------------------------------- | -------------------------------------------------- | --- | --------------------------------- | --------------------------------- | --------------------------------- | --------------------------------- | --------------------------------- | --------------------------------- | ------------------ | ----------------- |
| 0 (start)           | 1                      | 2                      | 0                                 | 3                                                  | 8   | 0 \| return rbracket              | 0 \| return lbracket              | 0 \| return comma                 | 0 \| return method_acess          | 0 \| return lsbracket             | 0 \| return rsbracket             | 7                  | 9                 |
| 1 (read_word)       | 1                      | 4                      | 0 \| return keyword or identifier | 3 \| return identifier (if its a keyword -> error) | 9   | 0 \| return keyword or identifier | 0 \| return keyword or identifier | 0 \| return keyword or identifier | 0 \| return keyword or identifier | 0 \| return keyword or identifier | 0 \| return keyword or identifier | 7                  | 9                 |
| 2 (read_num)        | 9                      | 2                      | 0 \| return integer               | 9                                                  | 9   | 9                                 | 0 \| return integer               | 0 \| return integer               | 5                                 | 9                                 | 0 \| return integer               | 9                  | 9                 |
| 3 (read_decl)       | 1 \| return decl       | 2 \| return decl       | 0 \| return decl                  | 9                                                  | 6   | 9                                 | 9                                 | 9                                 | 9                                 | 9                                 | 9                                 | 7 \| return decl   | 9                 |
| 4 (read_identifier) | 4                      | 4                      | 0 \| return identifier            | 3 \| return identifier                             | 9   | 0 \| return identifier            | 0 \| return identifier            | 0 \| return identifier            | 0 \| return identifier            | 0 \| return identifier            | 0 \| return identifier            | 9                  | 9                 |
| 5 (read_real)       | 9                      | 5                      | 0 \| return real                  | 9                                                  | 9   | 9                                 | 0 \| return real                  | 0 \| return real                  | 9                                 | 0 \| return real                  | 9                                 | 9                  | 9                 |
| 6 (read_assign)     | 1 \| return assignment | 2 \| return assignment | 0 \| return assignment            | 9                                                  | 9   | 9                                 | 9                                 | 9                                 | 9                                 | 9                                 | 9                                 | 7 \| return assign | 9                 |
| 7 (read_string)     | 7                      | 7                      | 7                                 | 7                                                  | 7   | 7                                 | 7                                 | 7                                 | 7                                 | 7                                 | 7                                 | 0 \| return string | 7                 |
| 8 (read_arrow)      | 9                      | 9                      | 9                                 | 9                                                  | 9   | 9                                 | 9                                 | 9                                 | 9                                 | 9                                 | 9                                 | 9                  | 0 \| return arrow |
| 9 (fail)            |                        |                        |                                   |                                                    |     |                                   |                                   |                                   |                                   |                                   |                                   |                    |                   |

### 1.2 Fail table

Generate based on the previous state 

### 1.3 Symbol table

Links:
- https://github.com/AdaCore/cpython/blob/94894dd45e4bad6efb27eac4497b24cdc18b3e2d/Lib/symtable.py#L43
- https://github.com/eryjus/small-ada/blob/master/include/SymTable.h