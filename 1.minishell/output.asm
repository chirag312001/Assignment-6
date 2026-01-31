PUSH 0
STORE 0		; i
PUSH 0
STORE 1		; sum
PUSH 10
STORE 2		; limit
L001:
LOAD 0		; i
LOAD 2		; limit
LT
JZ L002		; exit loop
LOAD 0		; i
PUSH 0
EQ
JZ L003		; if false jump
LOAD 1		; sum
PUSH 1
ADD
STORE 1		; sum
JMP L004		; jump over else
L003:
LOAD 0		; i
PUSH 5
LT
JZ L005		; if false jump
LOAD 1		; sum
LOAD 0		; i
PUSH 2
MUL
ADD
STORE 1		; sum
JMP L006		; jump over else
L005:
LOAD 1		; sum
LOAD 0		; i
PUSH 1
ADD
LOAD 0		; i
PUSH 1
SUB
MUL
ADD
STORE 1		; sum
L006:
L004:
LOAD 0		; i
PUSH 1
ADD
STORE 0		; i
JMP L001		; loop back
L002:
HALT
