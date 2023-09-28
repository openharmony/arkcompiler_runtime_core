/*---
desc: Access to field though interface type.
assert: A field access expression cannot denote a package, class type, or interface type.
tags: [negative]
---*/

interface A {
  x: int = 1;
}

function main(): void {
    assert A.x == 1;
}