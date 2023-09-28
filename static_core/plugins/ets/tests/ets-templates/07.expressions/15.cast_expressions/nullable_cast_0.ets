/*---
desc: nullable Cast T | null -> T. not-null value
---*/

function main(): void {
  let a: Int | null = new Int(1);
  let b: Int = a as Int;

  assert b == 1;
}
