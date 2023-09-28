/*---
desc: Inheritance default method from superinterface.
assert: An interface I inherits from its direct superinterfaces all abstract and default methods m for which all of the following are true - m is a member of a direct superinterface, J, of I.
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {}

class C implements B {}

function main(): void {
  let instance: C = new C();

  assert instance.foo() == 1;
}
