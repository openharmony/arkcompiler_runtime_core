/*---
desc: Nullable types
---*/

function main(): void {
  let a: (Int)|null = 4;
  let b: Int|null = 5;
  let c: Int|null|null = 9;
  let x: Int = a + b + c;

  assert x == 18;
}
