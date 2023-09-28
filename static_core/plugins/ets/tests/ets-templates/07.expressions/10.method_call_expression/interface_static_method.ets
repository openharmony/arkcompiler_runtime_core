/*---
desc: Call instance method of a interface.
assert: A method call expression calls a static or instance method of a class or an interface.
---*/

interface A {
  static bar(x: int): int {
    return x * 2;
  }
}

class B implements A {

}

function main(): void {
  assert new B().bar(3) == 6;
}
