# sturka-c

`sturka-c` is a plain C implementation of the `sturka` language.

This version mirrors the current English-style syntax:

- `let x be 1`
- `set x to 2`
- `print value`
- `if ... do ... else ... end`
- `while ... do ... end`
- arithmetic words: `plus`, `minus`, `times`, `over`
- comparison words: `is`, `is not`, `is less than`, `is greater than`, and their `or equal to` forms

Build:

```powershell
cd c
gcc -std=c11 -Wall -Wextra -pedantic -I./src src/main.c src/lexer.c src/ast.c src/parser.c src/interpreter.c -o sturka-c.exe
```

Run:

```powershell
.\sturka-c.exe ..\examples\sum.sturka
```
