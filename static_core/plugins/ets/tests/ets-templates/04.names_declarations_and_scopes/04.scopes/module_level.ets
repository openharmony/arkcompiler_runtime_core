/*---
desc: Separate module scope
---*/

let x = 1;

function foo(): int {
    return x - 1;
}

class C {
    y: int = x + 1;
}

interface I {
    static z(): int {
        return x * 3;
    }
}

function main(): void {
    assert x == 1
    assert new C().y == 2
    assert I.z() == 3
    assert foo() == 0
}
