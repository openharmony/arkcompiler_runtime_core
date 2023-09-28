/*---
desc: Default values for reference type
tags: [negative]
---*/

class A {
  foo(): int { return 5; }
}

function main(): void {
  let x: A;
  x.foo();
}
