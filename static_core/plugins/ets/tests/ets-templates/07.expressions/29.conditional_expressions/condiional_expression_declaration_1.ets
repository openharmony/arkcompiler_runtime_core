/*---
desc: First expression is of the type Boolean.
---*/

function main(): void {
  let a = new Boolean(true) ? 0 : 1;
  assert a == 0;
}