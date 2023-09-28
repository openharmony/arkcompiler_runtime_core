/*---
desc: Nullable types
tags: [negative]
---*/

class A {
  foo(): int {
    return 5;
  }
};

function main(): void {
  let b: A | null = null;
  let x: int = b.foo();    // NPE
}
