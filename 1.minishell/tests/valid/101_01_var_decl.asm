PUSH 0		; Address 0
STORE 0		; Address 5 (x)
PUSH 1		; Address 10
STORE 1		; Address 15 (y)
L001:
LOAD 0		; Address 20 (x)
PUSH 3		; Address 25
LT		; Address 30
JZ L002		; Address 31 (exit loop)
LOAD 0		; Address 36 (x)
PUSH 1		; Address 41
EQ		; Address 46
JZ L003		; Address 47 (if false jump)
LOAD 1		; Address 52 (y)
PUSH 1		; Address 57
ADD		; Address 62
STORE 1		; Address 63 (y)
JMP L004		; Address 68 (jump over else)
L003:
LOAD 1		; Address 73 (y)
PUSH 2		; Address 78
ADD		; Address 83
STORE 1		; Address 84 (y)
L004:
LOAD 0		; Address 89 (x)
PUSH 1		; Address 94
ADD		; Address 99
STORE 0		; Address 100 (x)
JMP L001		; Address 105 (loop back)
L002:
HALT		; Address 110
