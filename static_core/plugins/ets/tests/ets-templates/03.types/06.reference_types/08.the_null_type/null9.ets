/*---
desc: Function return null type
---*/

function f(): Int|null {
    return 5;
}

function main(): void {
    let a = f();
    assert a == 5;
}
