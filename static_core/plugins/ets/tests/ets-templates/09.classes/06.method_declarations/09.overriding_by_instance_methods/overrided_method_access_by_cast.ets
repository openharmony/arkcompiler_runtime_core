/*---
desc: Cast to superclass for access to overridden method.
assert: A qualified name or a cast to a superclass type is not effective in attempting to access an overridden method.
---*/

class A {
  public m(): int {
    return 1;
  }
}

final class C extends A {
  override m(): int {
    return 5;
  }
}

function main(): void {
  let instance: A = new C() as A;

  assert instance.m() == 5;
}
