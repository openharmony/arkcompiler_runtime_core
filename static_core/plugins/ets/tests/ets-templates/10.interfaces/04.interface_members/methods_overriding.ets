/*---
desc: Override inheritance methods.
assert: The interface inherits, from the interfaces it extends, all members of those interfaces, except for methods that it overrides.
---*/

interface A {
  m(): int { return 5; }
}

interface B extends A {
  m(): int { return 10; }
}

class C implements B {
  public bar(): int {
    return this.m();
  }
}

function main(): void {
  let x: C = new C();
  assert x.bar() == 10;
}
