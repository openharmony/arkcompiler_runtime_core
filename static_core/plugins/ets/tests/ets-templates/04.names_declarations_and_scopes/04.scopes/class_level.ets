/*---
desc: Class level scope
---*/

let x = 3

function foo(): int {
    return x;
}

class C {
    x: int = 1;
    foo(): int {
        return this.x;
    }
}

class D {
    x: int = 2;
    foo(): int {
        return this.x;
    }
}

function main(): void {
    let x = 4;
    assert new C().foo() == 1
    assert new D().foo() == 2
    assert foo() == 3
    assert x == 4
}
