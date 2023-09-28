/*---
desc: Interface level scope, public access
---*/

let x = 10

interface I {
    foo(): int {
        return x + 1;
    }
    bar(): int {
        return this.foo() + 2;
    }
}

class C implements I {
}

function main(): void {
    let c: C = new C()

    assert c.foo() == 11
    assert c.bar() == 13

    // module level scope is not affected
    assert x == 10
}
