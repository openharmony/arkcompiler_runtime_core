/*---
desc: Test call to private super
assert: Constructor is not visible here.
tags: [compile-only, negative]
---*/

class A {
    constructor() {

    }

    private constructor(v: int) {

    }
}

class B extends A {
    constructor() {
        super(100)
    }
}
