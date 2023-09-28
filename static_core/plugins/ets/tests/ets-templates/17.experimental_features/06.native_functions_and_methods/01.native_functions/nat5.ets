/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only]
---*/

native function foo(a: Object): int; // native with parameters

function main(): int {
    let v: int = foo(new Object());
    return 0;
}
