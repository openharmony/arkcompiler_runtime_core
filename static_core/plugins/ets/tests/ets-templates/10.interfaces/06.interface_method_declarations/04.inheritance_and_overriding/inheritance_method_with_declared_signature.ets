/*---
desc: Inheritance method from superinterface with declared method with same signature in interface.
assert: An interface I inherits from its direct superinterfaces all abstract and default methods m for which all of the following are true - No method declared in I has a signature that is a subsignature of the signature of m.
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  foo(): int {
    return 2;
  }
}

class C implements B {}

function main(): void {
  let instance: C = new C();

  assert instance.foo() == 2;
}
