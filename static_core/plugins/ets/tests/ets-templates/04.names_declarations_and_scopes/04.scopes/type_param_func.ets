/*---
desc: >-
  The scope of a type parameter name in a function declaration is that entire declaration (function parameter scope).
tags: 
- compile-only
- negative
---*/

let T = 1

function foo<T extends Number>(p: T): T {
    let x: T = p;
    let res: T = x * 2; // CTE: arithmetic is not applicable to type parameters
    return res;
}

function main(): void {
    assert foo<Number>(new Number(2.0)) == 4.0

    // module level scope is not affected
    assert T == 1
}
