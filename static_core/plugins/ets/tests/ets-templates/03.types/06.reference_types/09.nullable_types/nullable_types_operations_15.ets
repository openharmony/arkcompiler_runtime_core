/*---
desc: Nullable types
---*/

class A {
  foo(): int {
    return 5;
  }
};

function main(): void {
  let b: A|null = new A();
  let x: int = b.foo();

  assert x == 5;
}
