/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only, negative]
---*/

async native function foo<T extends Object>(): Promise<T>; // async can't coexist with native

function main(): int {
    return 0;
}
