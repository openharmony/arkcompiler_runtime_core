/*---
desc: Declaration of a final class and inheritance from a parent class
---*/

class A {
  bar(): int {
    return 5;
  }
}

final class B extends A {}

function main() {
  let instance: B = new B();
  assert instance.bar() == 5;
}