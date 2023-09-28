/*---
desc: Nullable types
---*/

function main(): void {
  let a: Int|null = 5;
  let b: int = 4;
  let x: Int|null = b + a;
  assert x == 9;
}
