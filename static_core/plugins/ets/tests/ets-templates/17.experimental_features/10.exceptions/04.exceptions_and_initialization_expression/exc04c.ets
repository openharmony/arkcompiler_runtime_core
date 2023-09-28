/*---
desc: >-
  A variable declaration (§4.7.1) or a constant declaration (§4.7.2) expression
  used to initialize a variable or constant must not have calls to functions
  that can throw or rethrow exceptions.
tags: [compile-only, negative]
---*/

let i = 2;

function foo(): int throws {
    if (i * i != 4) {
        throw new Exception()
    }
    return 0;
}

function rethrowing_function(fn: () => int throws): int rethrows {
    return fn();
}

const v: int = rethrowing_function(foo); // CTE expected

function main(): int {
    return v;
}
