/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only, negative]
---*/

native function foo(): void {}

function main(): int {
    foo();
    return 0;
}
