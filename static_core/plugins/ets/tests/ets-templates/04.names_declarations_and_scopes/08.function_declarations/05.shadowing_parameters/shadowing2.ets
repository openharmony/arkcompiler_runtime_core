/*---
desc: >-
    If the name of the parameter is identical to the name of the top-level variable
    accessible within the body of a function or a method with that a parameter,
    then within the body of such a function or a method the name of that parameter
    shadows the name of the top-level variable.
---*/

let q: int = 1;

function bar(): int {
    let q: int = 10   // local q shadows top-level q
    return q + q;
}

function main(): int {
    q = 1;
    if (bar() == 20 && q == 1) return 0;
    return 1;
}
