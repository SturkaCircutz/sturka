mod ast;
mod interpreter;
mod lexer;
mod parser;
mod token;

use std::env;
use std::fs;
use std::process;

use interpreter::Interpreter;
use lexer::Lexer;
use parser::Parser;

fn main() {
    if let Err(error) = run() {
        eprintln!("sturka-rs error: {error}");
        process::exit(1);
    }
}

fn run() -> Result<(), String> {
    let mut args = env::args().skip(1);
    let path = args
        .next()
        .ok_or_else(|| "Usage: sturka-rs <file.sturka>".to_string())?;

    if args.next().is_some() {
        return Err("Usage: sturka-rs <file.sturka>".to_string());
    }

    if !path.ends_with(".sturka") {
        return Err("Expected a .sturka source file".to_string());
    }

    let source = fs::read_to_string(&path)
        .map_err(|_| format!("Could not open file: {path}"))?;

    let tokens = Lexer::new(source).tokenize()?;
    let program = Parser::new(tokens).parse_program()?;

    let mut interpreter = Interpreter::new();
    interpreter.execute(&program)
}
