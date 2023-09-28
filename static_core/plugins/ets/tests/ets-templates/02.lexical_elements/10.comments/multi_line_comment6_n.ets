/*---
desc: Multi-line comments
tags: [compile-only, negative]
params: missing closing asterisk
---*/

function main(): int {
    let i: int /* CTE expected / = 5;
    return i;
}
