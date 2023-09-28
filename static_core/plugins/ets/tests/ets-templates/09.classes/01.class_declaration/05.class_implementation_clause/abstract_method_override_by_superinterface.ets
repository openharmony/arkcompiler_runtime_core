/*---
desc: Class overrides abstract methods from superinterface by inheritance them from superinterface.
assert: Unless the class being declared is abstract, all the abstract member methods of each direct superinterface must be implemented either by a declaration in this class or by an existing method declaration inherited from the direct superclass or a direct superinterface, because a class that is not abstract is not permitted to have abstract methods.
---*/

interface A {
  foo(): int {
    return 5;
  }

  bar(): int {
    return 10;
  }
}

final class D implements A, B, C {
}

interface B {
  bar(): int;
}

interface C {
  foo(): int;
}

function main(): void {
  let c: C = new D();
  let b: B = new D();

  assert c.foo() == 5;
  assert b.bar() == 10;
}
