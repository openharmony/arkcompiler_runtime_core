/*---
desc: >-
    Variables and other entities defined in a function are local to the function and cannot be accessed outside.
    If the name of a variable defined in the function is equal to the name of an entity in the outer scope,
    then the local definition shadows outer entity.
tags: [compile-only, negative]
---*/

let foo: int = 0;

function foo(): int {  // duplicate identifier
    return foo;
}

function main(): int {
    return foo();
}
