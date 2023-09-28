/*---
desc: >-
    The scope of a name declared immediately within the body of a function
    (method) declaration is the body of that function declaration (method
    or function scope) from the place of declaration and up to the end of
    the body.
tags: [compile-only, negative]
---*/

function f1() {
    let x = 9
}

function main() {
    f1()
    assert x == 9  // CTE
}
