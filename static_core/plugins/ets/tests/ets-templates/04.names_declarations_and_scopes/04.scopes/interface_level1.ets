/*---
desc: Interface level scope, public access
---*/

let x = -10

interface I {
    // this method is overridden by class instance method
    foo(): int {
        return x + 1;
    }

    bar(): int {
        return this.foo() + 2;
    }
}

class C implements I {
    public fld: int;
    // overrides default interface method
    public foo(): int {
        return 100 + this.fld;
    }
}

function main(): void {
    let c: C = new C()

    assert c.foo() == 100
    c.fld = 2
    assert c.foo() == 102

    assert c.bar() == 104
    c.fld = 4
    assert c.bar() == 106

    // module level scope is not affected
    assert x == -10
}
