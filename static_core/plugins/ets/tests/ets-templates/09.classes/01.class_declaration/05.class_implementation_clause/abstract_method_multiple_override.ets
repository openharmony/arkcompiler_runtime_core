/*---
desc: Class overrides abstract methods from superinterface.
assert: Unless the class being declared is abstract, all the abstract member methods of each direct superinterface must be implemented either by a declaration in this class or by an existing method declaration inherited from the direct superclass or a direct superinterface, because a class that is not abstract is not permitted to have abstract methods.
---*/

final class A implements C, B {
  override foo(): int {
    return 5;
  }

  override bar(): int {
    return 10;
  }
}

interface B {
  bar(): int;
}

interface C {
  foo(): int;
}

function main(): void {
  let c: A = new A();
  let b: A = new A();

  assert c.foo() == 5;
  assert b.bar() == 10;
}
