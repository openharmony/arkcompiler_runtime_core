/*---
desc: There is only one literal (called undefined literal) that denotes a reference with a value that is not defined.
tags: [compile-only, negative]
---*/

function main() {
    let x: Object = undefined; // CTE: x is not of a nullish type
    assert x === undefined;
}
