/*---
desc: Floating number equality
---*/

function main(): void {
    let a = 0.1;
    let b = -log(0);
    let c = log(0);
    assert a != b;
    assert b != a;
    assert a != c;
    assert c != a;
}
