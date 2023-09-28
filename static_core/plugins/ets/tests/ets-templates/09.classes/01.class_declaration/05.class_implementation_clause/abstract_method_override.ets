/*---
desc: Class overrides abstract methods from superinterface.
assert: Unless the class being declared is abstract, all the abstract member methods of each direct superinterface must be implemented either by a declaration in this class or by an existing method declaration inherited from the direct superclass or a direct superinterface, because a class that is not abstract is not permitted to have abstract methods.
---*/

final class A implements C {
  override foo(): int {
    return 5;
  }
}

interface C {
  foo(): int;
}

function main(): void {
  let a: A = new A();

  assert a.foo() == 5;
}
