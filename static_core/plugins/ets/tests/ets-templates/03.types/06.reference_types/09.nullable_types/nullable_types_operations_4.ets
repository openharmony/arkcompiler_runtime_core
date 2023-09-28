/*---
desc: Nullable types
---*/

function main(): void {
  let a: Int|null = 5;
  let b: Int|null = 4;
  let x: Int|null = b + a;
  assert x == 9;
}
