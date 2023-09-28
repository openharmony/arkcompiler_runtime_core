/*---
desc: Inherit private member.
assert: An interface inherits from its direct superinterfaces all the non-private member types of the superinterfaces that are both accessible to code in the interface and not hidden by a declaration in the interface.
tags: [negative, compile-only]
---*/

interface A {
  private m(): int {
    return 5;
  }
}

interface B extends A {
}

class D implements B {

}

function main(): void {
  assert new D().m() == 5;
}
