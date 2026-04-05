use std::collections::HashMap;

use crate::ast::{Expr, Program, Stmt};

pub struct Interpreter {
    variables: HashMap<String, i64>,
}

impl Interpreter {
    pub fn new() -> Self {
        Self {
            variables: HashMap::new(),
        }
    }

    pub fn execute(&mut self, program: &Program) -> Result<(), String> {
        for statement in program {
            self.execute_statement(statement)?;
        }
        Ok(())
    }

    fn execute_statement(&mut self, stmt: &Stmt) -> Result<(), String> {
        match stmt {
            Stmt::Let { name, value } => {
                if self.variables.contains_key(name) {
                    return Err(format!("Variable already defined: {name}"));
                }
                let computed = self.evaluate(value)?;
                self.variables.insert(name.clone(), computed);
            }
            Stmt::Assign { name, value } => {
                let computed = self.evaluate(value)?;
                let Some(slot) = self.variables.get_mut(name) else {
                    return Err(format!("Undefined variable: {name}"));
                };
                *slot = computed;
            }
            Stmt::Print { value } => {
                println!("{}", self.evaluate(value)?);
            }
            Stmt::If {
                condition,
                then_branch,
                else_branch,
            } => {
                if self.evaluate(condition)? != 0 {
                    for statement in then_branch {
                        self.execute_statement(statement)?;
                    }
                } else if let Some(branch) = else_branch {
                    for statement in branch {
                        self.execute_statement(statement)?;
                    }
                }
            }
            Stmt::While { condition, body } => {
                while self.evaluate(condition)? != 0 {
                    for statement in body {
                        self.execute_statement(statement)?;
                    }
                }
            }
        }

        Ok(())
    }

    fn evaluate(&mut self, expr: &Expr) -> Result<i64, String> {
        match expr {
            Expr::Integer(value) => Ok(*value),
            Expr::Variable(name) => self
                .variables
                .get(name)
                .copied()
                .ok_or_else(|| format!("Undefined variable: {name}")),
            Expr::Unary { op, operand } => {
                let value = self.evaluate(operand)?;
                match op.as_str() {
                    "-" => Ok(-value),
                    _ => Err(format!("Unsupported unary operator: {op}")),
                }
            }
            Expr::Binary { left, op, right } => {
                let left_value = self.evaluate(left)?;
                let right_value = self.evaluate(right)?;
                match op.as_str() {
                    "+" => Ok(left_value + right_value),
                    "-" => Ok(left_value - right_value),
                    "*" => Ok(left_value * right_value),
                    "/" => {
                        if right_value == 0 {
                            Err("Division by zero".to_string())
                        } else {
                            Ok(left_value / right_value)
                        }
                    }
                    "==" => Ok((left_value == right_value) as i64),
                    "!=" => Ok((left_value != right_value) as i64),
                    "<" => Ok((left_value < right_value) as i64),
                    "<=" => Ok((left_value <= right_value) as i64),
                    ">" => Ok((left_value > right_value) as i64),
                    ">=" => Ok((left_value >= right_value) as i64),
                    _ => Err(format!("Unsupported binary operator: {op}")),
                }
            }
        }
    }
}
