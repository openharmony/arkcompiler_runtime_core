/*---
desc: null-coalescing operator with other lower precedence operators
---*/

interface I {}
class A implements I {}
class B {}

function main(): int {
    let a: A = new A();
    let b: B = new B();
    let i: I = a;
    let an: A|null = null;
    let bn: B|null = null;

    // ternary
    let res: boolean = ((an ?? a) instanceof I ? bn ?? b : an ?? a) === b
    if (!res) return 1;

    res = ((a ?? an) instanceof B ? an ?? a : bn ?? b) === b
    if (!res) return 1;

    // assignment
    let x: Object = bn ?? a
    if (x !== a) return 1;

    let y: Object = an ?? b
    if (y !== b) return 1;

    return 0;
}
