/*---
desc: Floating number equality
---*/

function main(): void {
    let a = 0.1;
    let b = sqrt(-1);
    assert a != b;
    assert b != a;

    assert !(a == b);
    assert !(b == a);
}
