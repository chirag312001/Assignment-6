PUSH 3
STORE 0        ; i

PUSH 0
STORE 2        ; sum

outer:
LOAD 0
JZ end_outer

PUSH 2
STORE 1        ; j

inner:
LOAD 1
JZ end_inner

LOAD 2
PUSH 1
ADD
STORE 2        ; sum++

LOAD 1
PUSH 1
SUB
STORE 1

JMP inner

end_inner:
LOAD 0
PUSH 1
SUB
STORE 0

JMP outer

end_outer:
LOAD 2
HALT
