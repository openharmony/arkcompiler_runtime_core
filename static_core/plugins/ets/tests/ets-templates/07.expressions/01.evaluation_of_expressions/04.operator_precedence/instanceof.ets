/*---
desc: instanceof operator with other lower precedence operators
---*/

interface I {}
class A implements I {}
class B {}

function main(): int {
    let a: A = new A();
    let b: B = new B();
    let i: I = a;
    let an: A|null = null;

    // equality
    let res: boolean = a instanceof A == i instanceof A
    if (!res) return 1;

    res = a instanceof A != b instanceof A
    if (!res) return 1

    // ternary
    res = i instanceof A ? b instanceof A : b instanceof B
    if (res) return 1;

    res = i instanceof B ? b instanceof A : b instanceof B
    if (!res) return 1;

    return 0;
}
