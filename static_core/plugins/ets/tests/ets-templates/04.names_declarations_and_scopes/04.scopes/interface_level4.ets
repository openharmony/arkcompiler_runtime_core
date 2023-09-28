/*---
desc: Interface level scope, public access
---*/

let x = 10

interface I {
    static foo(): int {
        return x + 1;
    }
    bar(): int {
        return I.foo() + 2;
    }
}

class C implements I {
    public static foo(): int {
        return I.foo() + 10;
    }
}

function main(): void {
    let c: C = new C()

    assert I.foo() == 11
    assert c.bar() == 13
    assert C.foo() == 21

    // module level scope is not affected
    assert x == 10
}
