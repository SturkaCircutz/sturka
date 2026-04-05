# sturka

`sturka` is a tiny interpreted language implemented in C++.

Current features:

- `.sturka` source files
- `let` variable declarations
- integer arithmetic
- comparisons (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- `print(...)`
- `if` / `else`
- `while`

Example:

```sturka
let x = 3
let total = 0

while x > 0 {
    total = total + x
    x = x - 1
}

print(total)
```

Build with:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src/*.cpp -o sturka.exe
```

Run with:

```powershell
.\sturka.exe .\examples\sum.sturka
```

JIT is possible later, but this version intentionally starts as a clean AST interpreter.
