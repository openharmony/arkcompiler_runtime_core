/*---
desc: Function return null type
---*/

function f(): Double|null {
    return null;
}

function main(): void {
    let a = f();
    assert a == null;
}
