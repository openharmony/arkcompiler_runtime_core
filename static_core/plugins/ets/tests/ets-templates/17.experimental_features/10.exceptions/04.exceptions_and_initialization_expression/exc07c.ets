/*---
desc: >-
  A variable declaration (§4.7.1) or a constant declaration (§4.7.2) expression
  used to initialize a variable or constant must not have calls to functions
  that can throw or rethrow exceptions.
tags: [compile-only, negative]
---*/

let i = 2;

function non_throwing_function(): int {
    return --i;
}

function throwing_function(): int throws {
    if (i * i != 4) {
        throw new Exception();
    }
    return i;
}

const v: int = 42 + non_throwing_function() + throwing_function(); // CTE expected

function main(): int {
    return v;
}
