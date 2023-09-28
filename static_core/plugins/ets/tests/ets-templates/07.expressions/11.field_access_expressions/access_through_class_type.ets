/*---
desc: Access to field though class type.
assert: A field access expression cannot denote a package, class type, or interface type.
tags: [negative]
---*/

class A {
  public x: int;
}

function main(): void {
    assert A.x == 0;
}
