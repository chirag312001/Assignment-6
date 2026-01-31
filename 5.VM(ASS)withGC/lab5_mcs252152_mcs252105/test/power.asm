PUSH 2
STORE 0        ; base

PUSH 10
STORE 1        ; exponent

PUSH 1
STORE 2        ; result = 1

loop:
LOAD 1
JZ end         ; if exponent == 0, stop

LOAD 2
LOAD 0
MUL
STORE 2        ; result *= base

LOAD 1
PUSH 1
SUB
STORE 1        ; exponent--

JMP loop

end:
LOAD 2
HALT