/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only, negative]
---*/

native function foo(a: number); // CTE: return type is missing

function main(): int {
    foo(1.0);
    return 0;
}
