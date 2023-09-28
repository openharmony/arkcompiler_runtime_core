/*---
desc: Nullable types
---*/

class A {};

function main(): void {
  let b: A|null = null;
  let x: A|null = b;

  assert x == null;
}
