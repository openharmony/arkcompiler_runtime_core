/*---
desc: >-
    Variables and other entities defined in a function are local to the function and cannot be accessed outside.
    If the name of a variable defined in the function is equal to the name of an entity in the outer scope,
    then the local definition shadows outer entity.
---*/

let q: int = 1;

function getQ(): int {
    return q;  // returns top-level q
}

function setQ(p: int): void {
    q = p;  // top-level q
}

function bar(q: int): int {  // q is shadowing top-level q
    q = q + getQ();
    return q;
}

function foo(q: int): int {
    setQ(q);
    return ++q;
}

function main(): int {
    q = 1;  // reset due to jit mode warmup

    if (bar(10) == 11 && q == 1 && foo(5) == 6 && q == 5) return 0;
    return 1;
}
