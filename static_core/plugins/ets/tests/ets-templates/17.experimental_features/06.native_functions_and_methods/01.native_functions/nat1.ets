/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only, negative]
---*/

native function foo(a: int): int {
    return a;
}

function main(): int {
    return foo(0);
}
