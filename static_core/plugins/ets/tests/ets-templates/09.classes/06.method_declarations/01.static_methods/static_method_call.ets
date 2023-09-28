/*---
desc: Static method call.
assert: A class method is always invoked without reference to a particular object.
---*/

class A {
  public static run(): int {
    return 5;
  }
}

function main(): void {
  assert A.run() == 5;
}
