/*---
desc: Access to hided method by cast.
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
}

function main(): void {
  let instance: A = new C();

  assert instance.m() == 1;
}
