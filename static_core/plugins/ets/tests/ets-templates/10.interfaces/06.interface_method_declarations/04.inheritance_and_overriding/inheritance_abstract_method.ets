/*---
desc: Inheritance abstract method from superinterface.
assert: An interface I inherits from its direct superinterfaces all abstract and default methods m for which all of the following are true - m is a member of a direct superinterface, J, of I.
---*/

interface A {
  foo(): int;
}

interface B extends A {}

class C implements B {
  override foo(): int {
    return 1;
  }
}

function main(): void {
  let instance: B = new C();

  assert instance.foo() == 1;
}

