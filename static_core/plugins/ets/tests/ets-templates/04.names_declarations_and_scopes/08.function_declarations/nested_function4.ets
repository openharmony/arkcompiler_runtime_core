/*---
desc: Functions must be declared on the top-level.
tags: [compile-only, negative]
---*/

function main(): void {
    for (let i:int = 1; i < 10; i++) {
        function nested(a: int): void {}
        nested(i);
    }
}
