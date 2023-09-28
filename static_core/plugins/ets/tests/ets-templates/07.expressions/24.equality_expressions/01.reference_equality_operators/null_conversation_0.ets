/*---
desc: The result of === at runtime is true if both operand values are null
---*/

class A {}

function main(): void {
  let b: A|null = null;
  let c: A|null = null;
  assert b === c && b == c;
}
