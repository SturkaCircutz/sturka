# sturka-rs

`sturka-rs` is the next-generation Rust implementation of the `sturka` language.

This directory intentionally lives beside the original C++ implementation so the
language can evolve without losing the first prototype.

Planned pipeline:

1. lexer
2. parser
3. AST
4. semantic analysis
5. bytecode or IR
6. optimized execution backend

Current status:

- parses and interprets the same English-style `.sturka` syntax as the C++ version
- uses a cleaner module layout to prepare for future compiler work

Expected build command once Rust is installed:

```powershell
cd rust
cargo run -- ..\examples\sum.sturka
```
