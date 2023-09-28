/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only]
---*/

native function foo(a: Object): int throws; // native with throws

function main(): int throws {
    let v: int = foo(new Object());
    return 0;
}
