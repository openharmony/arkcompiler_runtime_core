/*---
desc: Multi-line comments
tags: [compile-only, negative]
params: space in comment closing
---*/

function main(): int {
    let i: int /* CTE expected * / = 5;
    return i;
}
