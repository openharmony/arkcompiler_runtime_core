/*---
desc: Nullable types
---*/

function main(): void {
  let b: int = 5
  let x: Int|null = b;
  assert x == 5;
}
