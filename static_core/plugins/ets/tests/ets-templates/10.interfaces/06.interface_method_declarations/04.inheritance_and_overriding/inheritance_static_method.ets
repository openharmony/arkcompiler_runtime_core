/*---
desc: Inheritance static method from superinterface.
assert: An interface does not inherit private or static methods from its superinterfaces.
tags: [negative]
---*/

interface A {
  static foo(): int { 
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
