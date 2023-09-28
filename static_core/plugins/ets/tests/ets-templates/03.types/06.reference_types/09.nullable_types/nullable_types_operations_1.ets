/*---
desc: Nullable types
---*/

function main(): void {
  let a: int = 5;
  let x: Int|null = 5 + a;
  assert x == 10;
}
