/*---
desc: Same method signature
tags: [negative, compile-only]
---*/

class C {
    function f(a: int): int {
        return 0;
    }

    function f(a: int): int {
        return 0;
    }
}

function main(): void {}
