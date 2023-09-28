/*---
desc: Nullable types
---*/

function main(): void {
  let a: int = 5;
  let b: int = 4;
  let x: Int|null = b + a;
  assert x == 9;
}
