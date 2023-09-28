/*---
desc: >-
  A variable declaration (§4.7.1) or a constant declaration (§4.7.2) expression
  used to initialize a variable or constant must not have calls to functions
  that can throw or rethrow exceptions.
---*/

let i: int;

function non_throwing_function(): int {
    return i - 1;
}

function main(): int {
    i = 1;
    let v: int = non_throwing_function();
    return v;
}
