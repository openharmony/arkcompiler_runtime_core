/*---
desc: >-
  An expression may occur:
  - inside a function body;
---*/

type Inc = (i: int) => long;

function inc(p: int): long {
    return (p + 1) as long;
}

function foo(i: int, func: Inc): long {
    return func(i);
}

function main(): int {
    let i: long = foo(12, inc);

    if (i != 13) return 1;
    return 0;
}
