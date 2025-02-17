## 1. Lexer

### 1.2 States

Table 1.1 - For words (either keyword or identifiers)

| id  | name                | action                                                                                   |
| --- | ------------------- | ---------------------------------------------------------------------------------------- |
| 0   | start_word          | expect next to be a letter                                                               |
| 1   | reading_word        | expect next to be a letter or a digit or a whitespace                                    |
| 2   | finish_reading_word | check if its a keyword ? return keyword token : return ident and push it to symbol table |

Table 1.2 - For numbers

| id  | name                 | action                             |
| --- | -------------------- | ---------------------------------- |
| 3   | start_num            | expect next to be a digit          |
| 4   | reading_digit        | expect next to be a digit or a dot |
| 5   | reading_real         | expect next to be a digit          |
| 6   | finish_reading_digit | return Integer                     |
| 7   | finish_reading_real  | return Real                        |

Table 1.3 - For special symbols (:=, :, (, ), ,, )

| id  | name                       | action                                         |
| --- | -------------------------- | ---------------------------------------------- |
| 8   | start_op                   | expect one of the defined symbols (operations) |
| 9   | reading_colon              | expect either '=' or ' '                       |
| 10  | reading_assignment         |                                                |
| 11  | finish_reading_assignment  |                                                |
| 12  | reading_declaration        |                                                |
| 13  | finish_reading_declaration |                                                |
| 14  | reading_rbacket            |                                                |
| 15  | finish_reading_rbracket    |                                                |
| 16  | reading_lbracket           |                                                |
| 17  | finish_reading_lbracket    |                                                |
| 18  | reading_comma_sep          |                                                |
| 19  | finish_reading_comma_sep   |                                                |

### 1.3 DFA table:

|     |     |
| --- | --- |
|     |     |
