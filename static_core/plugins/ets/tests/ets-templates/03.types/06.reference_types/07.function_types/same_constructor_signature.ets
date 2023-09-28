/*---
desc: Same constructor signature
tags: [negative, compile-only]
---*/

class C {
    constructor(c: int) {
    }

    constructor(c: int) {
    }
}

function main(): void {}
