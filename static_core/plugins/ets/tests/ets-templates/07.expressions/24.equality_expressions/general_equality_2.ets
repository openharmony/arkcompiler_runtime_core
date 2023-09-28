/*---
desc: Equality operators, common rules
---*/

function main(): void {
    let a = 0;
    let b = 1;

    let cmp1 = a != b;
    let cmp2 = !(a == b);

    assert cmp1 == cmp2;
}