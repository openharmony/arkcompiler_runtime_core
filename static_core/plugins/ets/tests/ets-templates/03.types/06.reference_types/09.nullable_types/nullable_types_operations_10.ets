/*---
desc: Nullable types
---*/

function main(): void {
  let b: Int|null = 5;
  let x: int = b;

  assert x == 5;
}
