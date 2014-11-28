## Pound

Pound was born out of the nonsensical wish to solve a programming assignment
that involves printing a matrix of `#` and `_` characters in a programming
language that can be completely programmed by only using `#` and `_` characters.

The bytecode that is being generated also only consists of `#` and `_` symbols.

Pound is a stack machine that runs on the bytecode generated from a basic
assembly language with only 9 instructions.

Numbers are represented in binary but instead of
`0` and `1`, `_` and `#` are used.

```
_ := 0
# := 1
```

Numeric constants can have up to 7 bits. The exact length of the tokens in the
assembly code matters (labels: 4 bits, instructions: 3 bits, numbers: 7 bits).
Don't pad with zeros (`_`).

The stack grows upwards (the bigger the stackpointer, the larger the stack).
The stackpointer always points to the next empty address above the stack.

```
sp := stackpointer
```

In the following documentation the value at a given position on the stack is
denoted by giving the stack address (e.g. sp or sp-1) and prefixing it with the
dereferencing operator used in c (`*`).

This means:

```
*(sp-1) -> The topmost value on the stack.
*(sp-2) -> The second value on the stack.
```

etc.

Generally all instructions consume their arguments. This means, that if our
stack looks like this:

```
0 5 7
      ^
      |
      stackpointer
```

and we execute an `add` operation our stack will look like this:

```
0 12
     ^
     |
     stackpointer
```

There are 8 registers which can be accessed by their addresses. They can be
used to save global variables. There is no RAM at this point.

Comments are denoted by the `;` symbol.

### Instruction table

```
+=============+==========+==============================================+
| Instruction | Mnemonic | Explanation                                  |
+-------------+----------+----------------------------------------------+
| NNNNNNN     | psh      | Pushes N (7 bits) onto the stack.            |
| ___         | nop      | Does nothing.                                |
| __#         | pop      | Decreases the sp.                            |
| _#_         | add      | Adds two numbers from the stack.             |
| _##         | sub      | Subtracts *(sp-1) from *(sp-2).              |
| #__         | ldr      | Pushes from register *(sp-1).                |
| #_#         | wtr      | Writes *(sp-2) to register *(sp-1).          |
| ##_         | jmp      | Jumps to *(sp-1) if *(sp-2) is zero.         |
| ###         | out      | Prints the ASCII value of *(sp-1) to stdout. |
+=============+==========+==============================================+
```

### Labels

Labels consist of a 4 bit number and are denoted by putting them directly in
the beginning of a line, followed by one or multiple whitespaces and then an
instruction. All lines without a label have to have at least one whitespace
character at the beginning of the line.

### Usage

`./bytecode_generator examples/Life.pound | ./stackmachine`

### Example

Print the numbers from 0 - 9 (mnemonics and clarifications in comments):

```
; r0
     _##____    ; 48       the current numbers ascii value
     _______
     #_#        ; wtr

; main:
____ _______    ; 0        output current ascii value
     #__        ; ldr
     ###        ; out

     ___#_#_    ; 10       output '\n'
     ###        ; out

     _______    ; 0        increment current ascii value
     #__        ; ldr
     ______#    ; 1
     _#_        ; add
     _______    ; 0
     #_#        ; wtr

     _###_#_    ; 58       48 + 10
     _______    ; 0
     #__        ; ldr      load r0
     _##        ; sub      58 - r0
     ___#       ; end
     ##_        ; jmp      jmp if (58 - r0) is equal to zero

     _______    ; 0
     ____       ; main
     ##_        ; jmp

; end:
___# ___        ; nop
```

The output:
```
0
1
2
3
4
5
6
7
8
9
```

### Random notes

Labels can only map to instructions up `_#######` (2^7 - 1) because the first
bit in each instruction is reserved for push instructions. This means, that in
practice programs get too big very fast!

For basic debugging of your code you can compile `stackmachine.cpp` with the
preprocessor macro `DEBUG` defined (`gcc -o stackmachine stackmachine.cpp
-DDEBUG`).
