/*---
desc: The null literal denotes the null reference which represents an absence of a value and is a valid value only for types T | null, (see Nullish Types).
tags: [compile-only, negative]
---*/

function main() {
    let x: Object = null; // CTE: x is not of a nullish type
    assert x === null;
}
