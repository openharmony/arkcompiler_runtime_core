/*---
desc: Override private default method from superinterface.
assert: An instance method mI declared in or inherited by interface I, overrides from I another instance method mJ declared in interface J, iff all of the following are true - mJ is public.
tags: [negative, compile-only]
---*/

interface A {
  private foo(): int {
    return 1;
  }
}

interface B extends A {
  override foo(): int {
    return 2;
  }
}

class C implements B {}

function main(): void {
  let instance: C = new C();

  assert instance.foo() == 2;
}
