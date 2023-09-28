/*---
desc: Override public default method from superinterface.
assert: An instance method mI declared in or inherited by interface I, overrides from I another instance method mJ declared in interface J, iff all of the following are true - I is a subinterface of J.
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
