/*---
desc: Test calling constructors recursively
assert: Recursive constructor invocation is not allowed
tags: [compile-only, negative]
---*/

class A {

    constructor() {
        this(5)
    }

    constructor(v: int) {
        this()
    }
}
