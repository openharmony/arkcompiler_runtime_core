/*---
desc: Interface level scope, public access
---*/

let x = 10

interface I {
    static foo(): int {
        return x + 1;
    }
    static bar(): int {
        return I.foo() + 2;
    }
}

function main(): void {
    assert I.foo() == 11
    assert I.bar() == 13

    // module level scope is not affected
    assert x == 10
}
