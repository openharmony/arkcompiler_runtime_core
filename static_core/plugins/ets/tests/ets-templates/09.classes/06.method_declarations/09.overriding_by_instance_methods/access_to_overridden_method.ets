/*---
desc: Access to overridden method of superclass.
assert: An overridden method can be accessed by using a method invocation expression (link to that exp) that contains the keyword super.
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

  superm(): int {
    return super.m();
  }
}

function main(): void {
  let instance: C = new C();

  assert instance.m() == 5;
  assert instance.superm() == 1;
}
