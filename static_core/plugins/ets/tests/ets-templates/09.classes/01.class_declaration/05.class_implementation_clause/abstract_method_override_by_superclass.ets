/*---
desc: Class overrides abstract methods from superinterface by inheritance them from superclass.
assert: Unless the class being declared is abstract, all the abstract member methods of each direct superinterface must be implemented either by a declaration in this class or by an existing method declaration inherited from the direct superclass or a direct superinterface, because a class that is not abstract is not permitted to have abstract methods.
---*/

class A {
  final foo(): int {
    return 5;
  }

  final bar(): int {
    return 10;
  }
}

final class D extends A implements B, C {
}

interface B {
  bar(): int;
}

interface C {
  foo(): int;
}

function main(): void {
  let c: D = new D();
  let b: D = new D();

  assert c.foo() == 5;
  assert b.bar() == 10;
}
