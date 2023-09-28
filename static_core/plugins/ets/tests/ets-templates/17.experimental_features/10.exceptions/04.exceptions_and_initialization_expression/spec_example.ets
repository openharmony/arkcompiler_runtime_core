/*---
desc: >-
  A variable declaration (§4.7.1) or a constant declaration (§4.7.2) expression
  used to initialize a variable or constant must not have calls to functions
  that can throw or rethrow exceptions.
tags: [compile-only, negative]
---*/

let global_var = throwing_function() // Compile-time error

function throwing_function() throws {
    throw new Exception();
}

function main(): int {
    return 0;
}
