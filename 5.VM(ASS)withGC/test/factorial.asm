PUSH 7
STORE 0        ; n

PUSH 1
STORE 1        ; result

loop:
LOAD 0
JZ end

LOAD 1
LOAD 0
MUL
STORE 1

LOAD 0
PUSH 1
SUB
STORE 0

JMP loop

end:
LOAD 1
HALT
