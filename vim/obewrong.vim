if exists("b:current_syntax")
  finish
endif

syn keyword oKeyword class extends var method this return
syn keyword oKeyword module import
syn keyword oKeyword if else while loop end then
syn keyword oKeyword true false

syn keyword oType Integer Real Boolean String Array List Char

syn match oComment "\v//.*"

syn region oString start=/"/ end=/"/

syn match oNumber "\v<\d+>"
syn match oNumber "\v<\d+\.\d+>"

syn match oOperator "\v\="
syn match oOperator "\v\:"
syn match oOperator "\v\."
syn match oOperator "\v\,"
syn match oOperator "\v\("
syn match oOperator "\v\)"
syn match oOperator "\v\{"
syn match oOperator "\v\}"
syn match oOperator "\v\["
syn match oOperator "\v\]"
syn match oOperator "\v\+\-"
syn match oOperator "\v\-\-"
syn match oOperator "\v\*\*"
syn match oOperator "\v\/\/" contained
syn match oOperator "\v\<\>"
syn match oOperator "\v\<\="
syn match oOperator "\v\>\="

hi def link oKeyword Keyword
hi def link oType Type
hi def link oComment Comment
hi def link oString String
hi def link oNumber Number
hi def link oOperator Operator

let b:current_syntax = "obewrong"