/*---
desc: Access to hided method by super keyword.
assert: It is a compile-time error if a static method hides an instance method.
---*/

class A {
 public m(): int {
    return 1;
  }
}

class C extends A {
  public m(): int {
    return 5;
  }

  public superm(): int {
    return super.m();
  }
}

function main(): void {
  let instance: C = new C();

  assert instance.superm() == 1;
}
