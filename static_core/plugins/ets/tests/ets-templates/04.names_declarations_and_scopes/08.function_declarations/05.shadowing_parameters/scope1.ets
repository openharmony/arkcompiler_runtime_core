/*---
desc: >-
    Variables and other entities defined in a function are local to the function and cannot be accessed outside.
    If the name of a variable defined in the function is equal to the name of an entity in the outer scope,
    then the local definition shadows outer entity.
tags: [no-warmup]
---*/

let q: int = 1;  // top-level scope

function getQ(): int {
    return q;   // returns top-level q
}

function bar(): int {
    q = q + 10;  // top-level scope, no shadowing
    return q;
}

function foo(): int {
    let q: int;  // function foo scope, top-level q is shadowed
    q = q + 20;
    return q;
}

function main(): int {
    let q: int;  // function main scope, top-level q is shadowed
    q = q + 30;

    if (getQ() == 1 && bar() == 11 && foo() == 20 && q == 30 && getQ() == 11) return 0;
    return 1;
}
