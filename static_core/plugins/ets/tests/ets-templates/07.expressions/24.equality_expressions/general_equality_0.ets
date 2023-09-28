/*---
desc: Equality operators, common rules
---*/

function main(): void {
    let a: int = 0;
    let b: int = 0;
    let c: int = 1;

    let cmp1: boolean = (a == b) == (b != c);
    let cmp2: boolean = (a == b) & b != c;

    assert cmp1 == cmp2;
}