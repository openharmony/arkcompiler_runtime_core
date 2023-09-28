/*---
desc: Override method declaration.
assert: The override keyword is mandatory if an instance method overrides the corresponding instance method from a superclass.
---*/

class A {
  foo(): int {
    return 1;
  }
}

final class B extends A {
  override foo(): int {
    return 5;
  }
}

function main(): void {
  let instance: B = new B();

  assert instance.foo() == 5;
}
