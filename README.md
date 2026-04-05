# sturka

`sturka` is a tiny programming language with a C++ prototype and a parallel Rust compiler track.

Current features:

- `.sturka` source files
- English-style declarations: `let x be 1`
- English-style assignment: `set x to 2`
- arithmetic with words: `plus`, `minus`, `times`, `over`
- comparisons with words: `is`, `is not`, `is less than`, `is less than or equal to`, `is greater than`, `is greater than or equal to`
- `print value`
- `if ... do ... else ... end`
- `while ... do ... end`
- two execution modes:
  interpreter mode
  JIT mode (`--jit`) that lowers the AST to C++ and compiles it just in time
- a parallel Rust implementation lives in `rust/` and is intended to become the cleaner compiler core

Example:

```sturka
let x be 3
let total be 0

while x is greater than 0 do
    set total to total plus x
    set x to x minus 1
end

print total
```

Build with:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic src\main.cpp src\jit_engine.cpp src\lexer.cpp src\parser.cpp src\interpreter.cpp -o sturka.exe
```

Run with:

```powershell
.\sturka.exe .\examples\sum.sturka
```

Run with JIT:

```powershell
.\sturka.exe --jit .\examples\sum.sturka
```

The current JIT is a practical first step: it generates C++ from the parsed AST,
builds a temporary DLL with `g++`, loads it, and executes the compiled code.
That keeps the front-end stable while moving execution off the tree-walking interpreter.

Project layout:

- `src/`: original C++ implementation
- `examples/`: shared `.sturka` programs
- `rust/`: next-generation Rust implementation with a cleaner compiler pipeline
