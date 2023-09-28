/*---
desc: Nullable types
---*/

function main(): int {
  let a: Int|null = null;
  let b: Int|null = 4;
  try {
      let x: Int|null = b + a;
  } catch (e: NullPointerException) {
      return 0;
  }
  return 1;
}
