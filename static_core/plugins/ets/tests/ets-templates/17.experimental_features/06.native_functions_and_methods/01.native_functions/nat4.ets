/*---
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only]
---*/

native function foo(): void; // native without parameters

function main(): int {
    foo();
    return 0;
}
