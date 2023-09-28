/*---
desc: First expression is of the type boolean.
---*/

function main(): void {
  let a = true ? 0 : 1;
  assert a == 0;
}