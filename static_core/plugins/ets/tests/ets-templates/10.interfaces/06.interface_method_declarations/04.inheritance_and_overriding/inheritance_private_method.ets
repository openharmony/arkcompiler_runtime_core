/*---
desc: Inheritance private method from superinterface.
assert: An interface does not inherit private or static methods from its superinterfaces.
tags: [negative, compile-only]
---*/

interface A {
  private foo(): int {
    return 5;
  };
}

interface B extends A {}

class C implements B {
}

function main(): void {
  let instance: C = new C();

  assert instance.foo() == 5;
}
